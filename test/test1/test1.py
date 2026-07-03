# -*- coding: utf-8 -*
'''!
  @file test1.py
  @brief Test is_connected() and get_heartbeat().
'''

import os
import sys
import time

cur_path = os.path.dirname(os.path.abspath(__file__))
lib_root = os.path.dirname(os.path.dirname(cur_path))
sys.path.insert(0, os.path.join(lib_root, 'python', 'raspberrypi'))
from DFRobot_C4004 import DFRobot_C4004

PORT = '/dev/ttyAMA0'
c4004 = DFRobot_C4004(PORT, 115200)


def print_heartbeat_result(mode_text, mode):
  heartbeat = c4004.get_heartbeat(mode)
  print('get_heartbeat(%s): %s' % (mode_text, 'OK' if heartbeat else 'FAIL'))


def run_connection_test():
  connected = c4004.is_connected()

  print('====================ConnectionTest====================')
  print('is_connected(): %s' % ('CONNECTED' if connected else 'NOT CONNECTED'))
  print_heartbeat_result('GET_DATA_ACTIVE', c4004.GET_DATA_ACTIVE)
  print_heartbeat_result('GET_DATA_REPORT', c4004.GET_DATA_REPORT)
  print('')


def main():
  while not c4004.begin():
    print('DFRobot C4004 begin failed, retrying...')
    time.sleep(1)
  print('DFRobot C4004 begin success.')
  print('Test1: is_connected / get_heartbeat')
  print('')

  while True:
    run_connection_test()
    time.sleep(3)


if __name__ == '__main__':
  main()
