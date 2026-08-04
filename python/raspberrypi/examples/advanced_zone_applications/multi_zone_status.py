# -*- coding: utf-8 -*
'''!
  @file multi_zone_status.py
  @brief Read multi-zone tag events and print living-room scene linkage status.
  @details This routine can use tags configured by the PC tool or optionally configure
  @n tags in code. It keeps the last event result for each tag, prints a summary table
  @n every 3 seconds or when a tag event arrives, and drives outputs based on
  @n game-area and sofa-area people counting results.
  @n Usage environment:
  @n - Please install the sensor at a height of 180 cm for use.
  @copyright Copyright (c) 2026 DFRobot Co.Ltd (http://www.dfrobot.com)
  @license The MIT License (MIT)
  @author JiaLi(jia.li@dfrobot.com)
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

from DFRobot_C4004 import DFRobot_C4004, DFRobot_TagConfig, FourSidedRange

try:
  import RPi.GPIO as rpi_gpio
except ImportError:
  rpi_gpio = None

port = '/dev/ttyAMA0'
c4004 = DFRobot_C4004(port, 115200)

tv_ctrl_pin = 2
light_ctrl_pin = 3

tag_game = 0
tag_sofa = 1
tag_home_door = 2
tag_kitchen_door = 3
tag_dining = 4
tag_curtain = 5
tag_plant = 6
tag_total = 7

# Users can adjust these times according to their own preferences, needs,
# application scenarios, etc. The default time is 5 seconds.
game_no_person_delay_s = 5.0   # Delay before the game area is treated as empty.
sofa_static_delay_s = 5.0      # Delay before the sofa area is treated as static.
sofa_motion_delay_s = 5.0      # Delay before the sofa area is treated as motion.
sofa_empty_delay_s = 5.0       # Delay before the sofa area is treated as empty.
light_pwm_low = 0
light_pwm_dim = 150
light_pwm_high = 255

tag_names = ['Game', 'Sofa', 'HomeDoor', 'KitchenDoor', 'Dining', 'CurtainNoise', 'PlantNoise']

# Tag cache entries keep one snapshot per tag index.
tag_cache = []
tag_print_pending = False
tv_output_high = False
light_output_value = light_pwm_low
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


def boundary_direction_to_text(value):
  if value == c4004.ENTER:
    return 'Enter'
  if value == c4004.EXIT:
    return 'Exit'
  return 'None'


def approach_away_direction_to_text(value):
  if value == c4004.APPROACH:
    return 'Approach'
  if value == c4004.AWAY:
    return 'Away'
  return 'None'


def make_empty_cache(index):
  return {
    'tag_index': index,
    'tag_type': c4004.TAG_NONE,
    'io_index': 0,
    'center_x': 0,
    'center_y': 0,
    'enter_exit': c4004.DIR_NONE,
    'motion_dir': c4004.DIR_NONE,
    'motion_num': 0,
    'static_num': 0,
  }


def init_tag_cache_from_config(tags):
  global tag_cache
  tag_cache = [make_empty_cache(i) for i in range(tag_total)]
  for tag in tags:
    index = tag.tag_index
    if index < 0 or index >= tag_total:
      continue
    tag_cache[index]['tag_index'] = index
    tag_cache[index]['tag_type'] = tag.tag_type
    tag_cache[index]['io_index'] = tag.io_index
    tag_cache[index]['center_x'] = tag.center_x
    tag_cache[index]['center_y'] = tag.center_y


def init_tag_cache_from_device():
  tags = c4004.get_tags()
  init_tag_cache_from_config(tags)
  print('Read tag config count: %d' % len(tags))
  return len(tags) > 0


def setup_gpio():
  global _pwm
  if rpi_gpio is None:
    print('RPi.GPIO is not available, running without physical output control.')
    return

  rpi_gpio.setmode(rpi_gpio.BCM)
  rpi_gpio.setup(tv_ctrl_pin, rpi_gpio.OUT)
  rpi_gpio.setup(light_ctrl_pin, rpi_gpio.OUT)
  rpi_gpio.output(tv_ctrl_pin, rpi_gpio.LOW)

  # Map 0~255 logical brightness to PWM duty on GPIO.
  _pwm = rpi_gpio.PWM(light_ctrl_pin, 1000)
  _pwm.start(0)


def apply_outputs():
  if rpi_gpio is None:
    return

  rpi_gpio.output(tv_ctrl_pin, rpi_gpio.HIGH if tv_output_high else rpi_gpio.LOW)

  if _pwm is not None:
    duty = max(0.0, min(100.0, float(light_output_value) * 100.0 / 255.0))
    _pwm.ChangeDutyCycle(duty)
  else:
    rpi_gpio.output(light_ctrl_pin, rpi_gpio.HIGH if light_output_value > 0 else rpi_gpio.LOW)


def setup_sensor_and_tags():
  while not c4004.begin():
    print('DFRobot C4004 begin failed, retrying...')
    time.sleep(1)
  print('DFRobot C4004 begin success.')

  # Side mount: default 180 cm, recommended 180±20 cm. Top mount: recommended 220-280 cm.
  if c4004.set_install_height(180):
    print('Set install height success.')
  else:
    print('Set install height failed.')
  time.sleep(0.05)

  if c4004.set_frame_generate_count(7):
    print('Set check-to-active frames success.')
  else:
    print('Set check-to-active frames failed.')
  time.sleep(0.05)

  if c4004.set_presence_enable(True):
    print('Set presence enable success.')
  else:
    print('Set presence enable failed.')

  range_info = FourSidedRange()
  range_info.x_max = 200
  range_info.x_min = -200
  range_info.y_max = 700
  range_info.y_min = 0
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
  #   scope_type: Tag shape. Use RECTANGLE or CIRCLE.
  #   io_index  : IO linkage index. 0 means unused; 2-6 maps to IO2-IO6.
  #   center_x  : Tag center X coordinate, in cm.
  #   center_y  : Tag center Y coordinate, in cm.
  #   width     : Rectangle: size along X-axis (cm); Circle: radius (cm).
  #   height    : Rectangle: size along Y-axis (cm); Circle: ignored.

  if c4004.clear_all_tags():
    print('Clear all tags success.')
  else:
    print('Clear all tags failed.')

  tags = []

  tag = DFRobot_TagConfig()
  tag.tag_index = tag_game
  tag.tag_type = c4004.TAG_PEOPLE_COUNTING
  tag.scope_type = c4004.CIRCLE
  tag.io_index = 0
  tag.center_x = -100
  tag.center_y = 550
  tag.width = 80
  tag.height = 0
  tags.append(tag)

  tag = DFRobot_TagConfig()
  tag.tag_index = tag_sofa
  tag.tag_type = c4004.TAG_PEOPLE_COUNTING
  tag.scope_type = c4004.RECTANGLE
  tag.io_index = 0
  tag.center_x = 100
  tag.center_y = 450
  tag.width = 100
  tag.height = 300
  tags.append(tag)

  tag = DFRobot_TagConfig()
  tag.tag_index = tag_home_door
  tag.tag_type = c4004.TAG_BOUNDARY
  tag.scope_type = c4004.RECTANGLE
  tag.io_index = 0
  tag.center_x = 100
  tag.center_y = 700
  tag.width = 80
  tag.height = 40
  tags.append(tag)

  tag = DFRobot_TagConfig()
  tag.tag_index = tag_kitchen_door
  tag.tag_type = c4004.TAG_BOUNDARY
  tag.scope_type = c4004.RECTANGLE
  tag.io_index = 0
  tag.center_x = -100
  tag.center_y = 700
  tag.width = 80
  tag.height = 40
  tags.append(tag)

  tag = DFRobot_TagConfig()
  tag.tag_index = tag_dining
  tag.tag_type = c4004.TAG_PEOPLE_COUNTING
  tag.scope_type = c4004.RECTANGLE
  tag.io_index = 0
  tag.center_x = 50
  tag.center_y = 150
  tag.width = 300
  tag.height = 150
  tags.append(tag)

  tag = DFRobot_TagConfig()
  tag.tag_index = tag_curtain
  tag.tag_type = c4004.TAG_NOISE
  tag.scope_type = c4004.RECTANGLE
  tag.io_index = 0
  tag.center_x = -150
  tag.center_y = 300
  tag.width = 50
  tag.height = 300
  tags.append(tag)

  tag = DFRobot_TagConfig()
  tag.tag_index = tag_plant
  tag.tag_type = c4004.TAG_NOISE
  tag.scope_type = c4004.CIRCLE
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
  for i in range(tag_total):
    info = tag_cache[i]
    name = tag_names[i]
    type_text = tag_type_to_text(info['tag_type'])

    motion_num = info['motion_num'] if info['tag_type'] == c4004.TAG_PEOPLE_COUNTING else 0
    static_num = info['static_num'] if info['tag_type'] == c4004.TAG_PEOPLE_COUNTING else 0
    motion_dir = approach_away_direction_to_text(info['motion_dir']) if info['tag_type'] == c4004.TAG_APPROACH_AWAY else '-'
    enter_exit = boundary_direction_to_text(info['enter_exit']) if info['tag_type'] == c4004.TAG_BOUNDARY else '-'

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

  print('TV IO level:\t%s' % ('HIGH' if tv_output_high else 'LOW'))
  print('Light PWM value:\t%d' % light_output_value)


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

      event = c4004.get_reported_event(0.1)

      if event == c4004.EVENT_TAG:
        info = c4004.get_tag_info()
        if info is not None and 0 <= info.tag_index < tag_total:
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

      game_has_person = (tag_cache[tag_game]['motion_num'] + tag_cache[tag_game]['static_num']) > 0
      if game_has_person:
        tv_output_high = True
        game_no_person_start_s = 0
      elif tv_output_high:
        if game_no_person_start_s == 0:
          game_no_person_start_s = now_s
        elif now_s - game_no_person_start_s >= game_no_person_delay_s:
          tv_output_high = False
      else:
        game_no_person_start_s = 0

      sofa_static_only = tag_cache[tag_sofa]['static_num'] > 0 and tag_cache[tag_sofa]['motion_num'] == 0
      sofa_has_motion = tag_cache[tag_sofa]['motion_num'] > 0
      sofa_no_person = (tag_cache[tag_sofa]['static_num'] + tag_cache[tag_sofa]['motion_num']) == 0
      if sofa_static_only:
        if sofa_static_start_s == 0:
          sofa_static_start_s = now_s
        elif now_s - sofa_static_start_s >= sofa_static_delay_s:
          light_output_value = light_pwm_dim
        sofa_motion_start_s = 0
        sofa_empty_start_s = 0
      elif sofa_has_motion:
        if sofa_motion_start_s == 0:
          sofa_motion_start_s = now_s
        elif now_s - sofa_motion_start_s >= sofa_motion_delay_s:
          light_output_value = light_pwm_low
        sofa_static_start_s = 0
        sofa_empty_start_s = 0
      elif sofa_no_person:
        if sofa_empty_start_s == 0:
          sofa_empty_start_s = now_s
        elif now_s - sofa_empty_start_s >= sofa_empty_delay_s:
          light_output_value = light_pwm_high
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
    if rpi_gpio is not None:
      rpi_gpio.cleanup()


if __name__ == '__main__':
  main()
