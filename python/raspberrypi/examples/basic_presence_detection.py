# -*- coding: utf-8 -*
'''!
@file basic_presence_detection.py
@brief Quickly check whether someone is in range, still or moving, and how many people are counted.
@details Use this example to quickly verify that the sensor can detect whether someone is in the
@n detection range, whether they are static or moving, and how many people are counted.
@n Run in a terminal to view live status prints.
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
from DFRobot_C4004 import DFRobot_C4004, FourSidedRange

port = '/dev/ttyAMA0'
c4004 = DFRobot_C4004(port, 115200)


def main():
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

  if c4004.set_occ_led(True):
    print('Set occupancy LED success.')
  else:
    print('Set occupancy LED failed.')

  if c4004.set_trk_led(True):
    print('Set trajectory LED success.')
  else:
    print('Set trajectory LED failed.')

  range_info = FourSidedRange()
  range_info.x_max = 200
  range_info.x_min = -200
  range_info.y_max = 700
  range_info.y_min = 0
  if c4004.set_four_sided_range_mode(range_info):
    print('Set boundary detection range success.')
  else:
    print('Set boundary detection range failed.')

  if c4004.set_real_time_report_interval(2):
    print('Set RealTimeReportInterval success.')
  else:
    print('Set RealTimeReportInterval failed.')

  if c4004.set_presence_enable(True):
    print('Set presence enable success.')
  else:
    print('Set presence enable failed.')

  clear_count_on_boot = False
  if clear_count_on_boot:
    if c4004.clear_live_count():
      print('Clear people count success.')
    else:
      print('Clear people count failed.')

  last_query = 0
  while True:
    # timeout=0.03: wait up to 30 ms for one UART report frame from the sensor.
    # Returns EVENT_NONE if no complete frame arrives within that time.
    # When state or data changes and the corresponding report function is enabled,
    # the module pushes the update immediately as an event via get_reported_event().
    # Use the matching getter with GET_DATA_REPORT to read the cached value
    # updated by that report, without issuing an extra UART query.
    event = c4004.get_reported_event(0.03)
    if event == c4004.EVENT_PRESENCE:
      presence = c4004.get_presence_state(c4004.GET_DATA_REPORT)
      if presence == c4004.NO_PRESENCE:
        print('Presence state: No Person Detected')
      elif presence == c4004.PRESENCE:
        print('Presence state: Presence')
    elif event == c4004.EVENT_MOTION:
      motion = c4004.get_motion_state(c4004.GET_DATA_REPORT)
      if motion == c4004.MOTION_STATIC:
        print('Motion state: Static')
      elif motion == c4004.MOTION_ACTIVE:
        print('Motion state: Motion')
      else:
        print('Motion state: No Target Detected')
    elif event == c4004.EVENT_PEOPLE_COUNT:
      count = c4004.get_live_count(c4004.GET_DATA_REPORT)
      print('Live Count:', count)

    # Every 3 s, actively poll and print people count / presence / motion (not event-driven).
    if time.time() - last_query > 3:
      last_query = time.time()
      print('Live Count:', c4004.get_live_count(c4004.GET_DATA_REPORT))

      query_presence = c4004.get_presence_state(c4004.GET_DATA_ACTIVE)
      if query_presence == c4004.NO_PRESENCE:
        print('Presence state: No Person Detected')
      elif query_presence == c4004.PRESENCE:
        print('Presence state: Presence')

      query_motion = c4004.get_motion_state(c4004.GET_DATA_ACTIVE)
      if query_motion == c4004.MOTION_NONE:
        print('Motion state: No Target Detected')
      elif query_motion == c4004.MOTION_STATIC:
        print('Motion state: Static')
      elif query_motion == c4004.MOTION_ACTIVE:
        print('Motion state: Motion')


if __name__ == '__main__':
  main()
