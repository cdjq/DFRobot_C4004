# -*- coding: utf-8 -*
'''!
@file basic_presence_detection.py
@brief Enable presence detection and print presence, motion and people-count reports.
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
from DFRobot_C4004 import DFRobot_C4004, FourSidedRange_t

PORT = '/dev/ttyAMA0'
c4004 = DFRobot_C4004(PORT, 115200)


def main():
  while not c4004.begin():
    print('DFRobot C4004 begin failed, retrying...')
    time.sleep(1)
  print('DFRobot C4004 begin success.')

  if c4004.set_check_to_active_frames(7):
    print('Set check-to-active frames success.')
  else:
    print('Set check-to-active frames failed.')
  time.sleep(0.05)

  if c4004.set_motion_led(True):
    print('Set motion LED success.')
  else:
    print('Set motion LED failed.')

  if c4004.set_trajectory_led(True):
    print('Set trajectory LED success.')
  else:
    print('Set trajectory LED failed.')

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

  if c4004.set_real_time_people_time(2):
    print('Set RealTimePeopleTime success.')
  else:
    print('Set RealTimePeopleTime failed.')

  if c4004.set_presence_enable(True):
    print('Set presence enable success.')
  else:
    print('Set presence enable failed.')

  clear_count_on_boot = False
  if clear_count_on_boot:
    if c4004.clear_people_count():
      print('Clear people count success.')
    else:
      print('Clear people count failed.')

  last_query = 0
  while True:
    # When state or data changes and the corresponding report function is enabled,
    # the module pushes the update immediately as an event via get_reported_info().
    # Use the matching getter with GET_DATA_REPORT to read the cached value
    # updated by that report, without issuing an extra UART query.
    event = c4004.get_reported_info(0.05)
    if event == c4004.EVENT_PRESENCE:
      presence = c4004.get_presence_state(c4004.GET_DATA_REPORT)
      if presence == c4004.NO_PRESENCE:
        print('Presence state: None')
      elif presence == c4004.PRESENCE:
        print('Presence state: Presence')
    elif event == c4004.EVENT_MOTION:
      motion = c4004.get_motion_state(c4004.GET_DATA_REPORT)
      if motion == c4004.MOTION_STATIC:
        print('Motion state: Static')
      elif motion == c4004.MOTION_ACTIVE:
        print('Motion state: Motion')
      else:
        print('Motion state: None')
    elif event == c4004.EVENT_PEOPLE_COUNT:
      count = c4004.get_people_time(c4004.GET_DATA_REPORT)
      print('People count:', count)

    if time.time() - last_query > 2:
      last_query = time.time()
      print('People count:', c4004.get_people_time(c4004.GET_DATA_REPORT))

      query_presence = c4004.get_presence_state(c4004.GET_DATA_ACTIVE)
      if query_presence == c4004.NO_PRESENCE:
        print('Presence state: None')
      elif query_presence == c4004.PRESENCE:
        print('Presence state: Presence')

      query_motion = c4004.get_motion_state(c4004.GET_DATA_ACTIVE)
      if query_motion == c4004.MOTION_NONE:
        print('Motion state: None')
      elif query_motion == c4004.MOTION_STATIC:
        print('Motion state: Static')
      elif query_motion == c4004.MOTION_ACTIVE:
        print('Motion state: Motion')


if __name__ == '__main__':
  main()
