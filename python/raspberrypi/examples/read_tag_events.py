# -*- coding: utf-8 -*
'''!
@file read_tag_events.py
@brief Configure tag zones and print live tag event reports.
@details Use this example to configure tag zones and watch live tag events in the terminal,
@n such as enter/exit, approach/away, people counting and noise-zone reports.
@n Usage environment:
@n - Please install the sensor at a height of 180 cm for use.
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
from DFRobot_C4004 import DFRobot_C4004, DFRobot_TagConfig, FourSidedRange

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
    range_text = 'Circle' if tag.scope_type == c4004.CIRCLE else 'Rectangle'
    print(
      '%3d  %-16s %-10s %2d  %7d  %7d  %5d  %6d'
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


def main():
  while not c4004.begin():
    print('DFRobot C4004 begin failed, retrying...')
    time.sleep(1)
  print('DFRobot C4004 begin success.')

  # Side mount: default 180 cm, recommended 180±20 cm. Top mount: recommended 220-280 cm.
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
    print('Set boundary detection range success.')
  else:
    print('Set boundary detection range failed.')

  if c4004.clear_all_tags():
    print('Clear all tags success.')
  else:
    print('Clear all tags failed.')

  set_tags = []

  # Tag type overview:
  #   TAG_NONE            : Invalid/unused tag type.
  #   TAG_BOUNDARY        : Edge/boundary tag, usually placed at a zone edge;
  #                         reports Enter/Exit when a person passes through.
  #   TAG_APPROACH_AWAY   : Approach/away tag; reports Approach/Away relative to the tag zone.
  #   TAG_PEOPLE_COUNTING : Status/people-counting tag; reports moving and stationary
  #                         people counts inside the tag zone.
  #   TAG_NOISE           : Noise/interference tag; marks the zone as an interference area.
  # Rectangle: width = X-axis size (cm), height = Y-axis size (cm), relative to center_x/center_y.
  # Circle: width = radius (cm), height is ignored.

  # tag0 TAG_NONE: invalid/unused tag, does not generate meaningful tag events.
  tag0 = DFRobot_TagConfig()
  tag0.tag_index = 0
  tag0.tag_type = c4004.TAG_NONE
  tag0.scope_type = c4004.RECTANGLE
  tag0.io_index = 0
  tag0.center_x = 0
  tag0.center_y = 100
  tag0.width = 120    # X-axis size (cm)
  tag0.height = 120   # Y-axis size (cm)
  set_tags.append(tag0)

  # tag1 TAG_BOUNDARY: edge tag; reports Enter/Exit when a person crosses this zone.
  tag1 = DFRobot_TagConfig()
  tag1.tag_index = 1
  tag1.tag_type = c4004.TAG_BOUNDARY
  tag1.scope_type = c4004.RECTANGLE
  tag1.io_index = 0
  tag1.center_x = 100
  tag1.center_y = 220
  tag1.width = 120    # X-axis size (cm)
  tag1.height = 120   # Y-axis size (cm)
  set_tags.append(tag1)

  # tag2 TAG_APPROACH_AWAY: reports Approach/Away relative to this tag zone.
  tag2 = DFRobot_TagConfig()
  tag2.tag_index = 2
  tag2.tag_type = c4004.TAG_APPROACH_AWAY
  tag2.scope_type = c4004.CIRCLE
  tag2.io_index = 0
  tag2.center_x = -80
  tag2.center_y = 350
  tag2.width = 80     # Circle radius (cm)
  tag2.height = 0     # Ignored for circle
  set_tags.append(tag2)

  # tag3 TAG_PEOPLE_COUNTING: counts moving and stationary people inside the tag zone.
  tag3 = DFRobot_TagConfig()
  tag3.tag_index = 3
  tag3.tag_type = c4004.TAG_PEOPLE_COUNTING
  tag3.scope_type = c4004.RECTANGLE
  tag3.io_index = 0
  tag3.center_x = 0
  tag3.center_y = 500
  tag3.width = 160    # X-axis size (cm)
  tag3.height = 160   # Y-axis size (cm)
  set_tags.append(tag3)

  # tag4 TAG_NOISE: marks this zone as an interference/noise area.
  tag4 = DFRobot_TagConfig()
  tag4.tag_index = 4
  tag4.tag_type = c4004.TAG_NOISE
  tag4.scope_type = c4004.RECTANGLE
  tag4.io_index = 0
  tag4.center_x = -100
  tag4.center_y = 620
  tag4.width = 100    # X-axis size (cm)
  tag4.height = 120   # Y-axis size (cm)
  set_tags.append(tag4)

  if c4004.set_tags_from_config(set_tags):
    print('Set 5 tags from config success.')
  else:
    print('Set 5 tags from config failed.')

  tags = c4004.get_tags()
  print_tag_list('Active tag list after setup:', tags)

  print('Setup done. Start detecting tag events...')
  print('Move in/out of configured zones to trigger reports.')
  print('')

  last_status = 0.0
  while True:
    # When state or data changes and the corresponding report function is enabled,
    # the module pushes the update immediately as an event via get_reported_event().
    # Use the matching getter with GET_DATA_REPORT to read the cached value
    # updated by that report, without issuing an extra UART query.
    event = c4004.get_reported_event(0.1)
    if event == c4004.EVENT_TAG:
      tag_info = c4004.get_tag_info()
      if tag_info is not None:
        print_tag_event(tag_info)
    elif time.time() - last_status > 3:
      last_status = time.time()
      print('Detecting... waiting for tag events.')


if __name__ == '__main__':
  main()
