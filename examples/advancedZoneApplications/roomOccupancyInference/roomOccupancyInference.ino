/*!
 * @file roomOccupancyInference.ino
 * @brief Infer kitchen occupancy from kitchen-door boundary tag events.
 * @details This example configures living-room, kitchen, and kitchen-door tags.
 * @n The kitchen-door tag is Boundary relative to the living-room range.
 * @n Enter-living-room events decrement the kitchen people count, and
 * @n exit-living-room events increment it.
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
const uint8_t TAG_KITCHEN = 1;       // Configured only; kitchen people count uses door enter/exit events.
const uint8_t TAG_KITCHEN_DOOR = 2;

const uint8_t CHECK_TO_ACTIVE_FRAMES = 2;
const uint32_t NO_PERSON_DELAY_S = 5;
const uint32_t TRACK_EXISTS_TIME_S = 1;

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
const uint16_t DOOR_SIZE_Y_CM = 100;

uint8_t livingMotionCount = 0;
uint8_t livingStaticCount = 0;
uint8_t livingPeopleCount = 0;

uint16_t kitchenDoorEnterCount = 0;
uint16_t kitchenDoorExitCount = 0;
int16_t kitchenInferredPeople = 0;
bool kitchenOccupied = false;
const char *lastDoorEvent = "None";

const char *doorEventToText(uint8_t enterExit)
{
  if (enterExit == 0) {
    return "Enter living room";
  }
  if (enterExit == 1) {
    return "Exit living room";
  }
  return "Unknown";
}

void processLivingRoomTag(const sTagInfo_t &tagInfo)
{
  livingMotionCount = tagInfo.motionNum;
  livingStaticCount = tagInfo.staticNum;
  livingPeopleCount = (uint8_t)(livingMotionCount + livingStaticCount);
}

void processKitchenDoorTag(const sTagInfo_t &tagInfo)
{
  if (tagInfo.enterExit == 0) {
    kitchenDoorEnterCount++;
    if (kitchenInferredPeople > 0) {
      kitchenInferredPeople--;
    }
    lastDoorEvent = "Enter living room";
  } else if (tagInfo.enterExit == 1) {
    kitchenDoorExitCount++;
    kitchenInferredPeople++;
    lastDoorEvent = "Exit living room";
  } else {
    lastDoorEvent = "Unknown";
  }

  kitchenOccupied = (kitchenInferredPeople > 0);

  Serial.println(F("------------------------------------------------------------"));
  Serial.print(F("Kitchen door event      : "));
  Serial.println(doorEventToText(tagInfo.enterExit));
  Serial.print(F("Kitchen inferred people : "));
  Serial.println(kitchenInferredPeople);
  Serial.print(F("Kitchen occupied        : "));
  Serial.println(kitchenOccupied ? F("YES") : F("NO"));
}

void processTagEvent()
{
  sTagInfo_t tagInfo;
  if (!c4004.getTagInfo(&tagInfo)) {
    return;
  }

  if (tagInfo.tagIndex == TAG_LIVING_ROOM && tagInfo.tagType == eTagPeopleCounting) {
    processLivingRoomTag(tagInfo);
  } else if (tagInfo.tagIndex == TAG_KITCHEN_DOOR && tagInfo.tagType == eTagBoundary) {
    processKitchenDoorTag(tagInfo);
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

  if (c4004.setCheckToActiveFrames(CHECK_TO_ACTIVE_FRAMES)) {
    Serial.println(F("Set check-to-active frames success."));
  } else {
    Serial.println(F("Set check-to-active frames failed."));
  }
  delay(50);

  uint8_t checkToActiveFrames = 0;
  if (c4004.getCheckToActiveFrames(&checkToActiveFrames)) {
    Serial.print(F("Current check-to-active frames: "));
    Serial.println(checkToActiveFrames);
  } else {
    Serial.println(F("Read current check-to-active frames failed."));
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

  sTagConfig_t tags[3] = {};

  tags[0].tagIndex = TAG_LIVING_ROOM;
  tags[0].tagType = eTagPeopleCounting;
  tags[0].scopeType = eTagRangeRectangle;
  tags[0].ioIndex = 0;
  tags[0].centerX = LIVING_ROOM_CENTER_X_CM;
  tags[0].centerY = LIVING_ROOM_CENTER_Y_CM;
  tags[0].width = LIVING_ROOM_SIZE_X_CM;
  tags[0].height = LIVING_ROOM_SIZE_Y_CM;

  tags[1].tagIndex = TAG_KITCHEN;
  tags[1].tagType = eTagPeopleCounting;
  tags[1].scopeType = eTagRangeRectangle;
  tags[1].ioIndex = 0;
  tags[1].centerX = KITCHEN_CENTER_X_CM;
  tags[1].centerY = KITCHEN_CENTER_Y_CM;
  tags[1].width = KITCHEN_SIZE_X_CM;
  tags[1].height = KITCHEN_SIZE_Y_CM;

  tags[2].tagIndex = TAG_KITCHEN_DOOR;
  tags[2].tagType = eTagBoundary;
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

  if (c4004.setTrackExistsTime(TRACK_EXISTS_TIME_S)) {
    Serial.println(F("Set TrackExistsTime success."));
  } else {
    Serial.println(F("Set TrackExistsTime failed."));
  }

  if (c4004.setUnmannedTime(NO_PERSON_DELAY_S)) {
    Serial.println(F("Set UnmannedTime success."));
  } else {
    Serial.println(F("Set UnmannedTime failed."));
  } 

  Serial.println(F("============================================================"));
  Serial.println(F("Kitchen occupancy inference started."));
  Serial.println(F("Direction: kitchen-door Boundary tag event relative to living room."));
  Serial.println(F("Kitchen people count decrements on Enter living room and increments on Exit living room."));
  Serial.println(F("Living-room count is printed only and does not affect kitchen state."));
  Serial.println(F("Kitchen tag is configured for range/tag testing only."));
  Serial.println(F("============================================================"));
}

void loop()
{
  uint32_t nowMs = millis();
  eReportedEvent_t event = c4004.getReportedInfo(50);

  if (event == eEventTag) {
    processTagEvent();
  }

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
    Serial.print(F("Last kitchen door event : "));
    Serial.println(lastDoorEvent);
    Serial.print(F("Door enter/exit count   : "));
    Serial.print(kitchenDoorEnterCount);
    Serial.print(F("/"));
    Serial.println(kitchenDoorExitCount);
    Serial.print(F("Kitchen inferred people : "));
    Serial.println(kitchenInferredPeople);
    Serial.print(F("Kitchen occupied        : "));
    Serial.println(kitchenOccupied ? F("YES") : F("NO"));
  }
}
