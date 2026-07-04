# -*- coding: utf-8 -*
'''!
  @file bed_stillness_detection.py
  @brief Bed-zone stillness lighting control using active polling APIs.
  @details Rule summary:
  @n 1) If any static person exists in bed area for over 5s, turn room light OFF.
  @n 2) While rule 1 is active, new people entering room keeps light OFF.
  @n 3) If bed area has no static person, keep light ON when room has people.
  @n 4) After room transitions from occupied to empty, wait 5s then turn light OFF.
  @copyright Copyright (c) 2026 DFRobot Co.Ltd (http://www.dfrobot.com)
  @license The MIT License (MIT)
  @author JiaLi(zhixin.liu@dfrobot.com)
  @version V1.0.0
  @date 2026-05-22
  @url https://github.com/DFRobot/DFRobot_C4004
'''
import os
import sys
import time

cur_path = os.path.dirname(os.path.abspath(__file__))
while cur_path != os.path.dirname(cur_path):
  if os.path.exists(os.path.join(cur_path, 'DFRobot_C4004.py')):
    sys.path.insert(0, cur_path)
    break
  cur_path = os.path.dirname(cur_path)

from DFRobot_C4004 import DFRobot_C4004, TagConfig, FourSidedRange_t

try:
  import RPi.GPIO as GPIO
except Exception:
  GPIO = None

PORT = '/dev/ttyAMA0'
c4004 = DFRobot_C4004(PORT, 115200)

BED_TAG_INDEX = 0
BEDROOM_TAG_INDEX = 1
BEDROOM_DOOR_TAG_INDEX = 2
LIGHT_CTRL_PIN = 3

# Users can adjust these times according to their own preferences, needs,
# application scenarios, etc. The default time is 5 seconds.
BED_STATIC_HOLD_S = 5.0       # Time required to keep the bed state static.
BEDROOM_EMPTY_HOLD_S = 5.0    # Time required to keep the bedroom state empty.

LIGHT_OFF_LEVEL = 1
LIGHT_ON_LEVEL = 0

bed_static_start_s = 0.0
bedroom_empty_start_s = 0.0
bed_static_lock_off = False
bedroom_was_occupied = False
bed_motion_count = 0
bed_static_count = 0
bedroom_motion_count = 0
bedroom_static_count = 0
current_light_level = LIGHT_OFF_LEVEL


def apply_light_output(level):
  global current_light_level
  current_light_level = level
  if GPIO is not None:
    GPIO.output(LIGHT_CTRL_PIN, GPIO.HIGH if level else GPIO.LOW)


def setup_gpio():
  if GPIO is None:
    print('RPi.GPIO is not available, running without physical output control.')
    return
  GPIO.setmode(GPIO.BCM)
  GPIO.setup(LIGHT_CTRL_PIN, GPIO.OUT)
  apply_light_output(LIGHT_OFF_LEVEL)


def setup_sensor_and_tags():
  while not c4004.begin():
    print('DFRobot C4004 begin failed, retrying...')
    time.sleep(1)
  print('DFRobot C4004 begin success.')

  if c4004.set_check_to_active_frames(7):
    print('Set check-to-active frames success.')
  else:
    print('Set check-to-active frames failed.')
  time.sleep(0.05)

  if c4004.set_presence_enable(True):
    print('Set presence enable success.')
  else:
    print('Set presence enable failed.')

  range_info = FourSidedRange_t()
  range_info.mode = c4004.RANGE_FOUR_SIDE
  range_info.x_positive_cm = 200
  range_info.x_negative_cm = -200
  range_info.y_positive_cm = 700
  range_info.y_negative_cm = 0
  if c4004.set_four_sided_range_mode(range_info):
    print('Set boundary detection range success.')
  else:
    print('Set boundary detection range failed.')

  # Tag configuration note:
  # If the tags have already been configured by the PC tool, you do not need
  # to configure them again here. In that case, skip clear_all_tags() and
  # set_tags_from_config(tags).
  #
  # Field meaning:
  #   tag_index : Tag index. It must be unique for each tag.
  #   tag_type  : Tag function, such as PeopleCounting or ApproachAway.
  #   scope_type: Tag shape. Use TAG_RANGE_RECTANGLE or TAG_RANGE_CIRCLE.
  #   io_index  : IO linkage index. 0 means unused; 2-6 maps to IO2-IO6.
  #   center_x  : Tag center X coordinate, in cm.
  #   center_y  : Tag center Y coordinate, in cm.
  #   width     : Rectangle width, or circle radius, in cm.
  #   height    : Rectangle height, in cm. Not used for circle tags.

  if c4004.clear_all_tags():
    print('Clear all tags success.')
  else:
    print('Clear all tags failed.')

  tags = []

  bed = TagConfig()
  bed.tag_index = BED_TAG_INDEX
  bed.tag_type = c4004.TAG_PEOPLE_COUNTING
  bed.scope_type = c4004.TAG_RANGE_RECTANGLE
  bed.io_index = 0
  bed.center_x = -50
  bed.center_y = 300
  bed.width = 300
  bed.height = 250
  tags.append(bed)

  bedroom = TagConfig()
  bedroom.tag_index = BEDROOM_TAG_INDEX
  bedroom.tag_type = c4004.TAG_PEOPLE_COUNTING
  bedroom.scope_type = c4004.TAG_RANGE_RECTANGLE
  bedroom.io_index = 0
  bedroom.center_x = 0
  bedroom.center_y = 350
  bedroom.width = 400
  bedroom.height = 700
  tags.append(bedroom)

  door = TagConfig()
  door.tag_index = BEDROOM_DOOR_TAG_INDEX
  door.tag_type = c4004.TAG_APPROACH_AWAY
  door.scope_type = c4004.TAG_RANGE_RECTANGLE
  door.io_index = 0
  door.center_x = 100
  door.center_y = 700
  door.width = 80
  door.height = 40
  tags.append(door)

  if c4004.set_tags_from_config(tags):
    print('Set bed/bedroom/door tags success.')
  else:
    print('Set bed/bedroom/door tags failed.')

  print('============================================================')
  print('Bed stillness light control started.')
  print('Rule A: bed static(any person) over 5s => LIGHT OFF.')
  print('Rule B: if rule A active, bedroom new entry still keeps OFF.')
  print('Rule C: if rule A inactive, bedroom people>0 => LIGHT ON.')
  print('Rule D: bedroom occupied->empty over 5s => LIGHT OFF.')
  print('============================================================')


def update_people_counts_from_tag_report():
  global bed_motion_count
  global bed_static_count
  global bedroom_motion_count
  global bedroom_static_count

  for _ in range(4):
    event = c4004.get_reported_info(0.005)
    if event != c4004.EVENT_TAG:
      continue
    info = c4004.get_tag_info()
    if info is None or info.tag_type != c4004.TAG_PEOPLE_COUNTING:
      continue
    if info.tag_index == BED_TAG_INDEX:
      bed_motion_count = info.motion_num
      bed_static_count = info.static_num
    elif info.tag_index == BEDROOM_TAG_INDEX:
      bedroom_motion_count = info.motion_num
      bedroom_static_count = info.static_num


def main():
  global bed_static_start_s
  global bedroom_empty_start_s
  global bed_static_lock_off
  global bedroom_was_occupied

  setup_gpio()
  apply_light_output(LIGHT_OFF_LEVEL)
  setup_sensor_and_tags()

  last_print_s = 0.0
  try:
    while True:
      now_s = time.time()
      update_people_counts_from_tag_report()

      bed_has_static_person = bed_static_count > 0
      bedroom_people_count = bedroom_motion_count + bedroom_static_count
      bedroom_has_people = bedroom_people_count > 0

      if bed_has_static_person:
        if bed_static_start_s == 0:
          bed_static_start_s = now_s
        elif now_s - bed_static_start_s >= BED_STATIC_HOLD_S:
          bed_static_lock_off = True
      else:
        bed_static_start_s = 0
        bed_static_lock_off = False

      light_should_off = False
      if bed_static_lock_off:
        light_should_off = True
        bedroom_empty_start_s = 0
        bedroom_was_occupied = bedroom_has_people
      else:
        if bedroom_has_people:
          bedroom_was_occupied = True
          bedroom_empty_start_s = 0
          light_should_off = False
        else:
          if bedroom_was_occupied:
            if bedroom_empty_start_s == 0:
              bedroom_empty_start_s = now_s
            elif now_s - bedroom_empty_start_s >= BEDROOM_EMPTY_HOLD_S:
              light_should_off = True
          else:
            light_should_off = True

      apply_light_output(LIGHT_OFF_LEVEL if light_should_off else LIGHT_ON_LEVEL)

      if now_s - last_print_s >= 1.0:
        last_print_s = now_s
        print('============================================================')
        print('Bedroom people count     : %d' % bedroom_people_count)
        print('Bedroom motion/static    : %d/%d' % (bedroom_motion_count, bedroom_static_count))
        print('Bed motion/static count  : %d/%d' % (bed_motion_count, bed_static_count))
        print('Bed static hold active   : %s' % ('YES' if bed_static_lock_off else 'NO'))
        print('Light pin(3)             : %s' % ('OFF' if current_light_level == LIGHT_OFF_LEVEL else 'ON'))

      time.sleep(0.1)
  finally:
    if GPIO is not None:
      GPIO.cleanup()


if __name__ == '__main__':
  main()
