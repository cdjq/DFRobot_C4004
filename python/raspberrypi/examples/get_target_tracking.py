# -*- coding: utf-8 -*
'''!
@file get_target_tracking.py
@brief Print live tracked targets in range: position, motion type, speed, and related data.
@details Use this example to view tracked targets in the detection range, including position,
@n motion feature, speed and related tracking data printed to the terminal.
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

c4004 = DFRobot_C4004('/dev/ttyAMA0', 115200)


def target_feature_to_string(feature):
  if feature == c4004.STATIC:
    return 'Static'
  if feature == c4004.MOTION:
    return 'Motion'
  if feature == c4004.UNCERTAIN:
    return 'Unsure'
  return 'Unknown'


def print_trajectory_data(data_mode):
  if data_mode == c4004.GET_DATA_ACTIVE:
    title = '======================TrajectoryActive======================='
    mode_text = 'Active Query'
  else:
    title = '======================TrajectoryReport======================='
    mode_text = 'Passive Report'

  targets = c4004.get_target_list(data_mode)
  count = len(targets)
  print(title)
  print('Mode:', mode_text)
  print('Target Count:', count)
  if count == 0:
    print('No target.')
  else:
    print('Row\tIndex\tKinesia\tFeature\tX(cm)\tY(cm)\tSpeed(cm/s)')
    for i in range(min(count, len(targets))):
      target = targets[i]
      print('%d\t%d\t%d\t%s\t%d\t%d\t%d' % (i, target.index, target.kinesia, target_feature_to_string(target.target_feature), target.pos_x, target.pos_y, target.speed))
  print('')


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

  range_info = FourSidedRange()
  range_info.x_max = 200
  range_info.x_min = -200
  range_info.y_max = 700
  range_info.y_min = 0
  if c4004.set_four_sided_range_mode(range_info):
    print('Set boundary detection range success.')
  else:
    print('Set boundary detection range failed.')

  if c4004.set_trajectory_track_enable(True):
    print('Set trajectory track enable success.')
  else:
    print('Set trajectory track enable failed.')

  if c4004.set_occ_led(True):
    print('Set occupancy LED success.')
  else:
    print('Set occupancy LED failed.')

  if c4004.set_trk_led(True):
    print('Set trajectory LED success.')
  else:
    print('Set trajectory LED failed.')

  last_query = 0
  while True:
    # When state or data changes and the corresponding report function is enabled,
    # the module pushes the update immediately as an event via get_reported_event().
    # Use the matching getter with GET_DATA_REPORT to read the cached value
    # updated by that report, without issuing an extra UART query.
    event = c4004.get_reported_event(0.1)
    if event == c4004.EVENT_TRAJECTORY:
      print_trajectory_data(c4004.GET_DATA_REPORT)

    if time.time() - last_query > 4:
      last_query = time.time()
      print_trajectory_data(c4004.GET_DATA_ACTIVE)


if __name__ == '__main__':
  main()
