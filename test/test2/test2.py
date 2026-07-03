# -*- coding: utf-8 -*
'''!
  @file test2.py
  @brief Test set_install_info() and get_install_info().
'''

import os
import sys
import time

cur_path = os.path.dirname(os.path.abspath(__file__))
lib_root = os.path.dirname(os.path.dirname(cur_path))
sys.path.insert(0, os.path.join(lib_root, 'python', 'raspberrypi'))
from DFRobot_C4004 import DFRobot_C4004, InstallInfo

PORT = '/dev/ttyAMA0'
c4004 = DFRobot_C4004(PORT, 115200)


def install_mode_to_string(mode):
  if mode == c4004.INSTALL_MODE_SIDE:
    return 'Side'
  if mode == c4004.INSTALL_MODE_TOP:
    return 'Top'
  return 'Unknown'


def print_install_info(title, info):
  print(title)
  print('Mode      : %s' % install_mode_to_string(info.mode))
  print('Height(cm): %d' % info.height_cm)
  print('X angle   : %d' % info.x_angle)
  print('Y angle   : %d' % info.y_angle)
  print('Z angle   : %d' % info.z_angle)
  print('')


def main():
  while not c4004.begin():
    print('DFRobot C4004 begin failed, retrying...')
    time.sleep(1)
  print('DFRobot C4004 begin success.')
  print('Test2: set_install_info / get_install_info')
  print('')

  write_info = InstallInfo(
    mode=c4004.INSTALL_MODE_TOP,
    height_cm=220,
    x_angle=0,
    y_angle=0,
    z_angle=15,
  )

  print_install_info('----- Write install info -----', write_info)

  if c4004.set_install_info(write_info):
    print('set_install_info(): SUCCESS')
  else:
    print('set_install_info(): FAILED')
  time.sleep(0.1)

  read_info = InstallInfo()
  if c4004.get_install_info(read_info):
    print('get_install_info(): SUCCESS')
    print_install_info('----- Read install info -----', read_info)

    if (read_info.mode == write_info.mode and
        read_info.height_cm == write_info.height_cm and
        read_info.x_angle == write_info.x_angle and
        read_info.y_angle == write_info.y_angle and
        read_info.z_angle == write_info.z_angle):
      print('Install info verify: PASS')
    else:
      print('Install info verify: FAIL')
  else:
    print('get_install_info(): FAILED')


if __name__ == '__main__':
  main()
