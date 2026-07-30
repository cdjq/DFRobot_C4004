# -*- coding: utf-8 -*
'''!
@file configure_tag.py
@brief Interactively set, clear, and query detection tag zones via a terminal menu.
@details Terminal-menu demo: wait for a single track, then set one tag by size mode;
@n also clear one/all tags, query tag list, and read live tag events.
@n Usage environment:
@n - Please install the sensor at a height of 180 cm for use.
@n - Detection range uses the common four-sided boundary (x +/-200 cm, y 0~700 cm);
@n   it can be modified according to your needs.
@n - Setting a tag requires exactly ONE track in view; stand where the tag center should be.
@n - Run in a terminal; send menu commands (1-6) then Enter, and follow prompts.

@copyright Copyright (c) 2026 DFRobot Co.Ltd (http://www.dfrobot.com)
@license The MIT License (MIT)
@author JiaLi(jia.li@dfrobot.com)
@version V1.0.0
@date 2026-07-30
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
from DFRobot_C4004 import DFRobot_C4004, DFRobot_TagConfig, FourSidedRange

c4004 = DFRobot_C4004('/dev/ttyAMA0', 115200)

single_track_confirm_times = 5
track_query_interval = 1.0
k_max_tags = 32


def print_menu():
  print('')
  print(' ====================Configure Tag Menu===================')
  print('Standard workflow:')
  print('  1) Init done, then this menu.')
  print("  2) Send '1' to set one tag: stand at the tag center, keep ONE track.")
  print('  3) Follow prompts to choose type/shape/index/size/ioIndex.')
  print("  4) Send '4' to query tags; send '5' to read tag events.")
  print("  5) Send '2'/'3' to clear one tag / all tags.")
  print(' ---------------------------------------------------------')
  print('|1: Configure one tag (requires one track)               |')
  print('|2: Clear one tag by index                               |')
  print('|3: Clear all tags                                       |')
  print('|4: Query configured tags                                |')
  print('|5: Read tag events (send q to stop)                     |')
  print('|6: Print this menu                                      |')
  print(' =========================================================')


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
    c4004.get_reported_event(0.02)


def read_line_value(prompt, allow_cancel=True):
  '''!
  @brief Read one line from stdin; return int, or None if canceled/invalid retry loop.
  '''
  while True:
    sys.stdout.write(prompt)
    sys.stdout.flush()
    while True:
      readable, _, _ = select.select([sys.stdin], [], [], 0.02)
      if readable:
        break
      c4004.get_reported_event(0.02)
    line = sys.stdin.readline()
    if not line:
      continue
    line = line.strip()
    if allow_cancel and line.lower() == 'q':
      return None
    if line == '':
      print('Empty input, try again.')
      continue
    if not line.isdigit():
      print('Invalid input, try again.')
      continue
    value = int(line)
    if value < 0 or value > 65535:
      print('Out of range, try again.')
      continue
    return value


def read_choice(prompt, min_choice, max_choice, allow_cancel=True):
  while True:
    sys.stdout.write(prompt)
    sys.stdout.flush()
    cmd = wait_command()
    print(cmd)
    if allow_cancel and cmd.lower() == 'q':
      return None
    if len(cmd) == 1 and min_choice <= cmd <= max_choice:
      return cmd
    print('Invalid choice, try again.')


def tag_type_to_string(tag_type):
  if tag_type == c4004.TAG_NONE:
    return 'None'
  if tag_type == c4004.TAG_BOUNDARY:
    return 'Boundary'
  if tag_type == c4004.TAG_APPROACH_AWAY:
    return 'ApproachAway'
  if tag_type == c4004.TAG_PEOPLE_COUNTING:
    return 'PeopleCounting'
  if tag_type == c4004.TAG_NOISE:
    return 'Noise'
  return 'Unknown'


def tag_set_status_text(status):
  if status == c4004.TAG_SET_SUCCESS:
    return 'Success'
  if status == c4004.TAG_SET_TRACK_COUNT_ERROR:
    return 'TrackCountError (need exactly 1 track)'
  if status == c4004.TAG_SET_ALREADY_USED:
    return 'AlreadyUsed'
  if status == c4004.TAG_SET_INDEX_OUT_OF_RANGE:
    return 'IndexOutOfRange'
  return 'CommError'


def boundary_direction_to_text(value):
  if value == c4004.ENTER:
    return 'Enter'
  if value == c4004.EXIT:
    return 'Exit'
  return 'None'


def approach_away_direction_to_text(value):
  if value == c4004.APPROACH:
    return 'Approach'
  if value == c4004.AWAY:
    return 'Away'
  return 'None'


def print_tag_list(title, tags):
  print('======================================================================')
  print(title)
  print('----------------------------------------------------------------------')
  print('Tag count:', len(tags))
  if len(tags) == 0:
    print('No tag.')
    print('')
    return

  print('Idx  Type             Range      IO  CenterX  CenterY  Width  Height')
  print('---- ---------------- ---------- --  -------  -------  -----  ------')
  for tag in tags:
    type_text = tag_type_to_string(tag.tag_type)
    range_text = 'Circle' if tag.scope_type == c4004.TAG_RANGE_CIRCLE else 'Rectangle'
    print(
      '%-4d %-16s %-10s %-2d  %-7d  %-7d  %-5d  %d'
      % (
        tag.tag_index,
        type_text,
        range_text,
        tag.io_index,
        tag.center_x,
        tag.center_y,
        tag.width,
        tag.height,
      )
    )
  print('')


def print_tag_event(info):
  if info is None:
    return

  print('======================================================================')
  print('============================TagEventReport============================')
  print('======================================================================')
  print('Tag Index : %d' % info.tag_index)
  print('Tag Type  : %s' % tag_type_to_string(info.tag_type))
  print('IO Index  : %d' % info.io_index)
  print('Center XY : %d / %d' % (info.center_x, info.center_y))

  if info.tag_type == c4004.TAG_BOUNDARY:
    print('Event     : Boundary (%s)' % boundary_direction_to_text(info.enter_exit))
  elif info.tag_type == c4004.TAG_APPROACH_AWAY:
    print('Event     : MotionDirection (%s)' % approach_away_direction_to_text(info.motion_dir))
  elif info.tag_type == c4004.TAG_PEOPLE_COUNTING:
    print('Event     : PeopleCounting (M:%d S:%d)' % (info.motion_num, info.static_num))
  else:
    print('Event     : %s' % tag_type_to_string(info.tag_type))
  print('')


def wait_for_single_track():
  confirm_count = 0
  while confirm_count < single_track_confirm_times:
    start_time = time.time()
    targets = c4004.get_target_list(c4004.GET_DATA_ACTIVE)
    target_count = len(targets)
    sys.stdout.write('Active track count: %d' % target_count)
    if target_count == 1:
      confirm_count += 1
    else:
      confirm_count = 0
    print('  confirm: %d/%d' % (confirm_count, single_track_confirm_times))

    while (time.time() - start_time) < track_query_interval:
      cmd = read_command(0.01)
      if cmd.lower() == 'q':
        return False
      c4004.get_reported_event(0.01)
  return True


def handle_set_tag():
  tag = DFRobot_TagConfig()

  print('')
  print(' ========================setTag===========================')
  print('Stand where the tag center should be.')
  print('Keep exactly ONE track in view.')
  print('Note: set_tag uses size mode; center_x/center_y are taken from the current track.')
  print('Send q to cancel while waiting.')

  if not wait_for_single_track():
    print('setTag canceled before start.')
    print_menu()
    return

  print('Single-track condition confirmed.')
  print('Please stay still at the current position until setTag finishes.')

  value = read_line_value('Enter tagIndex (0-254), then Enter: ')
  if value is None:
    print('setTag canceled.')
    print_menu()
    return
  tag.tag_index = value & 0xFF

  print('Select tagType:')
  print('  1: Boundary')
  print('  2: ApproachAway')
  print('  3: PeopleCounting')
  print('  4: Noise')
  choice = read_choice('Enter 1-4 (or q to cancel): ', '1', '4')
  if choice is None:
    print('setTag canceled.')
    print_menu()
    return
  if choice == '1':
    tag.tag_type = c4004.TAG_BOUNDARY
  elif choice == '2':
    tag.tag_type = c4004.TAG_APPROACH_AWAY
  elif choice == '3':
    tag.tag_type = c4004.TAG_PEOPLE_COUNTING
  else:
    tag.tag_type = c4004.TAG_NOISE

  print('Select scopeType:')
  print('  1: Rectangle (width=X size cm, height=Y size cm)')
  print('  2: Circle (enter radius cm)')
  choice = read_choice('Enter 1-2 (or q to cancel): ', '1', '2')
  if choice is None:
    print('setTag canceled.')
    print_menu()
    return

  if choice == '1':
    tag.scope_type = c4004.TAG_RANGE_RECTANGLE
    value = read_line_value('Enter width (cm), then Enter: ')
    if value is None:
      print('setTag canceled.')
      print_menu()
      return
    tag.width = value
    value = read_line_value('Enter height (cm), then Enter: ')
    if value is None:
      print('setTag canceled.')
      print_menu()
      return
    tag.height = value
  else:
    tag.scope_type = c4004.TAG_RANGE_CIRCLE
    value = read_line_value('Enter radius (cm), then Enter: ')
    if value is None:
      print('setTag canceled.')
      print_menu()
      return
    tag.width = value
    tag.height = 0

  print('Select ioIndex: 0=unused, 2-6=IO2-IO6 (IO1 is reserved).')
  value = read_line_value('Enter ioIndex (0 or 2-6), then Enter: ')
  if value is None:
    print('setTag canceled.')
    print_menu()
    return
  if not (value == 0 or (2 <= value <= 6)):
    print('Invalid ioIndex. setTag canceled.')
    print_menu()
    return
  tag.io_index = value

  print('Calling set_tag()...')
  status = c4004.set_tag(tag)
  print('setTag status:', tag_set_status_text(status))

  if status == c4004.TAG_SET_SUCCESS:
    handle_get_tags()
  else:
    print_menu()


def handle_clear_tag():
  print('')
  print(' =======================clearTag==========================')
  tag_index = read_line_value('Enter tagIndex to clear (0-254), then Enter (q cancel): ')
  if tag_index is None:
    print('clearTag canceled.')
    print_menu()
    return

  if c4004.clear_tag(tag_index):
    print('clearTag success.')
  else:
    print('clearTag failed.')
  print_menu()


def handle_clear_all_tags():
  print('')
  print(' =====================clearAllTags========================')
  if c4004.clear_all_tags():
    print('clearAllTags success.')
  else:
    print('clearAllTags failed.')
  print_menu()


def handle_get_tags():
  tags = c4004.get_tags(c4004.GET_DATA_ACTIVE, k_max_tags)
  print_tag_list('Configured tag list:', tags)
  print_menu()


def handle_read_tag_events():
  print('')
  print(' =====================Read Tag Events======================')
  print('Reading tag events. Send q to stop and return to menu.')

  while True:
    cmd = read_command(0.01)
    if cmd.lower() == 'q':
      print('Stop reading tag events.')
      print_menu()
      return

    event = c4004.get_reported_event(0.05)
    if event == c4004.EVENT_TAG:
      tag_info = c4004.get_tag_info()
      if tag_info is not None:
        print_tag_event(tag_info)


def main():
  while not c4004.begin():
    print('DFRobot C4004 begin failed, retrying...')
    time.sleep(1)
  print('DFRobot C4004 begin success.')

  # Side mount: default 180 cm, recommended 180+/-20 cm. Top mount: recommended 220-280 cm.
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
    print('Set boundary detection range success (about 4 m x 7 m).')
  else:
    print('Set boundary detection range failed.')
  time.sleep(0.05)

  if c4004.set_trajectory_track_enable(True):
    print('Enabled trajectory tracking.')
  else:
    print('Failed to enable trajectory tracking.')
  time.sleep(0.05)

  if c4004.set_occ_led(True):
    print('Enabled occupancy LED.')
  else:
    print('Failed to enable occupancy LED.')
  time.sleep(0.05)

  print_menu()

  while True:
    cmd = read_command(0.02)
    if not cmd:
      c4004.get_reported_event(0.02)
      continue

    if cmd == '1':
      handle_set_tag()
    elif cmd == '2':
      handle_clear_tag()
    elif cmd == '3':
      handle_clear_all_tags()
    elif cmd == '4':
      handle_get_tags()
    elif cmd == '5':
      handle_read_tag_events()
    elif cmd == '6':
      print_menu()
    else:
      print('Unknown command.')
      print_menu()


if __name__ == '__main__':
  main()
