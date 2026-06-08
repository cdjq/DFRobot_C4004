# -*- coding: utf-8 -*
'''!
  @file read_target_trajectory.py
  @brief Enable trajectory tracking and print target trajectory information.
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

c4004 = DFRobot_C4004('/dev/ttyAMA0', 115200)


def target_feature_to_string(feature):
  if feature == c4004.STATIC:
    return 'Static'
  if feature == c4004.MOTION:
    return 'Motion'
  if feature == c4004.UNCERTAIN:
    return 'Uncertain'
  return 'Unknown'


def print_trajectory_data(data_mode):
  if data_mode == c4004.GET_DATA_ACTIVE:
    title = '======================TrajectoryActive======================='
    mode_text = 'Active Query'
  else:
    title = '======================TrajectoryReport======================='
    mode_text = 'Passive Report'

  targets = c4004.get_target_list(data_mode)
  print(title)
  print('Mode:', mode_text)
  print('Target Count:', len(targets))
  if len(targets) == 0:
    print('No target.')
  else:
    print('Row\tID\tKinesia\tFeature\tX\tY\tSpeed')
    for i, target in enumerate(targets):
      print('%d\t%d\t%d\t%s\t%d\t%d\t%d' % (
        i,
        target.index,
        target.kinesia,
        target_feature_to_string(target.target_feature),
        target.x,
        target.y,
        target.speed))
  print('')


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

  if c4004.set_trajectory_track_enable(True):
    print('Set trajectory track enable success.')
  else:
    print('Set trajectory track enable failed.')

  if c4004.set_motion_led(True):
    print('Set motion LED success.')
  else:
    print('Set motion LED failed.')

  if c4004.set_trajectory_led(True):
    print('Set trajectory LED success.')
  else:
    print('Set trajectory LED failed.')

  last_query = 0
  while True:
    event = c4004.get_reported_info(0.1)
    if event == c4004.EVENT_TRAJECTORY:
      print_trajectory_data(c4004.GET_DATA_REPORT)

    if time.time() - last_query > 4:
      last_query = time.time()
      print_trajectory_data(c4004.GET_DATA_ACTIVE)


if __name__ == '__main__':
  main()
