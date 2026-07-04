# -*- coding: utf-8 -*
'''!
@file learning_trajectory_range.py
@brief Example for learning trajectory range.
@copyright Copyright (c) 2026 DFRobot Co.Ltd (http://www.dfrobot.com)
@license The MIT License (MIT)
@author JiaLi(zhixin.liu@dfrobot.com)
@version V1.0.0
@date 2026-05-22
@url https://github.com/DFRobot/DFRobot_C4004
'''

import os
import select
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

single_person_confirm_times = 5
people_query_interval = 1.0


def print_menu():
  print('')
  print(' ================Trajectory Range Menu=============')
  print('|1: Learn trajectory range                         |')
  print('|2: Use trajectory range mode                      |')
  print('|3: Query trajectory range points                  |')
  print('|4: Print this menu                                |')
  print('|During learning: send e to stop, q to cancel      |')
  print(' ==================================================')


def read_command(timeout=0):
  readable, _, _ = select.select([sys.stdin], [], [], timeout)
  if not readable:
    return ''

  line = sys.stdin.readline()
  if not line:
    return ''

  line = line.strip()
  if not line:
    return ''
  return line[0]


def wait_command():
  while True:
    cmd = read_command(0.02)
    if cmd:
      return cmd
    c4004.get_reported_info(0.02)


def setup_params():
  print(' ====================Init Params===================')

  range_info = FourSidedRange()
  range_info.mode = c4004.RANGE_FOUR_SIDE
  range_info.x_positive_cm = 500
  range_info.x_negative_cm = -500
  range_info.y_positive_cm = 800
  range_info.y_negative_cm = 0

  if c4004.set_four_sided_range_mode(range_info):
    print('Set four sided range success!')
  else:
    print('Set four sided range failed!')
  time.sleep(0.05)

  if c4004.set_presence_enable(False):
    print('Disabled presence report.')
  else:
    print('Failed to disable presence report.')
  time.sleep(0.05)

  if c4004.set_real_time_people_time(0):
    print('Disabled people count report.')
  else:
    print('Failed to disable people count report.')
  time.sleep(0.05)

  if c4004.set_trajectory_track_enable(True):
    print('Enabled trajectory tracking.')
  else:
    print('Failed to enable trajectory tracking.')
  time.sleep(0.05)

  if c4004.set_motion_led(True):
    print('Turned on motion LED.')
  else:
    print('Failed to turn on motion LED.')
  time.sleep(0.05)

  if c4004.set_trajectory_led(True):
    print('Turned on trajectory LED.')
  else:
    print('Failed to turn on trajectory LED.')
  time.sleep(0.05)


def print_trajectory_points(points):
  points_per_line = 4

  if not points:
    print('(no points)')
    return

  print('Point coordinates (x, y):')
  for start in range(0, len(points), points_per_line):
    chunk = points[start : start + points_per_line]
    print('  '.join('(%5d,%5d)' % (point.pos_x, point.pos_y) for point in chunk))


def wait_for_single_person():
  confirm_count = 0

  while confirm_count < single_person_confirm_times:
    start_time = time.time()
    target_count = len(c4004.get_target_list(c4004.GET_DATA_ACTIVE))

    if target_count == 1:
      confirm_count += 1
    else:
      confirm_count = 0

    print('Active target count: %d  confirm: %d/%d' % (target_count, confirm_count, single_person_confirm_times))

    while time.time() - start_time < people_query_interval:
      cmd = read_command(0.01)
      if cmd in ('q', 'Q'):
        return False
      c4004.get_reported_info(0.01)

  return True


def learn_trajectory_range():
  print('')
  print(' =================Learn Trajectory=================')
  print('Waiting until active people count is 1 for 5 times.')
  print('Send q to cancel.')

  if not wait_for_single_person():
    print('Learning canceled before start.')
    print_menu()
    return

  print('Single-person condition confirmed.')
  print('Send s to start trajectory learning, or q to cancel.')

  while True:
    cmd = wait_command()
    if cmd in ('q', 'Q'):
      print('Learning canceled before start.')
      print_menu()
      return
    if cmd in ('s', 'S'):
      break
    print('Send s to start, or q to cancel.')

  print('Start trajectory learning...')
  c4004.set_trajectory_range_mode(True)
  print('OK')

  print('Learning is running. Walk the required boundary path.')
  print('Send e to stop learning.')

  while True:
    cmd = read_command(0.05)
    if cmd in ('e', 'E', 'q', 'Q'):
      break
    c4004.get_reported_info(0.05)

  print('Set trajectory range mode (learning off).')
  c4004.set_trajectory_range_mode(False)

  time.sleep(0.2)
  query_trajectory_range()
  print_menu()


def set_use_trajectory_range_mode():
  print('')
  print(' ===============Use Trajectory Range===============')
  print('Set trajectory range mode (learning off).')
  c4004.set_trajectory_range_mode(False)


def query_trajectory_range():
  points = []
  point_count = [0]

  print('')
  print(' ==============Query Trajectory Range==============')
  if not c4004.get_trajectory_range_mode(points, point_count):
    print('Query trajectory range failed.')
    return

  print('Trajectory point count:', point_count[0])
  print_trajectory_points(points)


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

  setup_params()
  print_menu()

  while True:
    # When state or data changes and the corresponding report function is enabled,
    # the module pushes the update immediately as an event via get_reported_info().
    # Use the matching getter with GET_DATA_REPORT to read the cached value
    # updated by that report, without issuing an extra UART query.
    cmd = read_command(0.01)

    if not cmd:
      c4004.get_reported_info(0.01)
      continue

    if cmd == '1':
      learn_trajectory_range()
    elif cmd == '2':
      set_use_trajectory_range_mode()
      print_menu()
    elif cmd == '3':
      query_trajectory_range()
      print_menu()
    elif cmd == '4':
      print_menu()
    else:
      print('Unknown command.')
      print_menu()


if __name__ == '__main__':
  main()
