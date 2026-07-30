/*!
 * @file bedStillnessDetection.ino
 * @brief Bed-zone stillness lighting control using active polling APIs.
 * @details Rule summary:
 * @n 1) If any static person exists in bed area for over 5s, turn room light OFF.
 * @n 2) While rule 1 is active, new people entering room keeps light OFF.
 * @n 3) If bed area has no static person, keep light ON when room has people.
 * @n 4) After room transitions from occupied to empty, wait 5s then turn light OFF.
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

const uint8_t bedTagIndex         = 0;
const uint8_t bedroomTagIndex     = 1;
const uint8_t bedroomDoorTagIndex = 2;
const uint8_t lightCtrlPin        = 3;

// Users can adjust these times according to their own preferences, needs, application scenarios, etc. The default time is 5 seconds.
const uint32_t bedStaticHoldMs    = 5000;    // The time required to maintain a transition from other states to rest.
const uint32_t bedroomEmptyHoldMs = 5000;    // The time required to maintain a transition from other states to empty.

const uint8_t lightOffLevel = HIGH;
const uint8_t lightOnLevel  = LOW;

uint32_t bedStaticStartMs    = 0;
uint32_t bedroomEmptyStartMs = 0;
bool     bedStaticLockOff    = false;
bool     bedroomWasOccupied  = false;
uint8_t  bedMotionCount      = 0;
uint8_t  bedStaticCount      = 0;
uint8_t  bedroomMotionCount  = 0;
uint8_t  bedroomStaticCount  = 0;

void setup()
{
  Serial.begin(115200);

  pinMode(lightCtrlPin, OUTPUT);
  digitalWrite(lightCtrlPin, lightOffLevel);

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

  if (c4004.setFrameGenerateCount(7)) {
    Serial.println(F("Set check-to-active frames success."));
  } else {
    Serial.println(F("Set check-to-active frames failed."));
  }
  delay(50);

  if (c4004.setPresenceEnable(true)) {
    Serial.println(F("Set presence enable success."));
  } else {
    Serial.println(F("Set presence enable failed."));
  }

  DFRobot_C4004::sFourSidedRange_t range;
  range.xMax = 200;
  range.xMin = -200;
  range.yMax = 700;
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

  DFRobot_C4004::sTagConfig_t setTags[3] = {};

  /**
   * Note: For rectangle tags, width is the size along the X-axis and height is
   * the size along the Y-axis (unit: cm), relative to centerX/centerY.
   * For circle tags, width is the radius and height is ignored.
  */

  setTags[0].tagIndex  = bedTagIndex;
  setTags[0].tagType   = DFRobot_C4004::eTagPeopleCounting;
  setTags[0].scopeType = DFRobot_C4004::eTagRangeRectangle;
  setTags[0].ioIndex   = 0;
  setTags[0].centerX   = -50;
  setTags[0].centerY   = 300;
  setTags[0].width     = 300;
  setTags[0].height    = 250;

  setTags[1].tagIndex  = bedroomTagIndex;
  setTags[1].tagType   = DFRobot_C4004::eTagPeopleCounting;
  setTags[1].scopeType = DFRobot_C4004::eTagRangeRectangle;
  setTags[1].ioIndex   = 0;
  setTags[1].centerX   = 0;
  setTags[1].centerY   = 350;
  setTags[1].width     = 400;
  setTags[1].height    = 700;

  setTags[2].tagIndex  = bedroomDoorTagIndex;
  setTags[2].tagType   = DFRobot_C4004::eTagApproachAway;
  setTags[2].scopeType = DFRobot_C4004::eTagRangeRectangle;
  setTags[2].ioIndex   = 0;
  setTags[2].centerX   = 100;
  setTags[2].centerY   = 700;
  setTags[2].width     = 80;
  setTags[2].height    = 40;

  if (c4004.setTagsFromConfig(setTags, 3)) {
    Serial.println(F("Set bed/bedroom/door tags success."));
  } else {
    Serial.println(F("Set bed/bedroom/door tags failed."));
  }

  Serial.println(F("============================================================"));
  Serial.println(F("Bed stillness light control started."));
  Serial.println(F("Rule A: bed static(any person) over 5s => LIGHT OFF."));
  Serial.println(F("Rule B: if rule A active, bedroom new entry still keeps OFF."));
  Serial.println(F("Rule C: if rule A inactive, bedroom people>0 => LIGHT ON."));
  Serial.println(F("Rule D: bedroom occupied->empty over 5s => LIGHT OFF."));
  Serial.println(F("============================================================"));
}

void updatePeopleCountsFromTagReport()
{
  for (uint8_t i = 0; i < 4; i++) {
    DFRobot_C4004::eReportedEvent_t event = c4004.getReportedEvent(5);
    if (event == DFRobot_C4004::eEventTag) {
      DFRobot_C4004::sTagInfo_t tagInfo;
      if (c4004.getTagInfo(&tagInfo) && tagInfo.tagType == DFRobot_C4004::eTagPeopleCounting) {
        if (tagInfo.tagIndex == bedTagIndex) {
          bedMotionCount = tagInfo.motionNum;
          bedStaticCount = tagInfo.staticNum;
        } else if (tagInfo.tagIndex == bedroomTagIndex) {
          bedroomMotionCount = tagInfo.motionNum;
          bedroomStaticCount = tagInfo.staticNum;
        }
      }
    }
  }
}

void loop()
{
  uint32_t nowMs = millis();

  updatePeopleCountsFromTagReport();

  bool    bedHasStaticPerson = (bedStaticCount > 0);
  uint8_t bedroomPeopleCount = (uint8_t)(bedroomMotionCount + bedroomStaticCount);
  bool    bedroomHasPeople   = (bedroomPeopleCount > 0);

  if (bedHasStaticPerson) {
    if (bedStaticStartMs == 0) {
      bedStaticStartMs = nowMs;
    } else if ((uint32_t)(nowMs - bedStaticStartMs) >= bedStaticHoldMs) {
      bedStaticLockOff = true;
    }
  } else {
    bedStaticStartMs = 0;
    bedStaticLockOff = false;
  }

  bool lightShouldOff = false;

  if (bedStaticLockOff) {
    lightShouldOff      = true;
    bedroomEmptyStartMs = 0;
    bedroomWasOccupied  = bedroomHasPeople;
  } else {
    if (bedroomHasPeople) {
      bedroomWasOccupied  = true;
      bedroomEmptyStartMs = 0;
      lightShouldOff      = false;
    } else {
      if (bedroomWasOccupied) {
        if (bedroomEmptyStartMs == 0) {
          bedroomEmptyStartMs = nowMs;
        } else if ((uint32_t)(nowMs - bedroomEmptyStartMs) >= bedroomEmptyHoldMs) {
          lightShouldOff = true;
        }
      } else {
        lightShouldOff = true;
      }
    }
  }

  digitalWrite(lightCtrlPin, lightShouldOff ? lightOffLevel : lightOnLevel);

  static uint32_t lastPrintMs = 0;
  if ((uint32_t)(nowMs - lastPrintMs) >= 1000) {
    lastPrintMs = nowMs;
    Serial.println(F("============================================================"));
    Serial.print(F("Bedroom people count     : "));
    Serial.println(bedroomPeopleCount);
    Serial.print(F("Bedroom motion/static    : "));
    Serial.print(bedroomMotionCount);
    Serial.print(F("/"));
    Serial.println(bedroomStaticCount);
    Serial.print(F("Bed motion/static count  : "));
    Serial.print(bedMotionCount);
    Serial.print(F("/"));
    Serial.println(bedStaticCount);
    Serial.print(F("Bed static hold active   : "));
    Serial.println(bedStaticLockOff ? F("YES") : F("NO"));
    Serial.print(F("Light pin(3)             : "));
    Serial.println(digitalRead(lightCtrlPin) == lightOffLevel ? F("OFF") : F("ON"));
  }

  delay(100);
}
