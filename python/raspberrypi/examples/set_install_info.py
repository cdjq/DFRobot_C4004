# -*- coding: utf-8 -*
'''!
  @file set_install_info.py
  @brief Set and read DFRobot C4004 installation mode, height and angle.
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
from DFRobot_C4004 import DFRobot_C4004, InstallInfo

c4004 = DFRobot_C4004('/dev/ttyAMA0', 115200)


def main():
  while not c4004.begin():
    print('DFRobot C4004 begin failed, retrying...')
    time.sleep(1)

  set_inst_info = InstallInfo(
    mode=c4004.INSTALL_MODE_SIDE,
    height_cm=220,
    x_angle=0,
    y_angle=0,
    z_angle=30)  # Only Z-axis angle is valid；Adjustable range: 0-90 degrees.
  print('==============Set install info:==============')
  print('Setup Mode:', 'Top' if set_inst_info.mode == c4004.INSTALL_MODE_TOP else 'Side')
  print('Height(cm):', set_inst_info.height_cm)
  print('Angle z (deg):', set_inst_info.z_angle)

  if c4004.set_install_info(set_inst_info):
    print('Set install info success.')
  else:
    print('Set install info failed.')
  print('==============Get install info:==============')

  time.sleep(1)

  curset_inst_info = InstallInfo()
  if c4004.get_install_info(curset_inst_info):
    print('Setup Mode:', 'Top' if curset_inst_info.mode == c4004.INSTALL_MODE_TOP else 'Side')
    print('Height(cm):', curset_inst_info.height_cm)
    print('Angle z (deg):', curset_inst_info.z_angle)
  else:
    print('Read install info failed.')
  print('=============================================')


if __name__ == '__main__':
  main()
