# -*- coding: utf-8 -*
'''!
  @file read_tag_event.py
  @brief Configure tags and print tag region event reports.
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
from DFRobot_C4004 import DFRobot_C4004, TagConfig, FourSidedRange_t

c4004 = DFRobot_C4004('/dev/ttyAMA0', 115200)


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
    print('Event     : Boundary (%s)' % ('Enter' if info.enter_exit == 0 else 'Exit'))
  elif info.tag_type == c4004.TAG_APPROACH_AWAY:
    print('Event     : MotionDirection (%s)' % ('Approach' if info.motion_dir == 0 else 'Away'))
  elif info.tag_type == c4004.TAG_PEOPLE_COUNTING:
    print('Event     : PeopleCounting (M:%d S:%d)' % (info.motion_num, info.static_num))
  else:
    print('Event     : %s' % tag_type_to_string(info.tag_type))
  print('')


def print_tag_list(title, tags):
  print('======================================================================')
  print(title)
  print('----------------------------------------------------------------------')
  print('Tag count:', len(tags))
  if len(tags) == 0:
    print('No tag.')
    print('')
    return

  print('Idx\tType\t\tRange\t\tIO\tCenterX\tCenterY\tWidth\tHeight')
  print('----------------------------------------------------------------------')
  for tag in tags:
    type_text = tag_type_to_string(tag.tag_type)
    range_text = 'Circle' if tag.scope_type == c4004.TAG_RANGE_CIRCLE else 'Rectangle'

    line = '%d\t%s' % (tag.tag_index, type_text)
    if len(type_text) <= 8:
      line += '\t\t'
    else:
      line += '\t'

    line += range_text
    if len(range_text) <= 8:
      line += '\t\t'
    else:
      line += '\t'

    line += '%d\t%d\t%d\t%d\t%d' % (
      tag.io_index,
      tag.center_x,
      tag.center_y,
      tag.width,
      tag.height)
    print(line)
  print('')


def main():
  while not c4004.begin():
    print('DFRobot C4004 begin failed, retrying...')
    time.sleep(1)

  if c4004.set_check_to_active_frames(7):
    print('Set check-to-active frames success.')
  else:
    print('Set check-to-active frames failed.')
  time.sleep(0.05)

  range_info = FourSidedRange_t()
  range_info.mode = c4004.RANGE_FOUR_SIDE
  range_info.x_positive_cm = 200
  range_info.x_negative_cm = -200
  range_info.y_positive_cm = 700
  range_info.y_negative_cm = 0
  if c4004.set_four_sided_range_mode(range_info):
    print('Set boundary detection range success.')
  else:
    print('Set boundary detection range failed.')

  if c4004.clear_all_tags():
    print('Clear all tags success.')
  else:
    print('Clear all tags failed.')

  set_tags = []

  tag0 = TagConfig()
  tag0.tag_index = 0
  tag0.tag_type = c4004.TAG_NONE
  tag0.scope_type = c4004.TAG_RANGE_RECTANGLE
  tag0.io_index = 0
  tag0.center_x = 0
  tag0.center_y = 100
  tag0.width = 120
  tag0.height = 120
  set_tags.append(tag0)

  tag1 = TagConfig()
  tag1.tag_index = 1
  tag1.tag_type = c4004.TAG_BOUNDARY
  tag1.scope_type = c4004.TAG_RANGE_RECTANGLE
  tag1.io_index = 0
  tag1.center_x = 100
  tag1.center_y = 220
  tag1.width = 120
  tag1.height = 120
  set_tags.append(tag1)

  tag2 = TagConfig()
  tag2.tag_index = 2
  tag2.tag_type = c4004.TAG_APPROACH_AWAY
  tag2.scope_type = c4004.TAG_RANGE_CIRCLE
  tag2.io_index = 0
  tag2.center_x = -80
  tag2.center_y = 350
  tag2.width = 80
  tag2.height = 0
  set_tags.append(tag2)

  tag3 = TagConfig()
  tag3.tag_index = 3
  tag3.tag_type = c4004.TAG_PEOPLE_COUNTING
  tag3.scope_type = c4004.TAG_RANGE_RECTANGLE
  tag3.io_index = 0
  tag3.center_x = 0
  tag3.center_y = 500
  tag3.width = 160
  tag3.height = 160
  set_tags.append(tag3)

  tag4 = TagConfig()
  tag4.tag_index = 4
  tag4.tag_type = c4004.TAG_NOISE
  tag4.scope_type = c4004.TAG_RANGE_RECTANGLE
  tag4.io_index = 0
  tag4.center_x = -100
  tag4.center_y = 620
  tag4.width = 100
  tag4.height = 120
  set_tags.append(tag4)

  if c4004.set_tags_from_config(set_tags):
    print('Set 5 tags from config success.')
  else:
    print('Set 5 tags from config failed.')

  tags = c4004.get_tags()
  print_tag_list('Active tag list after setup:', tags)

  while True:
    event = c4004.get_reported_info(0.1)
    if event == c4004.EVENT_TAG:
      print_tag_event(c4004.get_tag_info())


if __name__ == '__main__':
  main()
