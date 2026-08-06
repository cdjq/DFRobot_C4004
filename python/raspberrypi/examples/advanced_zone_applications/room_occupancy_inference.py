# -*- coding: utf-8 -*
'''!
@file room_occupancy_inference.py
@brief Infer kitchen occupancy from kitchen-door boundary tag events.
@details This example configures living-room, kitchen, and kitchen-door tags.
@n The kitchen-door tag is Boundary relative to the living-room range.
@n Enter-living-room events decrement the kitchen people count, and
@n exit-living-room events increment it.
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

port = '/dev/ttyAMA0'
c4004 = DFRobot_C4004(port, 115200)

tag_living_room = 0
tag_kitchen = 1  # Configured only; kitchen people count uses door enter/exit events.
tag_kitchen_door = 2

check_to_active_frames_cfg = 2
no_person_delay_s = 5
track_exists_time_s = 1

living_room_center_x_cm = 0
living_room_center_y_cm = 200
# Rectangle size_x/size_y map to tag width/height: width along X-axis, height along Y-axis (cm).
living_room_size_x_cm = 400
living_room_size_y_cm = 400

kitchen_center_x_cm = 0
kitchen_center_y_cm = 600
kitchen_size_x_cm = 200
kitchen_size_y_cm = 400

door_center_x_cm = 0
door_center_y_cm = 400
door_size_x_cm = 100
door_size_y_cm = 100

living_motion_count = 0
living_static_count = 0
living_people_count = 0

kitchen_door_enter_count = 0
kitchen_door_exit_count = 0
kitchen_inferred_people = 0
last_door_event = 'None'


def door_event_to_text(enter_exit):
  if enter_exit == c4004.ENTER:
    return 'Enter living room'
  if enter_exit == c4004.EXIT:
    return 'Exit living room'
  return 'None'


def process_living_room_tag(info):
  global living_motion_count
  global living_static_count
  global living_people_count

  living_motion_count = info.motion_num
  living_static_count = info.static_num
  living_people_count = living_motion_count + living_static_count


def process_kitchen_door_tag(info):
  global kitchen_door_enter_count
  global kitchen_door_exit_count
  global kitchen_inferred_people
  global last_door_event

  if info.enter_exit == c4004.ENTER:
    kitchen_door_enter_count += 1
    if kitchen_inferred_people > 0:
      kitchen_inferred_people -= 1
    last_door_event = 'Enter living room'
  elif info.enter_exit == c4004.EXIT:
    kitchen_door_exit_count += 1
    kitchen_inferred_people += 1
    last_door_event = 'Exit living room'
  else:
    last_door_event = 'None'

  print('------------------------------------------------------------')
  print('Kitchen door event      : %s' % door_event_to_text(info.enter_exit))
  print('Kitchen inferred people : %d' % kitchen_inferred_people)


def process_tag_event():
  info = c4004.get_tag_info()
  if info is None:
    return

  if info.tag_index == tag_living_room and info.tag_type == c4004.TAG_PEOPLE_COUNTING:
    process_living_room_tag(info)
  elif info.tag_index == tag_kitchen_door and info.tag_type == c4004.TAG_BOUNDARY:
    process_kitchen_door_tag(info)


def setup_sensor_and_tags():
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

  if c4004.set_presence_enable(True):
    print('Set presence enable success.')
  else:
    print('Set presence enable failed.')

  if c4004.set_frame_generate_count(check_to_active_frames_cfg):
    print('Set check-to-active frames success.')
  else:
    print('Set check-to-active frames failed.')
  time.sleep(0.05)

  check_to_active_frames = [0]
  if c4004.get_frame_generate_count(check_to_active_frames):
    print('Current check-to-active frames: %d' % check_to_active_frames[0])
  else:
    print('Read current check-to-active frames failed.')

  range_info = FourSidedRange()
  range_info.x_max = 200
  range_info.x_min = -200
  range_info.y_max = 400
  range_info.y_min = 0
  if c4004.set_four_sided_range_mode(range_info):
    print('Set boundary detection range success.')
  else:
    print('Set boundary detection range failed.')

  if c4004.clear_all_tags():
    print('Clear all tags success.')
  else:
    print('Clear all tags failed.')

  tags = []

  living_room = DFRobot_TagConfig()
  living_room.tag_index = tag_living_room
  living_room.tag_type = c4004.TAG_PEOPLE_COUNTING
  living_room.scope_type = c4004.RECTANGLE
  living_room.io_index = 0
  living_room.center_x = living_room_center_x_cm
  living_room.center_y = living_room_center_y_cm
  living_room.width = living_room_size_x_cm
  living_room.height = living_room_size_y_cm
  tags.append(living_room)

  kitchen = DFRobot_TagConfig()
  kitchen.tag_index = tag_kitchen
  kitchen.tag_type = c4004.TAG_PEOPLE_COUNTING
  kitchen.scope_type = c4004.RECTANGLE
  kitchen.io_index = 0
  kitchen.center_x = kitchen_center_x_cm
  kitchen.center_y = kitchen_center_y_cm
  kitchen.width = kitchen_size_x_cm
  kitchen.height = kitchen_size_y_cm
  tags.append(kitchen)

  kitchen_door = DFRobot_TagConfig()
  kitchen_door.tag_index = tag_kitchen_door
  kitchen_door.tag_type = c4004.TAG_BOUNDARY
  kitchen_door.scope_type = c4004.RECTANGLE
  kitchen_door.io_index = 0
  kitchen_door.center_x = door_center_x_cm
  kitchen_door.center_y = door_center_y_cm
  kitchen_door.width = door_size_x_cm
  kitchen_door.height = door_size_y_cm
  tags.append(kitchen_door)

  if c4004.set_tags_from_config(tags):
    print('Set living/kitchen/door tags success.')
  else:
    print('Set living/kitchen/door tags failed.')

  if c4004.set_trajectory_track_enable(True):
    print('Set trajectory track enable success.')
  else:
    print('Set trajectory track enable failed.')

  if c4004.set_trajectory_lifetime(track_exists_time_s):
    print('Set TrajectoryLifetime success.')
  else:
    print('Set TrajectoryLifetime failed.')

  if c4004.set_unoccupied_time(no_person_delay_s):
    print('Set UnoccupiedTime success.')
  else:
    print('Set UnoccupiedTime failed.')

  print('============================================================')
  print('Kitchen occupancy inference started.')
  print('Direction: kitchen-door Boundary tag event relative to living room.')
  print('Kitchen people count decrements on Enter living room and increments on Exit living room.')
  print('Living-room count is printed only and does not affect kitchen state.')
  print('Kitchen tag is configured for range/tag testing only.')
  print('============================================================')


def print_status():
  print('============================================================')
  print('Living motion/static    : %d/%d' % (living_motion_count, living_static_count))
  print('Living people           : %d' % living_people_count)
  print('Last kitchen door event : %s' % last_door_event)
  print('Door enter/exit count   : %d/%d' % (kitchen_door_enter_count, kitchen_door_exit_count))
  print('Kitchen inferred people : %d' % kitchen_inferred_people)


def main():
  setup_sensor_and_tags()

  last_print_s = 0.0
  while True:
    now_s = time.time()
    event = c4004.get_reported_event(0.05)

    if event == c4004.EVENT_TAG:
      process_tag_event()

    if now_s - last_print_s >= 1.0:
      last_print_s = now_s
      print_status()


if __name__ == '__main__':
  main()
