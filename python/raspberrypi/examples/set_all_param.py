# -*- coding: utf-8 -*
'''!
  @file set_all_param.py
  @brief Configure and read back major DFRobot C4004 parameters.
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
from DFRobot_C4004 import DFRobot_C4004, FourSidedRange_t, Point

c4004 = DFRobot_C4004('/dev/ttyAMA0', 115200)


def main():
  while not c4004.begin():
    print('DFRobot C4004 begin failed, retrying...')
    time.sleep(1)
  print('DFRobot C4004 begin success.')

  print('===================Product Info===================')
  print('Current hardware version:', c4004.get_hardware_version())
  print('Current firmware version:', c4004.get_firmware_version())

  print('=================Set install info=================')
  if c4004.set_install_high(180):
    print('Set install high success!')
  else:
    print('Set install high failed!')
  time.sleep(0.05)

  device_high = c4004.get_install_high()
  if device_high > 0:
    print('Current install high(cm):', device_high)
  else:
    print('Read current install high failed.')

  print('==================Feature Switch==================')
  if c4004.set_presence_enable(True):
    print('Set presence enable success!')
  else:
    print('Set presence enable failed!')
  time.sleep(0.05)

  presence_enable = [False]
  if c4004.get_presence_enable(presence_enable):
    print('Current presence enable:', 'ON' if presence_enable[0] else 'OFF')
  else:
    print('Read current presence enable failed.')

  if c4004.set_trajectory_track_enable(True):
    print('Set trajectory track enable success!')
  else:
    print('Set trajectory track enable failed!')
  time.sleep(0.05)

  track_enable = [False]
  if c4004.get_trajectory_track_enable(track_enable):
    print('Current trajectory tracking function enable:', 'ON' if track_enable[0] else 'OFF')
  else:
    print('Read current trajectory tracking function enable failed.')

  if c4004.set_check_to_active_frames(7):
    print('Set check-to-active frames success!')
  else:
    print('Set check-to-active frames failed!')
  time.sleep(0.05)

  check_to_active_frames = [0]
  if c4004.get_check_to_active_frames(check_to_active_frames):
    print('Current check-to-active frames:', check_to_active_frames[0])
  else:
    print('Read current check-to-active frames failed.')

  if c4004.set_motion_led(True):
    print('Set motion LED success!')
  else:
    print('Set motion LED failed!')
  time.sleep(0.05)

  if c4004.set_trajectory_led(True):
    print('Set trajectory LED success!')
  else:
    print('Set trajectory LED failed!')
  time.sleep(0.05)

  print('Current motion LED:', 'ON' if c4004.get_motion_led() else 'OFF')
  print('Current trajectory LED:', 'ON' if c4004.get_trajectory_led() else 'OFF')

  range_info = FourSidedRange_t()
  range_info.mode = c4004.RANGE_FOUR_SIDE
  range_info.x_positive_cm = 200
  range_info.x_negative_cm = -200
  range_info.y_positive_cm = 700
  range_info.y_negative_cm = 0

  print('====================Range Param===================')
  if c4004.set_four_sided_range_mode(range_info):
    print('Set four sided range success!')
  else:
    print('Set four sided range failed!')
  time.sleep(0.05)

  mode = c4004.get_detection_range_mode()
  print('Current detection mode:', end=' ')
  if mode == c4004.RANGE_FOUR_SIDE:
    print('Four-side boundary')
  elif mode == c4004.RANGE_TRAJECTORY:
    print('Trajectory')
  else:
    print('Other')

  if mode == c4004.RANGE_FOUR_SIDE:
    current_range = FourSidedRange_t()
    if c4004.get_four_sided_range_mode(current_range):
      print('Current boundary x+/x-/y+/y- (cm): %d/%d/%d/%d' % (
        current_range.x_positive_cm,
        current_range.x_negative_cm,
        current_range.y_positive_cm,
        current_range.y_negative_cm))
    else:
      print('Read current boundary range failed.')
  else:
    print('Current mode is not four-side boundary, skip boundary range check.')

  # Set the trajectory detection range mode
  # if c4004.set_trajectory_range_mode(False):
  #   print('Set trajectory detection range mode success!')
  # else:
  #   print('Set trajectory detection range mode failed!')
  # time.sleep(0.05)
  #
  # points = []
  # point_count = [0]
  # if c4004.get_trajectory_range_mode(points, point_count):
  #   print('Current trajectory range query success.')
  #   print('Current trajectory points:', point_count[0])
  #   for i, point in enumerate(points):
  #     print('#%d x/y=%d/%d' % (i, point.x, point.y))
  # else:
  #   print('Current trajectory range query failed.')
  #
  # Set multi-point config by config-file mode points (mode 0x06)
  # cfg_points = [
  #   Point(200, 0),
  #   Point(200, 400),
  #   Point(-200, 400),
  #   Point(-200, 0)
  # ]
  # if c4004.set_config_file_mode_points(cfg_points):
  #   print('Set multi-point config points success!')
  # else:
  #   print('Set multi-point config points failed!')
  # time.sleep(0.05)

  # points = []
  # point_count = [0]
  # if c4004.get_config_file_mode_points(points, point_count):
  #   print('Current multi-point config query success.')
  #   print('Current multi-point config points:', point_count[0])
  #   for i, point in enumerate(points):
  #     print('#%d x/y=%d/%d' % (i, point.x, point.y))
  # else:
  #   print('Current multi-point config query failed.')

  print('================People Count Param================')
  if c4004.set_real_time_people_time(5):
    print('Set RealTimePeopleTime success!')
  else:
    print('Set RealTimePeopleTime failed!')
  time.sleep(0.05)

  if c4004.set_track_meters(50):
    print('Set TrackMeters success!')
  else:
    print('Set TrackMeters failed!')
  time.sleep(0.05)

  if c4004.set_track_exists_time(10):
    print('Set TrackExistsTime success!')
  else:
    print('Set TrackExistsTime failed!')
  time.sleep(0.05)

  if c4004.set_unmanned_time(30):
    print('Set UnmannedTime success!')
  else:
    print('Set UnmannedTime failed!')
  time.sleep(0.05)

  if c4004.clear_people_count():
    print('Clear people count success!')
  else:
    print('Clear people count failed!')
  time.sleep(0.05)

  print('Current RealTimePeopleTime(s):', c4004.get_real_time_people_time())
  print('Current TrackMeters(cm):', c4004.get_track_meters())
  print('Current TrackExistsTime(s):', c4004.get_track_exists_time())
  print('Current UnmannedTime(s):', c4004.get_unmanned_time())
  print('Current people count(active):', c4004.get_people_time(c4004.GET_DATA_ACTIVE))

  print('=======================Done=======================')


if __name__ == '__main__':
  main()
