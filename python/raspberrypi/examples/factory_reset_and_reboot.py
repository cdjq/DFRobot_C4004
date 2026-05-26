# -*- coding: utf-8 -*
'''!
  @file factory_reset_and_reboot.py
  @brief Restore factory settings and reboot the module.
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
from DFRobot_C4004 import DFRobot_C4004

c4004 = DFRobot_C4004('/dev/ttyAMA0', 115200)


def main():
  while not c4004.begin():
    print('DFRobot C4004 begin failed, retrying...')
    time.sleep(1)
  print('DFRobot C4004 begin success.')

  print('Product model:', c4004.get_product_model())
  print('Product ID:', hex(c4004.get_product_id()))
  print('Hardware version:', c4004.get_hardware_version())
  print('Firmware version:', c4004.get_firmware_version())

  print('Factory reset...')
  if c4004.factory_reset():
    print('Factory reset success.')
  else:
    print('Factory reset failed.')

  print('Reboot module...')
  if c4004.reset():
    print('Reboot command sent.')
  else:
    print('Reboot command failed.')


if __name__ == '__main__':
  main()
