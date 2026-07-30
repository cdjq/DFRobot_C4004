# -*- coding: utf-8 -*
'''!
@file read_zone_state_by_gpio.py
@brief Configure tag zones and read local GPIO presence states.
@details GPIO 1 is the whole area output. GPIO 2-6 are divided zone outputs.
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

c4004 = DFRobot_C4004('/dev/ttyAMA0', 115200)
zone_pins = [6, 13, 19, 26, 20, 21]


def main():
  if rpi_gpio is not None:
    rpi_gpio.setmode(rpi_gpio.BCM)
    for pin in zone_pins:
      rpi_gpio.setup(pin, rpi_gpio.IN)

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

  range_info = FourSidedRange()
  range_info.x_max = 200
  range_info.x_min = -200
  range_info.y_max = 700
  range_info.y_min = 0
  if c4004.set_four_sided_range_mode(range_info):
    print('Set boundary detection range success.')
  else:
    print('Set boundary detection range failed.')

  if c4004.clear_all_tags():
    print('Clear all tags success.')
  else:
    print('Clear all tags failed.')

  set_tags = []

  tag0 = DFRobot_TagConfig()
  tag0.tag_index = 0
  tag0.tag_type = c4004.TAG_PEOPLE_COUNTING
  tag0.scope_type = c4004.TAG_RANGE_RECTANGLE
  tag0.io_index = 2
  tag0.center_x = 0
  tag0.center_y = 100
  tag0.width = 120
  tag0.height = 120
  set_tags.append(tag0)

  tag1 = DFRobot_TagConfig()
  tag1.tag_index = 1
  tag1.tag_type = c4004.TAG_PEOPLE_COUNTING
  tag1.scope_type = c4004.TAG_RANGE_RECTANGLE
  tag1.io_index = 3
  tag1.center_x = 100
  tag1.center_y = 220
  tag1.width = 120
  tag1.height = 120
  set_tags.append(tag1)

  tag2 = DFRobot_TagConfig()
  tag2.tag_index = 2
  tag2.tag_type = c4004.TAG_PEOPLE_COUNTING
  tag2.scope_type = c4004.TAG_RANGE_CIRCLE
  tag2.io_index = 4
  tag2.center_x = -80
  tag2.center_y = 350
  tag2.width = 80
  tag2.height = 0
  # Note: For rectangle tags, width is the size along the X-axis and height is
  # the size along the Y-axis (unit: cm), relative to center_x/center_y.
  # For circle tags, width is the radius and height is ignored.
  set_tags.append(tag2)

  tag3 = DFRobot_TagConfig()
  tag3.tag_index = 3
  tag3.tag_type = c4004.TAG_PEOPLE_COUNTING
  tag3.scope_type = c4004.TAG_RANGE_RECTANGLE
  tag3.io_index = 5
  tag3.center_x = 0
  tag3.center_y = 500
  tag3.width = 160
  tag3.height = 160
  set_tags.append(tag3)

  tag4 = DFRobot_TagConfig()
  tag4.tag_index = 4
  tag4.tag_type = c4004.TAG_PEOPLE_COUNTING
  tag4.scope_type = c4004.TAG_RANGE_RECTANGLE
  tag4.io_index = 6
  tag4.center_x = -100
  tag4.center_y = 620
  tag4.width = 100
  tag4.height = 120
  set_tags.append(tag4)

  if c4004.set_tags_from_config(set_tags):
    print('Set 5 tags from config success.')
  else:
    print('Set 5 tags from config failed.')

  if c4004.set_occ_led(True):
    print('Set occupancy LED success.')
  else:
    print('Set occupancy LED failed.')

  if c4004.set_real_time_people_time(5):
    print('Set RealTimePeopleTime success.')
  else:
    print('Set RealTimePeopleTime failed.')

  try:
    last_print = 0
    while True:
      # When state or data changes and the corresponding report function is enabled,
      # the module pushes the update immediately as an event via get_reported_event().
      # Use the matching getter with GET_DATA_REPORT to read the cached value
      # updated by that report, without issuing an extra UART query.
      c4004.get_reported_event(0.05)
      if time.time() - last_print > 1:
        last_print = time.time()
        print('=============================================')
        print('GPIO presence (HIGH=Presence, LOW=None):')
        print('GPIO 1 = Whole area, GPIO 2-6 = Divided zones')
        if rpi_gpio is None:
          print('RPi.GPIO is not available.')
        else:
          for i, pin in enumerate(zone_pins):
            has_presence = rpi_gpio.input(pin) == rpi_gpio.HIGH
            if i == 0:
              label = 'GPIO 1 (Whole area)'
            else:
              label = 'GPIO %d (Zone %d)' % (i + 1, i)
            print('%s: %s' % (label, 'Presence' if has_presence else 'None'))
        print('=============================================')
  finally:
    if rpi_gpio is not None:
      rpi_gpio.cleanup()


if __name__ == '__main__':
  main()
