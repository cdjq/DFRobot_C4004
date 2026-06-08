# -*- coding: utf-8 -*
'''!
  @file multiZoneStatus.py
  @brief Read multi-zone tag events and print living-room scene linkage status.
  @details This routine can use tags configured by the PC tool or optionally configure
  @n tags in code. It keeps the last event result for each tag, prints a summary table
  @n every 3 seconds or when a tag event arrives, and drives outputs based on
  @n game-area and sofa-area people counting results.
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

TV_CTRL_PIN = 2
LIGHT_CTRL_PIN = 3

TAG_GAME = 0
TAG_SOFA = 1
TAG_HOME_DOOR = 2
TAG_KITCHEN_DOOR = 3
TAG_DINING = 4
TAG_CURTAIN = 5
TAG_PLANT = 6
TAG_TOTAL = 7

# Users can adjust these times according to their own preferences, needs,
# application scenarios, etc. The default time is 5 seconds.
GAME_NO_PERSON_DELAY_S = 5.0   # Delay before the game area is treated as empty.
SOFA_STATIC_DELAY_S = 5.0      # Delay before the sofa area is treated as static.
SOFA_MOTION_DELAY_S = 5.0      # Delay before the sofa area is treated as motion.
SOFA_EMPTY_DELAY_S = 5.0       # Delay before the sofa area is treated as empty.
LIGHT_PWM_LOW = 0
LIGHT_PWM_DIM = 150
LIGHT_PWM_HIGH = 255

TAG_NAMES = ['Game', 'Sofa', 'HomeDoor', 'KitchenDoor', 'Dining', 'CurtainNoise', 'PlantNoise']

# Tag cache entries keep one snapshot per tag index.
tag_cache = []
tag_print_pending = False
tv_output_high = False
light_output_value = LIGHT_PWM_LOW
game_no_person_start_s = 0.0
sofa_static_start_s = 0.0
sofa_motion_start_s = 0.0
sofa_empty_start_s = 0.0

_pwm = None


def tag_type_to_text(tag_type):
  if tag_type == c4004.TAG_NONE:
    return 'None'
  if tag_type == c4004.TAG_BOUNDARY:
    return 'Boundary'
  if tag_type == c4004.TAG_APPROACH_AWAY:
    return 'ApproachAway'
  if tag_type == c4004.TAG_PEOPLE_COUNTING:
    return 'PeopleCount'
  if tag_type == c4004.TAG_NOISE:
    return 'Noise'
  return 'Unknown'


def make_empty_cache(index):
  return {
    'tag_index': index,
    'tag_type': c4004.TAG_NONE,
    'io_index': 0,
    'center_x': 0,
    'center_y': 0,
    'enter_exit': 0,
    'motion_dir': 0,
    'motion_num': 0,
    'static_num': 0,
  }


def init_tag_cache_from_config(tags):
  global tag_cache
  tag_cache = [make_empty_cache(i) for i in range(TAG_TOTAL)]
  for tag in tags:
    index = tag.tag_index
    if index < 0 or index >= TAG_TOTAL:
      continue
    tag_cache[index]['tag_index'] = index
    tag_cache[index]['tag_type'] = tag.tag_type
    tag_cache[index]['io_index'] = tag.io_index
    tag_cache[index]['center_x'] = tag.center_x
    tag_cache[index]['center_y'] = tag.center_y
    if index in (TAG_HOME_DOOR, TAG_KITCHEN_DOOR):
      tag_cache[index]['motion_dir'] = 1


def init_tag_cache_from_device():
  tags = c4004.get_tags()
  init_tag_cache_from_config(tags)
  print('Read tag config count: %d' % len(tags))
  return len(tags) > 0


def setup_gpio():
  global _pwm
  if GPIO is None:
    print('RPi.GPIO is not available, running without physical output control.')
    return

  GPIO.setmode(GPIO.BCM)
  GPIO.setup(TV_CTRL_PIN, GPIO.OUT)
  GPIO.setup(LIGHT_CTRL_PIN, GPIO.OUT)
  GPIO.output(TV_CTRL_PIN, GPIO.LOW)

  # Map 0~255 logical brightness to PWM duty on GPIO.
  _pwm = GPIO.PWM(LIGHT_CTRL_PIN, 1000)
  _pwm.start(0)


def apply_outputs():
  if GPIO is None:
    return

  GPIO.output(TV_CTRL_PIN, GPIO.HIGH if tv_output_high else GPIO.LOW)

  if _pwm is not None:
    duty = max(0.0, min(100.0, float(light_output_value) * 100.0 / 255.0))
    _pwm.ChangeDutyCycle(duty)
  else:
    GPIO.output(LIGHT_CTRL_PIN, GPIO.HIGH if light_output_value > 0 else GPIO.LOW)


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
  # to configure them again here. In that case, keep the following tag setup
  # code commented out.
  #
  # If you want this example to configure tags automatically, uncomment the
  # clear_all_tags(), tags list, and set_tags_from_config(tags) code below.
  # init_tag_cache_from_device() reads the actual tag list from the sensor and
  # uses it to initialize the local print cache.
  #
  # Field meaning:
  #   tag_index : Tag index. It must be unique for each tag.
  #   tag_type  : Tag function, such as PeopleCounting, ApproachAway, or Noise.
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

  tag = TagConfig()
  tag.tag_index = TAG_GAME
  tag.tag_type = c4004.TAG_PEOPLE_COUNTING
  tag.scope_type = c4004.TAG_RANGE_CIRCLE
  tag.io_index = 0
  tag.center_x = -100
  tag.center_y = 550
  tag.width = 80
  tag.height = 0
  tags.append(tag)

  tag = TagConfig()
  tag.tag_index = TAG_SOFA
  tag.tag_type = c4004.TAG_PEOPLE_COUNTING
  tag.scope_type = c4004.TAG_RANGE_RECTANGLE
  tag.io_index = 0
  tag.center_x = 100
  tag.center_y = 450
  tag.width = 100
  tag.height = 300
  tags.append(tag)

  tag = TagConfig()
  tag.tag_index = TAG_HOME_DOOR
  tag.tag_type = c4004.TAG_APPROACH_AWAY
  tag.scope_type = c4004.TAG_RANGE_RECTANGLE
  tag.io_index = 0
  tag.center_x = 100
  tag.center_y = 700
  tag.width = 80
  tag.height = 40
  tags.append(tag)

  tag = TagConfig()
  tag.tag_index = TAG_KITCHEN_DOOR
  tag.tag_type = c4004.TAG_APPROACH_AWAY
  tag.scope_type = c4004.TAG_RANGE_RECTANGLE
  tag.io_index = 0
  tag.center_x = -100
  tag.center_y = 700
  tag.width = 80
  tag.height = 40
  tags.append(tag)

  tag = TagConfig()
  tag.tag_index = TAG_DINING
  tag.tag_type = c4004.TAG_PEOPLE_COUNTING
  tag.scope_type = c4004.TAG_RANGE_RECTANGLE
  tag.io_index = 0
  tag.center_x = 50
  tag.center_y = 150
  tag.width = 300
  tag.height = 150
  tags.append(tag)

  tag = TagConfig()
  tag.tag_index = TAG_CURTAIN
  tag.tag_type = c4004.TAG_NOISE
  tag.scope_type = c4004.TAG_RANGE_RECTANGLE
  tag.io_index = 0
  tag.center_x = -150
  tag.center_y = 300
  tag.width = 50
  tag.height = 300
  tags.append(tag)

  tag = TagConfig()
  tag.tag_index = TAG_PLANT
  tag.tag_type = c4004.TAG_NOISE
  tag.scope_type = c4004.TAG_RANGE_CIRCLE
  tag.io_index = 0
  tag.center_x = -50
  tag.center_y = 400
  tag.width = 40
  tag.height = 0
  tags.append(tag)

  if c4004.set_tags_from_config(tags):
    print('Set 7 tags from config success.')
  else:
    print('Set 7 tags from config failed.')

  if init_tag_cache_from_device():
    print('Init tag cache from device config success.')
  else:
    print('No device tag config read, tag cache uses default empty values.')

  print('===================================================================')
  print('Room occupancy inference started.')
  print('Rule 1: Game area has person -> TV IO HIGH immediately; no person for 5s -> LOW.')
  print('Rule 2: Sofa static-only for 5s -> Light PWM 150; motion for 5s -> 0; no person for 5s -> 255.')
  print('===================================================================')


def print_tag_cache_table():
  print('===================================================================')
  print('Tag Cache Table')
  print('Idx\tName\t\tType\t\tIO\tCenterX\tCenterY\tMotion\tStatic\tDir\tBoundary')
  for i in range(TAG_TOTAL):
    info = tag_cache[i]
    name = TAG_NAMES[i]
    type_text = tag_type_to_text(info['tag_type'])

    motion_num = info['motion_num'] if info['tag_type'] == c4004.TAG_PEOPLE_COUNTING else 0
    static_num = info['static_num'] if info['tag_type'] == c4004.TAG_PEOPLE_COUNTING else 0
    motion_dir = str(info['motion_dir']) if info['tag_type'] == c4004.TAG_APPROACH_AWAY else '-'
    enter_exit = str(info['enter_exit']) if info['tag_type'] == c4004.TAG_BOUNDARY else '-'

    line = '%d\t%s' % (i, name)
    if len(name) < 8:
      line += '\t'
    line += '\t%s' % type_text
    if len(type_text) < 8:
      line += '\t'
    line += '\t%d\t%d\t%d\t%d\t%d\t%s\t%s' % (
      info['io_index'],
      info['center_x'],
      info['center_y'],
      motion_num,
      static_num,
      motion_dir,
      enter_exit,
    )
    print(line)

  print('TV IO:\t%s' % ('HIGH' if tv_output_high else 'LOW'))
  print('Light PWM:\t%d' % light_output_value)


def main():
  global tag_print_pending
  global tv_output_high
  global light_output_value
  global game_no_person_start_s
  global sofa_static_start_s
  global sofa_motion_start_s
  global sofa_empty_start_s

  setup_gpio()
  setup_sensor_and_tags()

  last_print_s = 0.0
  try:
    while True:
      now_s = time.time()
      event = c4004.get_reported_info(0.1)

      if event == c4004.EVENT_TAG:
        info = c4004.get_tag_info()
        if info is not None and 0 <= info.tag_index < TAG_TOTAL:
          cache = tag_cache[info.tag_index]
          cache['tag_index'] = info.tag_index
          cache['tag_type'] = info.tag_type
          cache['io_index'] = info.io_index
          cache['center_x'] = info.center_x
          cache['center_y'] = info.center_y
          cache['enter_exit'] = info.enter_exit
          cache['motion_dir'] = info.motion_dir
          cache['motion_num'] = info.motion_num
          cache['static_num'] = info.static_num
          tag_print_pending = True

      game_has_person = (tag_cache[TAG_GAME]['motion_num'] + tag_cache[TAG_GAME]['static_num']) > 0
      if game_has_person:
        tv_output_high = True
        game_no_person_start_s = 0
      elif tv_output_high:
        if game_no_person_start_s == 0:
          game_no_person_start_s = now_s
        elif now_s - game_no_person_start_s >= GAME_NO_PERSON_DELAY_S:
          tv_output_high = False
      else:
        game_no_person_start_s = 0

      sofa_static_only = tag_cache[TAG_SOFA]['static_num'] > 0 and tag_cache[TAG_SOFA]['motion_num'] == 0
      sofa_has_motion = tag_cache[TAG_SOFA]['motion_num'] > 0
      sofa_no_person = (tag_cache[TAG_SOFA]['static_num'] + tag_cache[TAG_SOFA]['motion_num']) == 0
      if sofa_static_only:
        if sofa_static_start_s == 0:
          sofa_static_start_s = now_s
        elif now_s - sofa_static_start_s >= SOFA_STATIC_DELAY_S:
          light_output_value = LIGHT_PWM_DIM
        sofa_motion_start_s = 0
        sofa_empty_start_s = 0
      elif sofa_has_motion:
        if sofa_motion_start_s == 0:
          sofa_motion_start_s = now_s
        elif now_s - sofa_motion_start_s >= SOFA_MOTION_DELAY_S:
          light_output_value = LIGHT_PWM_LOW
        sofa_static_start_s = 0
        sofa_empty_start_s = 0
      elif sofa_no_person:
        if sofa_empty_start_s == 0:
          sofa_empty_start_s = now_s
        elif now_s - sofa_empty_start_s >= SOFA_EMPTY_DELAY_S:
          light_output_value = LIGHT_PWM_HIGH
        sofa_static_start_s = 0
        sofa_motion_start_s = 0
      else:
        sofa_static_start_s = 0
        sofa_motion_start_s = 0
        sofa_empty_start_s = 0

      apply_outputs()

      if tag_print_pending or (now_s - last_print_s) >= 3.0:
        tag_print_pending = False
        last_print_s = now_s
        print_tag_cache_table()
  finally:
    if _pwm is not None:
      _pwm.stop()
    if GPIO is not None:
      GPIO.cleanup()


if __name__ == '__main__':
  main()
