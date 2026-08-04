/*!
 * @file roomOccupancyInference.ino
 * @brief Infer kitchen occupancy from kitchen-door boundary tag events.
 * @details This example configures living-room, kitchen, and kitchen-door tags.
 * @n The kitchen-door tag is Boundary relative to the living-room range.
 * @n Enter-living-room events decrement the kitchen people count, and
 * @n exit-living-room events increment it.
 * @n Usage environment:
 * @n - Please install the sensor at a height of 180 cm for use.
 * @copyright Copyright (c) 2026 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @license The MIT License (MIT)
 * @author JiaLi(jia.li@dfrobot.com)
 * @version V1.0.0
 * @date 2026-05-22
 * @url https://github.com/DFRobot/DFRobot_C4004
 */

#include "DFRobot_C4004.h"

/* ---------------------------------------------------------------------------------------------------------------------
 *    board   |             MCU                | Leonardo/Mega2560/M0 |    UNO    | ESP8266 | ESP32 |  microbit  |   M0  |
 *     VCC    |              5V                |         5V           |     5V    |    5V   |   5V  |     X      |   5V  |
 *     GND    |              GND               |        GND           |    GND    |   GND   |  GND  |     X      |  GND  |
 *     RX     |              TX                |     Serial1 TX1      |     5     |   5     |  D3   |     X      |  TX1  |
 *     TX     |              RX                |     Serial1 RX1      |     4     |   4     |  D2   |     X      |  RX1  |
 * ----------------------------------------------------------------------------------------------------------------------*/
/* Baud rate is fixed at 115200. SoftSerial mode cannot guarantee stable data communication! */

#if defined(ESP8266) || defined(ARDUINO_AVR_UNO)
SoftwareSerial mySerial(4, 5);
DFRobot_C4004  c4004(&mySerial, 115200);
#elif defined(ESP32)
DFRobot_C4004 c4004(&Serial1, 115200, /*D2*/ D2, /*D3*/ D3);
#else
DFRobot_C4004 c4004(&Serial1, 115200);
#endif

const uint8_t tagLivingRoom  = 0;
const uint8_t tagKitchen     = 1;    // Configured only; kitchen people count uses door enter/exit events.
const uint8_t tagKitchenDoor = 2;

const uint8_t  checkToActiveFrames = 2;
const uint32_t noPersonDelayS      = 5;
const uint32_t trackExistsTimeS    = 1;

const int16_t  livingRoomCenterXCm = 0;
const int16_t  livingRoomCenterYCm = 200;
/* Rectangle SizeX/SizeY map to tag width/height: width along X-axis, height along Y-axis (cm). */
const uint16_t livingRoomSizeXCm   = 400;
const uint16_t livingRoomSizeYCm   = 400;

const int16_t  kitchenCenterXCm = 0;
const int16_t  kitchenCenterYCm = 600;
const uint16_t kitchenSizeXCm   = 200;
const uint16_t kitchenSizeYCm   = 400;

const int16_t  doorCenterXCm = 0;
const int16_t  doorCenterYCm = 400;
const uint16_t doorSizeXCm   = 100;
const uint16_t doorSizeYCm   = 100;

uint8_t livingMotionCount = 0;
uint8_t livingStaticCount = 0;
uint8_t livingPeopleCount = 0;

uint16_t    kitchenDoorEnterCount = 0;
uint16_t    kitchenDoorExitCount  = 0;
int16_t     kitchenInferredPeople = 0;
const char *lastDoorEvent         = "None";

const char *doorEventToText(DFRobot_C4004::eBoundaryDirection_t enterExit)
{
  if (enterExit == DFRobot_C4004::eEnter) {
    return "Enter living room";
  }
  if (enterExit == DFRobot_C4004::eExit) {
    return "Exit living room";
  }
  return "None";
}

void processLivingRoomTag(const DFRobot_C4004::sTagInfo_t &tagInfo)
{
  livingMotionCount = tagInfo.motionNum;
  livingStaticCount = tagInfo.staticNum;
  livingPeopleCount = (uint8_t)(livingMotionCount + livingStaticCount);
}

void processKitchenDoorTag(const DFRobot_C4004::sTagInfo_t &tagInfo)
{
  if (tagInfo.enterExit == DFRobot_C4004::eEnter) {
    kitchenDoorEnterCount++;
    if (kitchenInferredPeople > 0) {
      kitchenInferredPeople--;
    }
    lastDoorEvent = "Enter living room";
  } else if (tagInfo.enterExit == DFRobot_C4004::eExit) {
    kitchenDoorExitCount++;
    kitchenInferredPeople++;
    lastDoorEvent = "Exit living room";
  } else {
    lastDoorEvent = "None";
  }

  Serial.println(F("------------------------------------------------------------"));
  Serial.print(F("Kitchen door event      : "));
  Serial.println(doorEventToText(tagInfo.enterExit));
  Serial.print(F("Kitchen inferred people : "));
  Serial.println(kitchenInferredPeople);
}

void processTagEvent()
{
  DFRobot_C4004::sTagInfo_t tagInfo;
  if (!c4004.getTagInfo(&tagInfo)) {
    return;
  }

  if (tagInfo.tagIndex == tagLivingRoom && tagInfo.tagType == DFRobot_C4004::eTagPeopleCounting) {
    processLivingRoomTag(tagInfo);
  } else if (tagInfo.tagIndex == tagKitchenDoor && tagInfo.tagType == DFRobot_C4004::eTagBoundary) {
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

  // Side mount: default 180 cm, recommended 180±20 cm. Top mount: recommended 220-280 cm.
  if (c4004.setInstallHeight(180)) {
    Serial.println(F("Set install height success."));
  } else {
    Serial.println(F("Set install height failed."));
  }
  delay(50);

  if (c4004.setPresenceEnable(true)) {
    Serial.println(F("Set presence enable success."));
  } else {
    Serial.println(F("Set presence enable failed."));
  }

  if (c4004.setFrameGenerateCount(checkToActiveFrames)) {
    Serial.println(F("Set check-to-active frames success."));
  } else {
    Serial.println(F("Set check-to-active frames failed."));
  }
  delay(50);

  uint8_t checkToActiveFrames = 0;
  if (c4004.getFrameGenerateCount(&checkToActiveFrames)) {
    Serial.print(F("Current check-to-active frames: "));
    Serial.println(checkToActiveFrames);
  } else {
    Serial.println(F("Read current check-to-active frames failed."));
  }

  DFRobot_C4004::sFourSidedRange_t range;
  range.xMax = 200;
  range.xMin = -200;
  range.yMax = 400;
  range.yMin = 0;
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

  DFRobot_C4004::sTagConfig_t tags[3] = {};

  tags[0].tagIndex  = tagLivingRoom;
  tags[0].tagType   = DFRobot_C4004::eTagPeopleCounting;
  tags[0].scopeType = DFRobot_C4004::eRectangle;
  tags[0].ioIndex   = 0;
  tags[0].centerX   = livingRoomCenterXCm;
  tags[0].centerY   = livingRoomCenterYCm;
  tags[0].width     = livingRoomSizeXCm;
  tags[0].height    = livingRoomSizeYCm;

  tags[1].tagIndex  = tagKitchen;
  tags[1].tagType   = DFRobot_C4004::eTagPeopleCounting;
  tags[1].scopeType = DFRobot_C4004::eRectangle;
  tags[1].ioIndex   = 0;
  tags[1].centerX   = kitchenCenterXCm;
  tags[1].centerY   = kitchenCenterYCm;
  tags[1].width     = kitchenSizeXCm;
  tags[1].height    = kitchenSizeYCm;

  tags[2].tagIndex  = tagKitchenDoor;
  tags[2].tagType   = DFRobot_C4004::eTagBoundary;
  tags[2].scopeType = DFRobot_C4004::eRectangle;
  tags[2].ioIndex   = 0;
  tags[2].centerX   = doorCenterXCm;
  tags[2].centerY   = doorCenterYCm;
  tags[2].width     = doorSizeXCm;
  tags[2].height    = doorSizeYCm;

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

  if (c4004.setTrajectoryLifetime(trackExistsTimeS)) {
    Serial.println(F("Set TrajectoryLifetime success."));
  } else {
    Serial.println(F("Set TrajectoryLifetime failed."));
  }

  if (c4004.setUnoccupiedTime(noPersonDelayS)) {
    Serial.println(F("Set UnoccupiedTime success."));
  } else {
    Serial.println(F("Set UnoccupiedTime failed."));
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
  uint32_t         nowMs = millis();
  DFRobot_C4004::eReportedEvent_t event = c4004.getReportedEvent(50);

  if (event == DFRobot_C4004::eEventTag) {
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
  }
}
