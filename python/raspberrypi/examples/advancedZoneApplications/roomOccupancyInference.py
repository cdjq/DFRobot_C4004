# -*- coding: utf-8 -*
'''!
  @file roomOccupancyInference.py
  @brief Infer kitchen occupancy from kitchen-door boundary tag events.
  @details This example configures living-room, kitchen, and kitchen-door tags.
  @n The kitchen-door tag is Boundary relative to the living-room range.
  @n Enter-living-room events decrement the kitchen people count, and
  @n exit-living-room events increment it.
  @copyright Copyright (c) 2026 DFRobot Co.Ltd (http://www.dfrobot.com)
  @license The MIT License (MIT)
  @author JiaLi(zhixin.liu@dfrobot.com)
  @version V1.0.0
  @date 2026-05-25
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

from DFRobot_C4004 import DFRobot_C4004, TagConfig, FourSidedRange

PORT = '/dev/ttyAMA0'
c4004 = DFRobot_C4004(PORT, 115200)

TAG_LIVING_ROOM = 0
TAG_KITCHEN = 1       # Configured only; kitchen people count uses door enter/exit events.
TAG_KITCHEN_DOOR = 2

CHECK_TO_ACTIVE_FRAMES = 2
NO_PERSON_DELAY_S = 5
TRACK_EXISTS_TIME_S = 1

LIVING_ROOM_CENTER_X_CM = 0
LIVING_ROOM_CENTER_Y_CM = 200
LIVING_ROOM_SIZE_X_CM = 400
LIVING_ROOM_SIZE_Y_CM = 400

KITCHEN_CENTER_X_CM = 0
KITCHEN_CENTER_Y_CM = 600
KITCHEN_SIZE_X_CM = 200
KITCHEN_SIZE_Y_CM = 400

DOOR_CENTER_X_CM = 0
DOOR_CENTER_Y_CM = 400
DOOR_SIZE_X_CM = 100
DOOR_SIZE_Y_CM = 100

living_motion_count = 0
living_static_count = 0
living_people_count = 0

kitchen_door_enter_count = 0
kitchen_door_exit_count = 0
kitchen_inferred_people = 0
kitchen_occupied = False
last_door_event = 'None'


def door_event_to_text(enter_exit):
  if enter_exit == 0:
    return 'Enter living room'
  if enter_exit == 1:
    return 'Exit living room'
  return 'Unknown'


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
  global kitchen_occupied
  global last_door_event

  if info.enter_exit == 0:
    kitchen_door_enter_count += 1
    if kitchen_inferred_people > 0:
      kitchen_inferred_people -= 1
    last_door_event = 'Enter living room'
  elif info.enter_exit == 1:
    kitchen_door_exit_count += 1
    kitchen_inferred_people += 1
    last_door_event = 'Exit living room'
  else:
    last_door_event = 'Unknown'

  kitchen_occupied = kitchen_inferred_people > 0

  print('------------------------------------------------------------')
  print('Kitchen door event      : %s' % door_event_to_text(info.enter_exit))
  print('Kitchen inferred people : %d' % kitchen_inferred_people)
  print('Kitchen occupied        : %s' % ('YES' if kitchen_occupied else 'NO'))


def process_tag_event():
  info = c4004.get_tag_info()
  if info is None:
    return

  if info.tag_index == TAG_LIVING_ROOM and info.tag_type == c4004.TAG_PEOPLE_COUNTING:
    process_living_room_tag(info)
  elif info.tag_index == TAG_KITCHEN_DOOR and info.tag_type == c4004.TAG_BOUNDARY:
    process_kitchen_door_tag(info)


def setup_sensor_and_tags():
  while not c4004.begin():
    print('DFRobot C4004 begin failed, retrying...')
    time.sleep(1)
  print('DFRobot C4004 begin success.')

  if c4004.set_presence_enable(True):
    print('Set presence enable success.')
  else:
    print('Set presence enable failed.')

  if c4004.set_check_to_active_frames(CHECK_TO_ACTIVE_FRAMES):
    print('Set check-to-active frames success.')
  else:
    print('Set check-to-active frames failed.')
  time.sleep(0.05)

  check_to_active_frames = [0]
  if c4004.get_check_to_active_frames(check_to_active_frames):
    print('Current check-to-active frames: %d' % check_to_active_frames[0])
  else:
    print('Read current check-to-active frames failed.')

  range_info = FourSidedRange()
  range_info.mode = c4004.RANGE_FOUR_SIDE
  range_info.x_positive_cm = 200
  range_info.x_negative_cm = -200
  range_info.y_positive_cm = 400
  range_info.y_negative_cm = 0
  if c4004.set_four_sided_range_mode(range_info):
    print('Set boundary detection range success.')
  else:
    print('Set boundary detection range failed.')

  if c4004.clear_all_tags():
    print('Clear all tags success.')
  else:
    print('Clear all tags failed.')

  tags = []

  living_room = TagConfig()
  living_room.tag_index = TAG_LIVING_ROOM
  living_room.tag_type = c4004.TAG_PEOPLE_COUNTING
  living_room.scope_type = c4004.TAG_RANGE_RECTANGLE
  living_room.io_index = 0
  living_room.center_x = LIVING_ROOM_CENTER_X_CM
  living_room.center_y = LIVING_ROOM_CENTER_Y_CM
  living_room.width = LIVING_ROOM_SIZE_X_CM
  living_room.height = LIVING_ROOM_SIZE_Y_CM
  tags.append(living_room)

  kitchen = TagConfig()
  kitchen.tag_index = TAG_KITCHEN
  kitchen.tag_type = c4004.TAG_PEOPLE_COUNTING
  kitchen.scope_type = c4004.TAG_RANGE_RECTANGLE
  kitchen.io_index = 0
  kitchen.center_x = KITCHEN_CENTER_X_CM
  kitchen.center_y = KITCHEN_CENTER_Y_CM
  kitchen.width = KITCHEN_SIZE_X_CM
  kitchen.height = KITCHEN_SIZE_Y_CM
  tags.append(kitchen)

  kitchen_door = TagConfig()
  kitchen_door.tag_index = TAG_KITCHEN_DOOR
  kitchen_door.tag_type = c4004.TAG_BOUNDARY
  kitchen_door.scope_type = c4004.TAG_RANGE_RECTANGLE
  kitchen_door.io_index = 0
  kitchen_door.center_x = DOOR_CENTER_X_CM
  kitchen_door.center_y = DOOR_CENTER_Y_CM
  kitchen_door.width = DOOR_SIZE_X_CM
  kitchen_door.height = DOOR_SIZE_Y_CM
  tags.append(kitchen_door)

  if c4004.set_tags_from_config(tags):
    print('Set living/kitchen/door tags success.')
  else:
    print('Set living/kitchen/door tags failed.')

  if c4004.set_trajectory_track_enable(True):
    print('Set trajectory track enable success.')
  else:
    print('Set trajectory track enable failed.')

  if c4004.set_track_exists_time(TRACK_EXISTS_TIME_S):
    print('Set TrackExistsTime success.')
  else:
    print('Set TrackExistsTime failed.')

  if c4004.set_unmanned_time(NO_PERSON_DELAY_S):
    print('Set UnmannedTime success.')
  else:
    print('Set UnmannedTime failed.')

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
  print('Kitchen occupied        : %s' % ('YES' if kitchen_occupied else 'NO'))


def main():
  setup_sensor_and_tags()

  last_print_s = 0.0
  while True:
    now_s = time.time()
    event = c4004.get_reported_info(0.05)

    if event == c4004.EVENT_TAG:
      process_tag_event()

    if now_s - last_print_s >= 1.0:
      last_print_s = now_s
      print_status()


if __name__ == '__main__':
  main()
