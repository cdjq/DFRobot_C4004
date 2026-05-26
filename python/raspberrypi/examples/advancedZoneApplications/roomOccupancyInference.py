# -*- coding: utf-8 -*
'''!
  @file roomOccupancyInference.py
  @brief Infer kitchen occupancy from doorway crossing coordinates.
  @details Configure living-room, kitchen, and kitchen-door tags.
  @n The kitchen-door tag is ApproachAway. A crossing session starts when a
  @n target approaches the door and ends when a target moves away from the door.
  @n For both events, the closest active target to the door center is used.
  @n Direction is inferred from the start/end coordinate zones only.
  @n This is logical inference only, not direct detection inside the kitchen.
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

from DFRobot_C4004 import DFRobot_C4004, TagConfig, BoundaryDetectionRange

PORT = '/dev/ttyAMA0'
c4004 = DFRobot_C4004(PORT, 115200)

TAG_LIVING_ROOM = 0
TAG_KITCHEN = 1
TAG_KITCHEN_DOOR = 2

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
DOOR_SIZE_Y_CM = 50

LINK_WINDOW_S = 4.0

INFER_NONE = 0
INFER_ENTER_KITCHEN = 1
INFER_EXIT_KITCHEN = 2

ZONE_UNKNOWN = 0
ZONE_LIVING_ROOM = 1
ZONE_KITCHEN = 2

living_motion_count = 0
living_static_count = 0
living_people_count = 0

kitchen_occupied = False
kitchen_inferred_people = 0

door_session_active = False
door_start_zone = ZONE_UNKNOWN
door_start_target_x = 0
door_start_target_y = 0
door_session_start_s = 0.0

last_evidence = 'None'


def infer_dir_to_text(direction):
  if direction == INFER_ENTER_KITCHEN:
    return 'EnterKitchen'
  if direction == INFER_EXIT_KITCHEN:
    return 'ExitKitchen'
  return 'None'


def zone_to_text(zone):
  if zone == ZONE_LIVING_ROOM:
    return 'LivingRoom'
  if zone == ZONE_KITCHEN:
    return 'Kitchen'
  return 'Unknown'


def get_point_zone(x, y):
  living_min_x = LIVING_ROOM_CENTER_X_CM - LIVING_ROOM_SIZE_X_CM // 2
  living_max_x = LIVING_ROOM_CENTER_X_CM + LIVING_ROOM_SIZE_X_CM // 2
  living_min_y = LIVING_ROOM_CENTER_Y_CM - LIVING_ROOM_SIZE_Y_CM // 2
  living_max_y = LIVING_ROOM_CENTER_Y_CM + LIVING_ROOM_SIZE_Y_CM // 2

  kitchen_min_x = KITCHEN_CENTER_X_CM - KITCHEN_SIZE_X_CM // 2
  kitchen_max_x = KITCHEN_CENTER_X_CM + KITCHEN_SIZE_X_CM // 2
  kitchen_min_y = KITCHEN_CENTER_Y_CM - KITCHEN_SIZE_Y_CM // 2
  kitchen_max_y = KITCHEN_CENTER_Y_CM + KITCHEN_SIZE_Y_CM // 2

  in_living_room = (living_min_x <= x <= living_max_x and living_min_y <= y < living_max_y)
  in_kitchen = (kitchen_min_x <= x <= kitchen_max_x and kitchen_min_y < y <= kitchen_max_y)

  if in_living_room and not in_kitchen:
    return ZONE_LIVING_ROOM
  if in_kitchen and not in_living_room:
    return ZONE_KITCHEN
  return ZONE_UNKNOWN


def get_closest_target_to_door():
  targets = c4004.get_target_list(c4004.GET_DATA_ACTIVE)
  if len(targets) == 0:
    return None

  closest_target = None
  closest_dist_sq = None
  for target in targets:
    dx = target.x - DOOR_CENTER_X_CM
    dy = target.y - DOOR_CENTER_Y_CM
    dist_sq = dx * dx + dy * dy
    if closest_dist_sq is None or dist_sq < closest_dist_sq:
      closest_dist_sq = dist_sq
      closest_target = target
  return closest_target


def print_coordinate_evidence(title, start_zone, start_x, start_y, end_zone, end_x, end_y):
  print('------------------------------------------------------------')
  print(title)
  print('Approach coordinate    : (%d, %d), %s' % (start_x, start_y, zone_to_text(start_zone)))
  print('Away coordinate        : (%d, %d), %s' % (end_x, end_y, zone_to_text(end_zone)))


def clear_door_session():
  global door_session_active
  global door_start_zone
  global door_start_target_x
  global door_start_target_y
  global door_session_start_s

  door_session_active = False
  door_start_zone = ZONE_UNKNOWN
  door_start_target_x = 0
  door_start_target_y = 0
  door_session_start_s = 0.0


def confirm_kitchen_event(direction, evidence):
  global kitchen_occupied
  global kitchen_inferred_people
  global last_evidence

  if direction == INFER_NONE:
    return

  if direction == INFER_ENTER_KITCHEN:
    kitchen_inferred_people += 1
  elif kitchen_inferred_people > 0:
    kitchen_inferred_people -= 1

  kitchen_occupied = kitchen_inferred_people > 0
  last_evidence = evidence

  print('------------------------------------------------------------')
  print('Kitchen inference event : %s' % infer_dir_to_text(direction))
  print('Evidence                : %s' % evidence)
  print('Kitchen inferred people : %d' % kitchen_inferred_people)
  print('Kitchen occupied        : %s' % ('YES' if kitchen_occupied else 'NO'))


def start_door_session():
  global door_session_active
  global door_start_zone
  global door_start_target_x
  global door_start_target_y
  global door_session_start_s
  global last_evidence

  target = get_closest_target_to_door()
  if target is None:
    clear_door_session()
    last_evidence = 'approach door: no target'
    print('------------------------------------------------------------')
    print('Door session ignored    : approach door, no active target')
    return

  door_session_active = True
  door_start_zone = get_point_zone(target.x, target.y)
  door_start_target_x = target.x
  door_start_target_y = target.y
  door_session_start_s = time.time()
  last_evidence = 'approach door coordinate recorded'

  print('------------------------------------------------------------')
  print('Door session started    : approach door')
  print('Approach coordinate    : (%d, %d), %s' % (
    door_start_target_x,
    door_start_target_y,
    zone_to_text(door_start_zone)))


def finish_door_session():
  global last_evidence

  if not door_session_active:
    last_evidence = 'leave door without approach'
    print('------------------------------------------------------------')
    print('Door session ignored    : leave door without approach')
    return

  if time.time() - door_session_start_s > LINK_WINDOW_S:
    last_evidence = 'door session timeout'
    print('------------------------------------------------------------')
    print('Door session ignored    : timeout before leave door')
    clear_door_session()
    return

  target = get_closest_target_to_door()
  if target is None:
    last_evidence = 'leave door: no target'
    print('------------------------------------------------------------')
    print('Door session ignored    : leave door, no active target')
    clear_door_session()
    return

  end_zone = get_point_zone(target.x, target.y)
  direction = INFER_NONE
  evidence = 'invalid door coordinate zones'

  if door_start_zone == ZONE_LIVING_ROOM and end_zone == ZONE_KITCHEN:
    direction = INFER_ENTER_KITCHEN
    evidence = 'living room to kitchen crossing'
  elif door_start_zone == ZONE_KITCHEN and end_zone == ZONE_LIVING_ROOM:
    direction = INFER_EXIT_KITCHEN
    evidence = 'kitchen to living room crossing'

  if direction == INFER_NONE:
    title = 'Door crossing ignored  : invalid coordinate zones'
  else:
    title = 'Door crossing confirmed: valid coordinate zones'

  print_coordinate_evidence(
    title,
    door_start_zone,
    door_start_target_x,
    door_start_target_y,
    end_zone,
    target.x,
    target.y)

  if direction == INFER_NONE:
    last_evidence = evidence
  else:
    confirm_kitchen_event(direction, evidence)

  clear_door_session()


def check_door_session_timeout():
  global last_evidence

  if not door_session_active:
    return

  if time.time() - door_session_start_s > LINK_WINDOW_S:
    last_evidence = 'door session timeout'
    print('------------------------------------------------------------')
    print('Door session cleared    : timeout')
    clear_door_session()


def process_living_room_tag(info):
  global living_motion_count
  global living_static_count
  global living_people_count

  living_motion_count = info.motion_num
  living_static_count = info.static_num
  living_people_count = living_motion_count + living_static_count


def process_tag_event():
  info = c4004.get_tag_info()
  if info is None:
    return

  if info.index == TAG_LIVING_ROOM and info.type == c4004.TAG_TYPE_PEOPLE_COUNTING:
    process_living_room_tag(info)
  elif info.index == TAG_KITCHEN_DOOR and info.type == c4004.TAG_TYPE_APPROACH_AWAY:
    if info.motion_dir == 0:
      start_door_session()
    elif info.motion_dir == 1:
      finish_door_session()


def setup_sensor_and_tags():
  while not c4004.begin():
    print('DFRobot C4004 begin failed, retrying...')
    time.sleep(1)
  print('DFRobot C4004 begin success.')

  if c4004.set_presence_enable(True):
    print('Set presence enable success.')
  else:
    print('Set presence enable failed.')

  range_info = BoundaryDetectionRange()
  range_info.mode = c4004.RANGE_FOUR_SIDE_BOUNDARY
  range_info.x_positive_cm = 200
  range_info.x_negative_cm = -200
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

  tags = []

  living_room = TagConfig()
  living_room.index = TAG_LIVING_ROOM
  living_room.type = c4004.TAG_TYPE_PEOPLE_COUNTING
  living_room.range_type = c4004.TAG_RANGE_RECTANGLE
  living_room.center_x = LIVING_ROOM_CENTER_X_CM
  living_room.center_y = LIVING_ROOM_CENTER_Y_CM
  living_room.x_size = LIVING_ROOM_SIZE_X_CM
  living_room.y_size = LIVING_ROOM_SIZE_Y_CM
  tags.append(living_room)

  kitchen = TagConfig()
  kitchen.index = TAG_KITCHEN
  kitchen.type = c4004.TAG_TYPE_PEOPLE_COUNTING
  kitchen.range_type = c4004.TAG_RANGE_RECTANGLE
  kitchen.center_x = KITCHEN_CENTER_X_CM
  kitchen.center_y = KITCHEN_CENTER_Y_CM
  kitchen.x_size = KITCHEN_SIZE_X_CM
  kitchen.y_size = KITCHEN_SIZE_Y_CM
  tags.append(kitchen)

  kitchen_door = TagConfig()
  kitchen_door.index = TAG_KITCHEN_DOOR
  kitchen_door.type = c4004.TAG_TYPE_APPROACH_AWAY
  kitchen_door.range_type = c4004.TAG_RANGE_RECTANGLE
  kitchen_door.center_x = DOOR_CENTER_X_CM
  kitchen_door.center_y = DOOR_CENTER_Y_CM
  kitchen_door.x_size = DOOR_SIZE_X_CM
  kitchen_door.y_size = DOOR_SIZE_Y_CM
  tags.append(kitchen_door)

  if c4004.set_tags_from_config(tags):
    print('Set living/kitchen/door tags success.')
  else:
    print('Set living/kitchen/door tags failed.')

  if c4004.set_trajectory_track_enable(True):
    print('Set trajectory track enable success.')
  else:
    print('Set trajectory track enable failed.')

  if c4004.set_trajectory_hold_time(1):
    print('Set trajectory hold time success.')
  else:
    print('Set trajectory hold time failed.')

  print('============================================================')
  print('Kitchen occupancy inference started.')
  print('Direction: approach coordinate zone + leave coordinate zone.')
  print('Living-room count is printed only and does not affect kitchen state.')
  print('Kitchen zone is configured but not used as direct occupancy evidence.')
  print('Result is logical inference, not direct kitchen presence detection.')
  print('============================================================')


def print_status():
  print('============================================================')
  print('Living motion/static    : %d/%d' % (living_motion_count, living_static_count))
  print('Living people           : %d' % living_people_count)
  print('Door session active     : %s' % ('YES' if door_session_active else 'NO'))
  print('Door start zone         : %s' % zone_to_text(door_start_zone))
  print('Door start coordinate   : (%d, %d)' % (door_start_target_x, door_start_target_y))
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

    check_door_session_timeout()

    if now_s - last_print_s >= 1.0:
      last_print_s = now_s
      print_status()


if __name__ == '__main__':
  main()
