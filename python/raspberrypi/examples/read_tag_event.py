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
from DFRobot_C4004 import DFRobot_C4004, TagConfig, BoundaryDetectionRange

c4004 = DFRobot_C4004('/dev/ttyAMA0', 115200)


def tag_type_to_string(tag_type):
  if tag_type == c4004.TAG_TYPE_NONE:
    return 'None'
  if tag_type == c4004.TAG_TYPE_ENTER_EXIT:
    return 'EnterExit'
  if tag_type == c4004.TAG_TYPE_APPROACH_AWAY:
    return 'ApproachAway'
  if tag_type == c4004.TAG_TYPE_PEOPLE_COUNTING:
    return 'PeopleCounting'
  if tag_type == c4004.TAG_TYPE_NOISE:
    return 'Noise'
  return 'Unknown'


def print_tag_event(info):
  if info is None:
    return

  print('======================================================================')
  print('============================TagEventReport============================')
  print('======================================================================')
  print('Tag Index : %d' % info.index)
  print('Center XY : %d / %d' % (info.center_x, info.center_y))

  if info.type == c4004.TAG_TYPE_ENTER_EXIT:
    print('Event     : EnterExit (%s)' % ('Enter' if info.enter_exit == 0 else 'Exit'))
  elif info.type == c4004.TAG_TYPE_APPROACH_AWAY:
    print('Event     : MotionDirection (%s)' % ('Approach' if info.motion_dir == 0 else 'Away'))
  elif info.type == c4004.TAG_TYPE_PEOPLE_COUNTING:
    print('Event     : PeopleCounting (M:%d S:%d)' % (info.motion_num, info.static_num))
  else:
    print('Event     : %s' % tag_type_to_string(info.type))
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

  print('Idx\tType\t\tRange\t\tCenterX\tCenterY\tWidth\tHeight')
  print('----------------------------------------------------------------------')
  for tag in tags:
    type_text = tag_type_to_string(tag.type)
    range_text = 'Circle' if tag.range_type == c4004.TAG_RANGE_CIRCLE else 'Rectangle'

    line = '%d\t%s' % (tag.index, type_text)
    if len(type_text) <= 8:
      line += '\t\t'
    else:
      line += '\t'

    line += range_text
    if len(range_text) <= 8:
      line += '\t\t'
    else:
      line += '\t'

    line += '%d\t%d\t%d\t%d' % (tag.center_x, tag.center_y, tag.x_size, tag.y_size)
    print(line)
  print('')


def main():
  while not c4004.begin():
    print('DFRobot C4004 begin failed, retrying...')
    time.sleep(1)

  range_info = BoundaryDetectionRange()
  range_info.mode = c4004.RANGE_FOUR_SIDE_BOUNDARY
  range_info.x_positive_cm = 400
  range_info.x_negative_cm = -400
  range_info.y_positive_cm = 600
  range_info.y_negative_cm = 0
  if c4004.set_boundary_detection_range(range_info):
    print('Set boundary detection range success.')
  else:
    print('Set boundary detection range failed.')

  if c4004.clear_all_tags():
    print('Clear all tags success.')
  else:
    print('Clear all tags failed.')

  set_tags = []

  tag0 = TagConfig()
  tag0.index = 0
  tag0.type = c4004.TAG_TYPE_NONE
  tag0.range_type = c4004.TAG_RANGE_RECTANGLE
  tag0.center_x = 0
  tag0.center_y = 80
  tag0.x_size = 180
  tag0.y_size = 120
  set_tags.append(tag0)

  tag1 = TagConfig()
  tag1.index = 1
  tag1.type = c4004.TAG_TYPE_ENTER_EXIT
  tag1.range_type = c4004.TAG_RANGE_RECTANGLE
  tag1.center_x = 180
  tag1.center_y = 160
  tag1.x_size = 200
  tag1.y_size = 140
  set_tags.append(tag1)

  tag2 = TagConfig()
  tag2.index = 2
  tag2.type = c4004.TAG_TYPE_APPROACH_AWAY
  tag2.range_type = c4004.TAG_RANGE_CIRCLE
  tag2.center_x = -180
  tag2.center_y = 240
  tag2.x_size = 160
  tag2.y_size = 160
  set_tags.append(tag2)

  tag3 = TagConfig()
  tag3.index = 3
  tag3.type = c4004.TAG_TYPE_PEOPLE_COUNTING
  tag3.range_type = c4004.TAG_RANGE_RECTANGLE
  tag3.center_x = 80
  tag3.center_y = 260
  tag3.x_size = 220
  tag3.y_size = 150
  set_tags.append(tag3)

  tag4 = TagConfig()
  tag4.index = 4
  tag4.type = c4004.TAG_TYPE_NOISE
  tag4.range_type = c4004.TAG_RANGE_RECTANGLE
  tag4.center_x = -220
  tag4.center_y = 360
  tag4.x_size = 260
  tag4.y_size = 180
  set_tags.append(tag4)

  if c4004.set_tags_from_config(set_tags):
    print('Set 5 tags from config success.')
  else:
    print('Set 5 tags from config failed.')

  tags = c4004.get_tags(c4004.GET_DATA_ACTIVE)
  print_tag_list('Active tag list after setup:', tags)

  while True:
    event = c4004.get_reported_info(0.1)
    if event == c4004.EVENT_TAG:
      print('Tag event report received.')
      print_tag_event(c4004.get_tag_info())


if __name__ == '__main__':
  main()

