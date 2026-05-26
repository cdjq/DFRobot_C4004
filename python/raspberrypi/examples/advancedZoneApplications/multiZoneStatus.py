# -*- coding: utf-8 -*
'''!
  @file multiZoneStatus.py
  @brief Configure multi-zone tags and print tag event status for living-room scene linkage.
  @details This routine configures 5 monitoring tags and 2 noise tags, keeps the last event
  @n result for each tag, prints a summary table every 3 seconds or when a tag event arrives,
  @n and drives outputs based on game-area and sofa-area people counting results.
  @copyright Copyright (c) 2026 DFRobot Co.Ltd (http://www.dfrobot.com)
  @license The MIT License (MIT)
  @author JiaLi(zhixin.liu@dfrobot.com)
  @version V1.0.0
  @date 2026-05-25
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

from DFRobot_C4004 import DFRobot_C4004, TagConfig, BoundaryDetectionRange

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

GAME_NO_PERSON_DELAY_S = 60.0
SOFA_STATIC_DELAY_S = 30.0
SOFA_MOTION_DELAY_S = 10.0
SOFA_EMPTY_DELAY_S = 30.0
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
  if tag_type == c4004.TAG_TYPE_NONE:
    return 'None'
  if tag_type == c4004.TAG_TYPE_ENTER_EXIT:
    return 'EnterExit'
  if tag_type == c4004.TAG_TYPE_APPROACH_AWAY:
    return 'ApproachAway'
  if tag_type == c4004.TAG_TYPE_PEOPLE_COUNTING:
    return 'PeopleCount'
  if tag_type == c4004.TAG_TYPE_NOISE:
    return 'Noise'
  return 'Unknown'


def make_empty_cache(index):
  return {
    'index': index,
    'type': c4004.TAG_TYPE_NONE,
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
    index = tag.index
    if index < 0 or index >= TAG_TOTAL:
      continue
    tag_cache[index]['index'] = index
    tag_cache[index]['type'] = tag.type
    tag_cache[index]['center_x'] = tag.center_x
    tag_cache[index]['center_y'] = tag.center_y
    if index in (TAG_HOME_DOOR, TAG_KITCHEN_DOOR):
      tag_cache[index]['motion_dir'] = 1


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

  if c4004.set_presence_enable(True):
    print('Set presence enable success.')
  else:
    print('Set presence enable failed.')

  range_info = BoundaryDetectionRange()
  range_info.mode = c4004.RANGE_FOUR_SIDE_BOUNDARY
  range_info.x_positive_cm = 500
  range_info.x_negative_cm = -500
  range_info.y_positive_cm = 800
  range_info.y_negative_cm = 0
  if c4004.set_boundary_detection_range(range_info):
    print('Set boundary detection range success.')
  else:
    print('Set boundary detection range failed.')

  if c4004.clear_all_tags():
    print('Clear all tags success.')
  else:
    print('Clear all tags failed.')

  tags = []

  tag = TagConfig()
  tag.index = TAG_GAME
  tag.type = c4004.TAG_TYPE_PEOPLE_COUNTING
  tag.range_type = c4004.TAG_RANGE_CIRCLE
  tag.center_x = 50
  tag.center_y = 450
  tag.x_size = 100
  tag.y_size = 0
  tags.append(tag)

  tag = TagConfig()
  tag.index = TAG_SOFA
  tag.type = c4004.TAG_TYPE_PEOPLE_COUNTING
  tag.range_type = c4004.TAG_RANGE_RECTANGLE
  tag.center_x = 300
  tag.center_y = 550
  tag.x_size = 100
  tag.y_size = 300
  tags.append(tag)

  tag = TagConfig()
  tag.index = TAG_HOME_DOOR
  tag.type = c4004.TAG_TYPE_APPROACH_AWAY
  tag.range_type = c4004.TAG_RANGE_RECTANGLE
  tag.center_x = 100
  tag.center_y = 700
  tag.x_size = 80
  tag.y_size = 40
  tags.append(tag)

  tag = TagConfig()
  tag.index = TAG_KITCHEN_DOOR
  tag.type = c4004.TAG_TYPE_APPROACH_AWAY
  tag.range_type = c4004.TAG_RANGE_RECTANGLE
  tag.center_x = -100
  tag.center_y = 700
  tag.x_size = 80
  tag.y_size = 40
  tags.append(tag)

  tag = TagConfig()
  tag.index = TAG_DINING
  tag.type = c4004.TAG_TYPE_PEOPLE_COUNTING
  tag.range_type = c4004.TAG_RANGE_RECTANGLE
  tag.center_x = 150
  tag.center_y = 200
  tag.x_size = 400
  tag.y_size = 200
  tags.append(tag)

  tag = TagConfig()
  tag.index = TAG_CURTAIN
  tag.type = c4004.TAG_TYPE_NOISE
  tag.range_type = c4004.TAG_RANGE_RECTANGLE
  tag.center_x = -250
  tag.center_y = 400
  tag.x_size = 50
  tag.y_size = 400
  tags.append(tag)

  tag = TagConfig()
  tag.index = TAG_PLANT
  tag.type = c4004.TAG_TYPE_NOISE
  tag.range_type = c4004.TAG_RANGE_CIRCLE
  tag.center_x = -200
  tag.center_y = 650
  tag.x_size = 40
  tag.y_size = 0
  tags.append(tag)

  if c4004.set_tags_from_config(tags):
    print('Set 7 tags from config success.')
  else:
    print('Set 7 tags from config failed.')

  init_tag_cache_from_config(tags)

  print('===================================================================')
  print('Room occupancy inference started.')
  print('Rule 1: Game area has person -> TV IO HIGH immediately; no person for 60s -> LOW.')
  print('Rule 2: Sofa static-only for 30s -> Light PWM 150; motion for 10s -> 0; no person for 30s -> 255.')
  print('===================================================================')


def print_tag_cache_table():
  print('===================================================================')
  print('Tag Cache Table')
  print('Idx\tName\t\tType\t\tCenterX\tCenterY\tMotion\tStatic\tDir\tEnterExit')
  for i in range(TAG_TOTAL):
    info = tag_cache[i]
    name = TAG_NAMES[i]
    type_text = tag_type_to_text(info['type'])

    motion_num = info['motion_num'] if info['type'] == c4004.TAG_TYPE_PEOPLE_COUNTING else 0
    static_num = info['static_num'] if info['type'] == c4004.TAG_TYPE_PEOPLE_COUNTING else 0
    motion_dir = str(info['motion_dir']) if info['type'] == c4004.TAG_TYPE_APPROACH_AWAY else '-'
    enter_exit = str(info['enter_exit']) if info['type'] == c4004.TAG_TYPE_ENTER_EXIT else '-'

    line = '%d\t%s' % (i, name)
    if len(name) < 8:
      line += '\t'
    line += '\t%s' % type_text
    if len(type_text) < 8:
      line += '\t'
    line += '\t%d\t%d\t%d\t%d\t%s\t%s' % (
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
        if info is not None and 0 <= info.index < TAG_TOTAL:
          cache = tag_cache[info.index]
          cache['index'] = info.index
          cache['type'] = info.type
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
