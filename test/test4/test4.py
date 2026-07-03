# -*- coding: utf-8 -*
'''!
  @file test4.py
  @brief Test clear_tag() for the tag index written by test3.py.
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

TEST_TAG_INDEX = 1


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


def has_tag_index(tags, tag_index):
  for tag in tags:
    if tag.tag_index == tag_index:
      return True
  return False


def main():
  while not c4004.begin():
    print('DFRobot C4004 begin failed, retrying...')
    time.sleep(1)
  print('DFRobot C4004 begin success.')
  print('Test4: clear_tag')
  print('Target tag index: %d' % TEST_TAG_INDEX)
  print('')

  tags = c4004.get_tags(c4004.GET_DATA_ACTIVE)
  print_tag_list('----- Tag list before clear_tag -----', tags)

  if not has_tag_index(tags, TEST_TAG_INDEX):
    print('Warning: target tag not found. Run test3.py first.')

  print('clear_tag(%d): %s' % (
    TEST_TAG_INDEX,
    'SUCCESS' if c4004.clear_tag(TEST_TAG_INDEX) else 'FAILED'))
  time.sleep(0.1)

  tags = c4004.get_tags(c4004.GET_DATA_ACTIVE)
  print_tag_list('----- Tag list after clear_tag -----', tags)

  if not has_tag_index(tags, TEST_TAG_INDEX):
    print('clear_tag verify: PASS')
  else:
    print('clear_tag verify: FAIL')


if __name__ == '__main__':
  main()
