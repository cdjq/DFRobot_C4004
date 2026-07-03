# -*- coding: utf-8 -*
'''!
  @file test3.py
  @brief Test set_tag() after single-target confirmation.
'''

import os
import sys
import time

cur_path = os.path.dirname(os.path.abspath(__file__))
lib_root = os.path.dirname(os.path.dirname(cur_path))
sys.path.insert(0, os.path.join(lib_root, 'python', 'raspberrypi'))
from DFRobot_C4004 import DFRobot_C4004, FourSidedRange_t, TagConfig

PORT = '/dev/ttyAMA0'
c4004 = DFRobot_C4004(PORT, 115200)

TEST_TAG_INDEX = 1
SINGLE_PERSON_CONFIRM_TIMES = 5
PEOPLE_QUERY_INTERVAL = 1.0


def tag_set_status_to_string(status):
  if status == c4004.TAG_SET_SUCCESS:
    return 'Success'
  if status == c4004.TAG_SET_TRACK_COUNT_ERROR:
    return 'TrackCountError'
  if status == c4004.TAG_SET_ALREADY_USED:
    return 'AlreadyUsed'
  if status == c4004.TAG_SET_INDEX_OUT_OF_RANGE:
    return 'IndexOutOfRange'
  return 'CommError'


def print_tag_list(title, tags):
  print(title)
  print('Tag count: %d' % len(tags))
  if not tags:
    print('No tag.')
    print('')
    return

  print('Idx  Type             Range      IO  CenterX  CenterY  Width  Height')
  print('---- ---------------- ---------- --  -------  -------  -----  ------')
  for tag in tags:
    if tag.tag_type == c4004.TAG_NONE:
      type_text = 'None'
    elif tag.tag_type == c4004.TAG_BOUNDARY:
      type_text = 'Boundary'
    elif tag.tag_type == c4004.TAG_APPROACH_AWAY:
      type_text = 'ApproachAway'
    elif tag.tag_type == c4004.TAG_PEOPLE_COUNTING:
      type_text = 'PeopleCounting'
    elif tag.tag_type == c4004.TAG_NOISE:
      type_text = 'Noise'
    else:
      type_text = 'Unknown'

    range_text = 'Circle' if tag.scope_type == c4004.TAG_RANGE_CIRCLE else 'Rectangle'
    print('%3u  %-16s %-10s %2u  %7d  %7d  %5u  %6u' % (
      tag.tag_index, type_text, range_text, tag.io_index,
      tag.center_x, tag.center_y, tag.width, tag.height))
  print('')


def wait_for_single_target():
  confirm_count = 0

  print('Wait until active target count stays at 1 for 5 checks...')

  while confirm_count < SINGLE_PERSON_CONFIRM_TIMES:
    start_time = time.time()
    target_count = len(c4004.get_target_list(c4004.GET_DATA_ACTIVE))

    print('Active target count: %d' % target_count, end='')

    if target_count == 1:
      confirm_count += 1
    else:
      confirm_count = 0

    print('  confirm: %d/%d' % (confirm_count, SINGLE_PERSON_CONFIRM_TIMES))

    while time.time() - start_time < PEOPLE_QUERY_INTERVAL:
      c4004.get_reported_info(0.01)

  return True


def main():
  while not c4004.begin():
    print('DFRobot C4004 begin failed, retrying...')
    time.sleep(1)
  print('DFRobot C4004 begin success.')
  print('Test3: set_tag')
  print('')

  if c4004.set_check_to_active_frames(7):
    print('Set check-to-active frames success.')
  else:
    print('Set check-to-active frames failed.')
  time.sleep(0.05)

  range_info = FourSidedRange_t()
  range_info.mode = c4004.RANGE_FOUR_SIDE
  range_info.x_positive_cm = 500
  range_info.x_negative_cm = -500
  range_info.y_positive_cm = 800
  range_info.y_negative_cm = 0
  if c4004.set_four_sided_range_mode(range_info):
    print('Set four sided range success.')
  else:
    print('Set four sided range failed.')
  time.sleep(0.05)

  if c4004.set_trajectory_track_enable(True):
    print('Set trajectory track enable success.')
  else:
    print('Set trajectory track enable failed.')
  time.sleep(0.05)

  if c4004.clear_all_tags():
    print('clear_all_tags(): SUCCESS')
  else:
    print('clear_all_tags(): FAILED')
  time.sleep(0.1)

  if not wait_for_single_target():
    print('Single target confirmation failed.')
    return

  tag = TagConfig()
  tag.tag_index = TEST_TAG_INDEX
  tag.tag_type = c4004.TAG_PEOPLE_COUNTING
  tag.scope_type = c4004.TAG_RANGE_RECTANGLE
  tag.io_index = 0
  tag.width = 120
  tag.height = 120

  print('set_tag() tagIndex=%d width=%d height=%d' % (
    TEST_TAG_INDEX, tag.width, tag.height))

  status = c4004.set_tag(tag)
  print('set_tag() status: %s' % tag_set_status_to_string(status))

  if status != c4004.TAG_SET_SUCCESS:
    print('set_tag test stopped.')
    return

  time.sleep(0.1)
  tags = c4004.get_tags(c4004.GET_DATA_ACTIVE)
  print_tag_list('----- Tag list after set_tag -----', tags)


if __name__ == '__main__':
  main()
