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
from DFRobot_C4004 import DFRobot_C4004, BoundaryDetectionRange, Point

c4004 = DFRobot_C4004('/dev/ttyAMA0', 115200)


def main():
  while not c4004.begin():
    print('DFRobot C4004 begin failed, retrying...')
    time.sleep(1)
  print('DFRobot C4004 begin success.')

  print('===================Product Info===================')
  print('Current product model:', c4004.get_product_model())
  print('Current product ID:', hex(c4004.get_product_id()))
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

  range_info = BoundaryDetectionRange()
  range_info.mode = c4004.RANGE_FOUR_SIDE_BOUNDARY
  range_info.x_positive_cm = 300
  range_info.x_negative_cm = -300
  range_info.y_positive_cm = 500
  range_info.y_negative_cm = 0

  print('====================Range Param===================')
  if c4004.set_boundary_detection_range(range_info):
    print('Set boundary detection range success!')
  else:
    print('Set boundary detection range failed!')
  time.sleep(0.05)

  mode = c4004.get_detection_range_mode()
  print('Current detection mode:', end=' ')
  if mode == c4004.RANGE_FOUR_SIDE_BOUNDARY:
    print('Four-side boundary')
  elif mode == c4004.RANGE_TRAJECTORY:
    print('Trajectory')
  else:
    print('Other')

  if mode == c4004.RANGE_FOUR_SIDE_BOUNDARY:
    current_range = BoundaryDetectionRange()
    if c4004.get_boundary_detection_range(current_range):
      print('Current boundary x+/x-/y+/y- (cm): %d/%d/%d/%d' % (
        current_range.x_positive_cm,
        current_range.x_negative_cm,
        current_range.y_positive_cm,
        current_range.y_negative_cm))
    else:
      print('Read current boundary range failed.')
  else:
    print('Current mode is not four-side boundary, skip boundary range check.')

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
  if c4004.set_people_report_interval(5):
    print('Set people report interval success!')
  else:
    print('Set people report interval failed!')
  time.sleep(0.05)

  if c4004.set_trajectory_generate_distance(50):
    print('Set trajectory generate distance success!')
  else:
    print('Set trajectory generate distance failed!')
  time.sleep(0.05)

  if c4004.set_trajectory_hold_time(10):
    print('Set trajectory hold time success!')
  else:
    print('Set trajectory hold time failed!')
  time.sleep(0.05)

  if c4004.set_no_person_delay(30):
    print('Set no person delay success!')
  else:
    print('Set no person delay failed!')
  time.sleep(0.05)

  if c4004.clear_people_count():
    print('Clear people count success!')
  else:
    print('Clear people count failed!')
  time.sleep(0.05)

  print('Current people report interval(s):', c4004.get_people_report_interval())
  print('Current trajectory generate distance(cm):', c4004.get_trajectory_generate_distance())
  print('Current trajectory hold time(s):', c4004.get_trajectory_hold_time())
  print('Current no person delay(s):', c4004.get_no_person_delay())
  print('Current people count(active):', c4004.get_people_count_info(c4004.GET_DATA_ACTIVE))

  print('=======================Done=======================')


if __name__ == '__main__':
  main()
