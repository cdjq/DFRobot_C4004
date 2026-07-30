# -*- coding: utf-8 -*
'''!
@file factory_reset_and_reboot.py
@brief Factory reset and reboot demo.
@details Use this example when you need to restore the module to factory settings and reboot it
@n (for example after a wrong configuration, or before starting a clean setup).
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
from DFRobot_C4004 import DFRobot_C4004

c4004 = DFRobot_C4004('/dev/ttyAMA0', 115200)


def main():
  while not c4004.begin():
    print('DFRobot C4004 begin failed, retrying...')
    time.sleep(1)

  print('Hardware version:', c4004.get_hardware_version())
  print('Firmware version:', c4004.get_firmware_version())

  print('Module factory resetting...')
  if c4004.factory_reset():
    print('Factory reset success.')
  else:
    print('Factory reset failed.')

  print('Module rebooting ...')
  if c4004.reset():
    print('Reboot success.')
  else:
    print('Reboot failed.')


if __name__ == '__main__':
  main()
