/*!
 * @file roomOccupancyInference.ino
 * @brief Infer kitchen occupancy from doorway crossing coordinates.
 * @details This example configures living-room, kitchen, and kitchen-door tags.
 * @n The kitchen-door tag is ApproachAway. Each crossing session starts when a
 * @n target approaches the door and ends when a target moves away from the door.
 * @n For both events, the closest active target to the door center is used.
 * @n Direction is inferred from the start/end coordinate zones only.
 * @n This is logical inference only, not direct detection inside the kitchen.
 * @copyright Copyright (c) 2026 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @license The MIT License (MIT)
 * @author JiaLi(zhixin.liu@dfrobot.com)
 * @version V1.0.0
 * @date 2026-05-25
 * @url https://github.com/DFRobot/DFRobot_C4004
 */

#include "DFRobot_C4004.h"

#if defined(ESP8266) || defined(ARDUINO_AVR_UNO)
SoftwareSerial mySerial(4, 5);
DFRobot_C4004 c4004(&mySerial, 115200);
#elif defined(ESP32)
DFRobot_C4004 c4004(&Serial1, 115200, /*D2*/ D2, /*D3*/ D3);
#else
DFRobot_C4004 c4004(&Serial1, 115200);
#endif

const uint8_t TAG_LIVING_ROOM = 0;
const uint8_t TAG_KITCHEN = 1;       // Configured only; not used as direct occupancy evidence.
const uint8_t TAG_KITCHEN_DOOR = 2;

const int16_t LIVING_ROOM_CENTER_X_CM = 0;
const int16_t LIVING_ROOM_CENTER_Y_CM = 200;
const uint16_t LIVING_ROOM_SIZE_X_CM = 400;
const uint16_t LIVING_ROOM_SIZE_Y_CM = 400;

const int16_t KITCHEN_CENTER_X_CM = 0;
const int16_t KITCHEN_CENTER_Y_CM = 600;
const uint16_t KITCHEN_SIZE_X_CM = 200;
const uint16_t KITCHEN_SIZE_Y_CM = 400;

const int16_t DOOR_CENTER_X_CM = 0;
const int16_t DOOR_CENTER_Y_CM = 400;
const uint16_t DOOR_SIZE_X_CM = 100;
const uint16_t DOOR_SIZE_Y_CM = 50;

const uint32_t LINK_WINDOW_MS = 4000;

typedef enum {
  eInferNone = 0,
  eInferEnterKitchen,
  eInferExitKitchen
} eInferDir_t;

typedef enum {
  eZoneUnknown = 0,
  eZoneLivingRoom,
  eZoneKitchen
} eZone_t;

uint8_t livingMotionCount = 0;
uint8_t livingStaticCount = 0;
uint8_t livingPeopleCount = 0;

bool kitchenOccupied = false;
int16_t kitchenInferredPeople = 0;

bool doorSessionActive = false;
eZone_t doorStartZone = eZoneUnknown;
int16_t doorStartTargetX = 0;
int16_t doorStartTargetY = 0;
uint32_t doorSessionStartMs = 0;

const char *lastEvidence = "None";

const char *inferDirToText(eInferDir_t dir)
{
  if (dir == eInferEnterKitchen) {
    return "EnterKitchen";
  } else if (dir == eInferExitKitchen) {
    return "ExitKitchen";
  }
  return "None";
}

const char *zoneToText(eZone_t zone)
{
  if (zone == eZoneLivingRoom) {
    return "LivingRoom";
  } else if (zone == eZoneKitchen) {
    return "Kitchen";
  }
  return "Unknown";
}

eZone_t getPointZone(int16_t x, int16_t y)
{
  int16_t livingMinX = LIVING_ROOM_CENTER_X_CM - (int16_t)(LIVING_ROOM_SIZE_X_CM / 2);
  int16_t livingMaxX = LIVING_ROOM_CENTER_X_CM + (int16_t)(LIVING_ROOM_SIZE_X_CM / 2);
  int16_t livingMinY = LIVING_ROOM_CENTER_Y_CM - (int16_t)(LIVING_ROOM_SIZE_Y_CM / 2);
  int16_t livingMaxY = LIVING_ROOM_CENTER_Y_CM + (int16_t)(LIVING_ROOM_SIZE_Y_CM / 2);

  int16_t kitchenMinX = KITCHEN_CENTER_X_CM - (int16_t)(KITCHEN_SIZE_X_CM / 2);
  int16_t kitchenMaxX = KITCHEN_CENTER_X_CM + (int16_t)(KITCHEN_SIZE_X_CM / 2);
  int16_t kitchenMinY = KITCHEN_CENTER_Y_CM - (int16_t)(KITCHEN_SIZE_Y_CM / 2);
  int16_t kitchenMaxY = KITCHEN_CENTER_Y_CM + (int16_t)(KITCHEN_SIZE_Y_CM / 2);

  bool inLivingRoom = (x >= livingMinX && x <= livingMaxX && y >= livingMinY && y < livingMaxY);
  bool inKitchen = (x >= kitchenMinX && x <= kitchenMaxX && y > kitchenMinY && y <= kitchenMaxY);

  if (inLivingRoom && !inKitchen) {
    return eZoneLivingRoom;
  } else if (inKitchen && !inLivingRoom) {
    return eZoneKitchen;
  }
  return eZoneUnknown;
}

bool getClosestTargetToDoor(sTargetInfo_t *target)
{
  if (target == NULL) {
    return false;
  }

  sTargetInfo_t targets[MAX_TARGETS];
  uint8_t count = c4004.getTargetList(targets, MAX_TARGETS, eGetDataActive);
  if (count == 0) {
    return false;
  }

  uint8_t closestIndex = 0;
  uint32_t closestDistSq = 0xFFFFFFFFUL;
  for (uint8_t i = 0; i < count; i++) {
    int32_t dx = (int32_t)targets[i].x - DOOR_CENTER_X_CM;
    int32_t dy = (int32_t)targets[i].y - DOOR_CENTER_Y_CM;
    uint32_t distSq = (uint32_t)(dx * dx + dy * dy);
    if (distSq < closestDistSq) {
      closestDistSq = distSq;
      closestIndex = i;
    }
  }

  *target = targets[closestIndex];
  return true;
}

void printCoordinateEvidence(const char *title, eZone_t startZone, int16_t startX, int16_t startY,
                             eZone_t endZone, int16_t endX, int16_t endY)
{
  Serial.println(F("------------------------------------------------------------"));
  Serial.println(title);
  Serial.print(F("Approach coordinate    : ("));
  Serial.print(startX);
  Serial.print(F(", "));
  Serial.print(startY);
  Serial.print(F("), "));
  Serial.println(zoneToText(startZone));
  Serial.print(F("Away coordinate        : ("));
  Serial.print(endX);
  Serial.print(F(", "));
  Serial.print(endY);
  Serial.print(F("), "));
  Serial.println(zoneToText(endZone));
}

void clearDoorSession()
{
  doorSessionActive = false;
  doorStartZone = eZoneUnknown;
  doorStartTargetX = 0;
  doorStartTargetY = 0;
  doorSessionStartMs = 0;
}

void confirmKitchenEvent(eInferDir_t dir, const char *evidence)
{
  if (dir == eInferNone) {
    return;
  }

  if (dir == eInferEnterKitchen) {
    kitchenInferredPeople++;
  } else if (kitchenInferredPeople > 0) {
    kitchenInferredPeople--;
  }

  kitchenOccupied = (kitchenInferredPeople > 0);
  lastEvidence = evidence;

  Serial.println(F("------------------------------------------------------------"));
  Serial.print(F("Kitchen inference event : "));
  Serial.println(inferDirToText(dir));
  Serial.print(F("Evidence                : "));
  Serial.println(evidence);
  Serial.print(F("Kitchen inferred people : "));
  Serial.println(kitchenInferredPeople);
  Serial.print(F("Kitchen occupied        : "));
  Serial.println(kitchenOccupied ? F("YES") : F("NO"));
}

void startDoorSession()
{
  sTargetInfo_t target;
  if (!getClosestTargetToDoor(&target)) {
    clearDoorSession();
    lastEvidence = "approach door: no target";
    Serial.println(F("------------------------------------------------------------"));
    Serial.println(F("Door session ignored    : approach door, no active target"));
    return;
  }

  doorSessionActive = true;
  doorStartZone = getPointZone(target.x, target.y);
  doorStartTargetX = target.x;
  doorStartTargetY = target.y;
  doorSessionStartMs = millis();
  lastEvidence = "approach door coordinate recorded";

  Serial.println(F("------------------------------------------------------------"));
  Serial.println(F("Door session started    : approach door"));
  Serial.print(F("Approach coordinate    : ("));
  Serial.print(doorStartTargetX);
  Serial.print(F(", "));
  Serial.print(doorStartTargetY);
  Serial.print(F("), "));
  Serial.println(zoneToText(doorStartZone));
}

void finishDoorSession()
{
  if (!doorSessionActive) {
    lastEvidence = "leave door without approach";
    Serial.println(F("------------------------------------------------------------"));
    Serial.println(F("Door session ignored    : leave door without approach"));
    return;
  }

  if ((uint32_t)(millis() - doorSessionStartMs) > LINK_WINDOW_MS) {
    lastEvidence = "door session timeout";
    Serial.println(F("------------------------------------------------------------"));
    Serial.println(F("Door session ignored    : timeout before leave door"));
    clearDoorSession();
    return;
  }

  sTargetInfo_t target;
  if (!getClosestTargetToDoor(&target)) {
    lastEvidence = "leave door: no target";
    Serial.println(F("------------------------------------------------------------"));
    Serial.println(F("Door session ignored    : leave door, no active target"));
    clearDoorSession();
    return;
  }

  eZone_t endZone = getPointZone(target.x, target.y);
  eInferDir_t dir = eInferNone;
  const char *evidence = "invalid door coordinate zones";

  if (doorStartZone == eZoneLivingRoom && endZone == eZoneKitchen) {
    dir = eInferEnterKitchen;
    evidence = "living room to kitchen crossing";
  } else if (doorStartZone == eZoneKitchen && endZone == eZoneLivingRoom) {
    dir = eInferExitKitchen;
    evidence = "kitchen to living room crossing";
  }

  printCoordinateEvidence(dir == eInferNone ? "Door crossing ignored  : invalid coordinate zones"
                                            : "Door crossing confirmed: valid coordinate zones",
                          doorStartZone, doorStartTargetX, doorStartTargetY,
                          endZone, target.x, target.y);

  if (dir == eInferNone) {
    lastEvidence = evidence;
  } else {
    confirmKitchenEvent(dir, evidence);
  }

  clearDoorSession();
}

void checkDoorSessionTimeout()
{
  if (!doorSessionActive) {
    return;
  }

  if ((uint32_t)(millis() - doorSessionStartMs) > LINK_WINDOW_MS) {
    lastEvidence = "door session timeout";
    Serial.println(F("------------------------------------------------------------"));
    Serial.println(F("Door session cleared    : timeout"));
    clearDoorSession();
  }
}

void processLivingRoomTag(const sTagInfo_t &tagInfo)
{
  livingMotionCount = tagInfo.motionNum;
  livingStaticCount = tagInfo.staticNum;
  livingPeopleCount = (uint8_t)(livingMotionCount + livingStaticCount);
}

void processTagEvent()
{
  sTagInfo_t tagInfo;
  if (!c4004.getTagInfo(&tagInfo)) {
    return;
  }

  if (tagInfo.tagIndex == TAG_LIVING_ROOM && tagInfo.tagType == eTagTypePeopleCounting) {
    processLivingRoomTag(tagInfo);
  } else if (tagInfo.tagIndex == TAG_KITCHEN_DOOR && tagInfo.tagType == eTagTypeApproachAway) {
    if (tagInfo.motionDir == 0) {
      startDoorSession();
    } else if (tagInfo.motionDir == 1) {
      finishDoorSession();
    }
  }
}

void setup()
{
  Serial.begin(115200);

  while (!c4004.begin()) {
    Serial.println(F("DFRobot C4004 begin failed, retrying..."));
    delay(1000);
  }
  Serial.println(F("DFRobot C4004 begin success."));

  if (c4004.setPresenceEnable(true)) {
    Serial.println(F("Set presence enable success."));
  } else {
    Serial.println(F("Set presence enable failed."));
  }

  sFourSidedRange range;
  range.mode = eRangeFourSide;
  range.xPositiveCm = 200;
  range.xNegativeCm = -200;
  range.yPositiveCm = 400;
  range.yNegativeCm = 0;
  if (c4004.setFourSidedRangeMode(range)) {
    Serial.println(F("Set boundary detection range success."));
  } else {
    Serial.println(F("Set boundary detection range failed."));
  }

  if (c4004.clearAllTags()) {
    Serial.println(F("Clear all tags success."));
  } else {
    Serial.println(F("Clear all tags failed."));
  }

  sTagConfig_t tags[3];

  tags[0].tagIndex = TAG_LIVING_ROOM;
  tags[0].tagType = eTagTypePeopleCounting;
  tags[0].scopeType = eTagRangeRectangle;
  tags[0].ioIndex = 0;
  tags[0].centerX = LIVING_ROOM_CENTER_X_CM;
  tags[0].centerY = LIVING_ROOM_CENTER_Y_CM;
  tags[0].width = LIVING_ROOM_SIZE_X_CM;
  tags[0].height = LIVING_ROOM_SIZE_Y_CM;

  tags[1].tagIndex = TAG_KITCHEN;
  tags[1].tagType = eTagTypePeopleCounting;
  tags[1].scopeType = eTagRangeRectangle;
  tags[1].ioIndex = 0;
  tags[1].centerX = KITCHEN_CENTER_X_CM;
  tags[1].centerY = KITCHEN_CENTER_Y_CM;
  tags[1].width = KITCHEN_SIZE_X_CM;
  tags[1].height = KITCHEN_SIZE_Y_CM;

  tags[2].tagIndex = TAG_KITCHEN_DOOR;
  tags[2].tagType = eTagTypeApproachAway;
  tags[2].scopeType = eTagRangeRectangle;
  tags[2].ioIndex = 0;
  tags[2].centerX = DOOR_CENTER_X_CM;
  tags[2].centerY = DOOR_CENTER_Y_CM;
  tags[2].width = DOOR_SIZE_X_CM;
  tags[2].height = DOOR_SIZE_Y_CM;

  if (c4004.setTagsFromConfig(tags, 3)) {
    Serial.println(F("Set living/kitchen/door tags success."));
  } else {
    Serial.println(F("Set living/kitchen/door tags failed."));
  }

  if (c4004.setTrajectoryTrackEnable(true)) {
    Serial.println(F("Set trajectory track enable success."));
  } else {
    Serial.println(F("Set trajectory track enable failed."));
  }

  if (c4004.setTrackExistsTime(1)) {
    Serial.println(F("Set TrackExistsTime success."));
  } else {
    Serial.println(F("Set TrackExistsTime failed."));
  }

  Serial.println(F("============================================================"));
  Serial.println(F("Kitchen occupancy inference started."));
  Serial.println(F("Direction: approach coordinate zone + leave coordinate zone."));
  Serial.println(F("Living-room count is printed only and does not affect kitchen state."));
  Serial.println(F("Kitchen zone is configured but not used as direct occupancy evidence."));
  Serial.println(F("Result is logical inference, not direct kitchen presence detection."));
  Serial.println(F("============================================================"));
}

void loop()
{
  uint32_t nowMs = millis();
  eReportedEvent_t event = c4004.getReportedInfo(50);

  if (event == eEventTag) {
    processTagEvent();
  }

  checkDoorSessionTimeout();

  static uint32_t lastPrintMs = 0;
  if ((uint32_t)(nowMs - lastPrintMs) >= 1000) {
    lastPrintMs = nowMs;
    Serial.println(F("============================================================"));
    Serial.print(F("Living motion/static    : "));
    Serial.print(livingMotionCount);
    Serial.print(F("/"));
    Serial.println(livingStaticCount);
    Serial.print(F("Living people           : "));
    Serial.println(livingPeopleCount);
    Serial.print(F("Door session active     : "));
    Serial.println(doorSessionActive ? F("YES") : F("NO"));
    Serial.print(F("Door start zone         : "));
    Serial.println(zoneToText(doorStartZone));
    Serial.print(F("Door start coordinate   : ("));
    Serial.print(doorStartTargetX);
    Serial.print(F(", "));
    Serial.print(doorStartTargetY);
    Serial.println(F(")"));
    // Serial.print(F("Last evidence           : "));
    // Serial.println(lastEvidence);
    Serial.print(F("Kitchen inferred people : "));
    Serial.println(kitchenInferredPeople);
    Serial.print(F("Kitchen occupied        : "));
    Serial.println(kitchenOccupied ? F("YES") : F("NO"));
  }
}
