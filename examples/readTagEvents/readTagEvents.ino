/*!
 * @file readTagEvents.ino
 * @brief Configure tag zones and print live tag event reports.
 * @details Use this example to configure tag zones and watch live tag events over Serial,
 * @n such as enter/exit, approach/away, people counting and noise-zone reports.
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

void printCol(const __FlashStringHelper *text, uint8_t width)
{
  uint8_t len = strlen_P(reinterpret_cast<PGM_P>(text));
  Serial.print(text);
  for (uint8_t i = len; i < width; i++) {
    Serial.print(' ');
  }
}

void printCol(long value, uint8_t width)
{
  char buf[12];
  ltoa(value, buf, 10);
  Serial.print(buf);
  for (uint8_t i = (uint8_t)strlen(buf); i < width; i++) {
    Serial.print(' ');
  }
}

const __FlashStringHelper *tagTypeText(DFRobot_C4004::eTagType_t type)
{
  switch (type) {
    case DFRobot_C4004::eTagNone:
      return F("None");
    case DFRobot_C4004::eTagBoundary:
      return F("Boundary");
    case DFRobot_C4004::eTagApproachAway:
      return F("ApproachAway");
    case DFRobot_C4004::eTagPeopleCounting:
      return F("PeopleCounting");
    case DFRobot_C4004::eTagNoise:
      return F("Noise");
    default:
      return F("Unknown");
  }
}

void printTagEvent(const DFRobot_C4004::sTagInfo_t &info)
{
  Serial.println(F("======================================================================"));
  Serial.println(F("============================TagEventReport============================"));
  Serial.println(F("======================================================================"));
  Serial.print(F("Tag Index : "));
  Serial.println(info.tagIndex);
  Serial.print(F("Tag Type  : "));
  Serial.println(tagTypeText(info.tagType));
  Serial.print(F("IO Index  : "));
  Serial.println(info.ioIndex);
  Serial.print(F("Center XY : "));
  Serial.print(info.centerX);
  Serial.print(F(" / "));
  Serial.println(info.centerY);

  if (info.tagType == DFRobot_C4004::eTagBoundary) {
    Serial.print(F("Event     : Boundary ("));
    if (info.enterExit == DFRobot_C4004::eEnter) {
      Serial.print(F("Enter"));
    } else if (info.enterExit == DFRobot_C4004::eExit) {
      Serial.print(F("Exit"));
    } else {
      Serial.print(F("None"));
    }
    Serial.println(F(")"));
  } else if (info.tagType == DFRobot_C4004::eTagApproachAway) {
    Serial.print(F("Event     : MotionDirection ("));
    if (info.motionDir == DFRobot_C4004::eApproach) {
      Serial.print(F("Approach"));
    } else if (info.motionDir == DFRobot_C4004::eAway) {
      Serial.print(F("Away"));
    } else {
      Serial.print(F("None"));
    }
    Serial.println(F(")"));
  } else if (info.tagType == DFRobot_C4004::eTagPeopleCounting) {
    Serial.print(F("Event     : PeopleCounting (M:"));
    Serial.print(info.motionNum);
    Serial.print(F(" S:"));
    Serial.print(info.staticNum);
    Serial.println(F(")"));
  } else {
    Serial.print(F("Event     : "));
    Serial.println(tagTypeText(info.tagType));
  }
  Serial.println();
}

void printTagList(const __FlashStringHelper *title, DFRobot_C4004::sTagConfig_t *tags, uint8_t count)
{
  Serial.println(F("======================================================================"));
  Serial.println(title);
  Serial.println(F("----------------------------------------------------------------------"));
  Serial.print(F("Tag count: "));
  Serial.println(count);
  if (count == 0) {
    Serial.println(F("No tag."));
    Serial.println();
    return;
  }

  printCol(F("Idx"), 5);
  printCol(F("Type"), 16);
  printCol(F("Range"), 11);
  printCol(F("IO"), 4);
  printCol(F("CenterX"), 9);
  printCol(F("CenterY"), 9);
  printCol(F("Width"), 7);
  Serial.println(F("Height"));
  for (uint8_t i = 0; i < count; i++) {
    printCol((long)tags[i].tagIndex, 5);
    printCol(tagTypeText(tags[i].tagType), 16);
    printCol(tags[i].scopeType == DFRobot_C4004::eTagRangeCircle ? F("Circle") : F("Rectangle"), 11);
    printCol((long)tags[i].ioIndex, 4);
    printCol((long)tags[i].centerX, 9);
    printCol((long)tags[i].centerY, 9);
    printCol((long)tags[i].width, 7);
    Serial.println(tags[i].height);
  }
  Serial.println();
}

void setup()
{
  Serial.begin(115200);

  while (!c4004.begin()) {
    Serial.println(F("DFRobot C4004 begin failed, retrying..."));
    delay(1000);
  }
  Serial.println("DFRobot C4004 begin success.");

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

  DFRobot_C4004::sTagConfig_t setTags[5] = {};

  /**
   * Tag type overview:
   *   eTagNone           : Invalid/unused tag type.
   *   eTagBoundary       : Edge/boundary tag, usually placed at a zone edge;
   *                        reports Enter/Exit when a person passes through.
   *   eTagApproachAway   : Approach/away tag; reports Approach/Away relative to the tag zone.
   *   eTagPeopleCounting : Status/people-counting tag; reports moving and stationary
   *                        people counts inside the tag zone.
   *   eTagNoise          : Noise/interference tag; marks the zone as an interference area.
   *
   * Rectangle size note:
   *   width  = size along the X-axis (cm), relative to centerX
   *   height = size along the Y-axis (cm), relative to centerY
   * Circle size note:
   *   width  = radius (cm), height is ignored
   */

  // Set tag 0, type is None, range is Rectangle, center is (0, 100), width/height is (120, 120)
  // eTagNone: invalid/unused tag, does not generate meaningful tag events.
  setTags[0].tagIndex  = 0;
  setTags[0].tagType   = DFRobot_C4004::eTagNone;
  setTags[0].scopeType = DFRobot_C4004::eTagRangeRectangle;
  setTags[0].ioIndex   = 0;
  setTags[0].centerX   = 0;
  setTags[0].centerY   = 100;
  setTags[0].width     = 120;    // X-axis size (cm)
  setTags[0].height    = 120;    // Y-axis size (cm)

  // Set tag 1, type is Boundary, range is Rectangle, center is (100, 220), width/height is (120, 120)
  // eTagBoundary: edge tag; reports Enter/Exit when a person crosses this zone.
  setTags[1].tagIndex  = 1;
  setTags[1].tagType   = DFRobot_C4004::eTagBoundary;
  setTags[1].scopeType = DFRobot_C4004::eTagRangeRectangle;
  setTags[1].ioIndex   = 0;
  setTags[1].centerX   = 100;
  setTags[1].centerY   = 220;
  setTags[1].width     = 120;    // X-axis size (cm)
  setTags[1].height    = 120;    // Y-axis size (cm)

  // Set tag 2, type is ApproachAway, range is Circle, center is (-80, 350), radius is 80
  // eTagApproachAway: reports Approach/Away relative to this tag zone.
  setTags[2].tagIndex  = 2;
  setTags[2].tagType   = DFRobot_C4004::eTagApproachAway;
  setTags[2].scopeType = DFRobot_C4004::eTagRangeCircle;
  setTags[2].ioIndex   = 0;
  setTags[2].centerX   = -80;
  setTags[2].centerY   = 350;
  setTags[2].width     = 80;     // Circle radius (cm)
  setTags[2].height    = 0;      // Ignored for circle

  // Set tag 3, type is PeopleCounting, range is Rectangle, center is (0, 500), width/height is (160, 160)
  // eTagPeopleCounting: counts moving and stationary people inside the tag zone.
  setTags[3].tagIndex  = 3;
  setTags[3].tagType   = DFRobot_C4004::eTagPeopleCounting;
  setTags[3].scopeType = DFRobot_C4004::eTagRangeRectangle;
  setTags[3].ioIndex   = 0;
  setTags[3].centerX   = 0;
  setTags[3].centerY   = 500;
  setTags[3].width     = 160;    // X-axis size (cm)
  setTags[3].height    = 160;    // Y-axis size (cm)

  // Set tag 4, type is Noise, range is Rectangle, center is (-100, 620), width/height is (100, 120)
  // eTagNoise: marks this zone as an interference/noise area.
  setTags[4].tagIndex  = 4;
  setTags[4].tagType   = DFRobot_C4004::eTagNoise;
  setTags[4].scopeType = DFRobot_C4004::eTagRangeRectangle;
  setTags[4].ioIndex   = 0;
  setTags[4].centerX   = -100;
  setTags[4].centerY   = 620;
  setTags[4].width     = 100;    // X-axis size (cm)
  setTags[4].height    = 120;    // Y-axis size (cm)

  const uint8_t setTagCount = (uint8_t)(sizeof(setTags) / sizeof(setTags[0]));
  if (c4004.setTagsFromConfig(setTags, setTagCount)) {
    Serial.println(F("Set 5 tags from config success."));
  } else {
    Serial.println(F("Set 5 tags from config failed."));
  }

  uint8_t count = c4004.getTags(setTags, setTagCount);
  printTagList(F("Active tag list after setup:"), setTags, count);

  Serial.println(F("Setup done. Start detecting tag events..."));
  Serial.println(F("Move in/out of configured zones to trigger reports."));
  Serial.println();
}

void loop()
{
  DFRobot_C4004::sTagInfo_t       tagInfo;
  DFRobot_C4004::eReportedEvent_t event = c4004.getReportedEvent(100);
  /*
   * When state or data changes and the corresponding report function is enabled,
   * the module pushes the update immediately as an event via getReportedEvent().
   * Use the matching getter with DFRobot_C4004::eGetDataReport to read the cached value
   * updated by that report, without issuing an extra UART query.
   */

  if (event == DFRobot_C4004::eEventTag) {
    if (c4004.getTagInfo(&tagInfo)) {
      printTagEvent(tagInfo);
    }
  } else {
    static uint32_t lastStatus = 0;
    if (millis() - lastStatus > 3000) {
      lastStatus = millis();
      Serial.println(F("Detecting... waiting for tag events."));
    }
  }
}
