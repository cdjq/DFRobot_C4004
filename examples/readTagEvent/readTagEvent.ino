/*!
 * @file readTagEvent.ino
 * @brief Configure tags and print tag region event reports.
 * @copyright Copyright (c) 2026 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @license The MIT License (MIT)
 * @author JiaLi(zhixin.liu@dfrobot.com)
 * @version V1.0.0
 * @date 2026-05-22
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

const __FlashStringHelper *tagTypeText(eTagType_t type)
{
  switch (type) {
    case eTagNone:           return F("None");
    case eTagBoundary:       return F("Boundary");
    case eTagApproachAway:   return F("ApproachAway");
    case eTagPeopleCounting: return F("PeopleCounting");
    case eTagNoise:          return F("Noise");
    default:                 return F("Unknown");
  }
}

void printTagEvent(const sTagInfo_t &info)
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

  if (info.tagType == eTagBoundary) {
    Serial.print(F("Event     : Boundary ("));
    if (info.enterExit == eBoundaryDirection_t::eEnter) {
      Serial.print(F("Enter"));
    } else if (info.enterExit == eBoundaryDirection_t::eExit) {
      Serial.print(F("Exit"));
    } else {
      Serial.print(F("None"));
    }
    Serial.println(F(")"));
  } else if (info.tagType == eTagApproachAway) {
    Serial.print(F("Event     : MotionDirection ("));
    if (info.motionDir == eApproachAwayDirection_t::eApproach) {
      Serial.print(F("Approach"));
    } else if (info.motionDir == eApproachAwayDirection_t::eAway) {
      Serial.print(F("Away"));
    } else {
      Serial.print(F("None"));
    }
    Serial.println(F(")"));
  } else if (info.tagType == eTagPeopleCounting) {
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

void printTagList(const __FlashStringHelper *title, sTagConfig_t *tags, uint8_t count)
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
    printCol(tags[i].scopeType == eTagRangeCircle ? F("Circle") : F("Rectangle"), 11);
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

  if (c4004.setCheckToActiveFrames(7)) {
    Serial.println(F("Set check-to-active frames success."));
  } else {
    Serial.println(F("Set check-to-active frames failed."));
  }
  delay(50);

  sFourSidedRange_t range;
  range.mode = eRangeFourSide;
  range.xPositiveCm = 200;
  range.xNegativeCm = -200;
  range.yPositiveCm = 700;
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

  sTagConfig_t setTags[5] = {};

  // Set tag 0, type is None, range is Rectangle, center is (0, 100), width/height is (120, 120)
  setTags[0].tagIndex = 0;
  setTags[0].tagType = eTagNone;
  setTags[0].scopeType = eTagRangeRectangle;
  setTags[0].ioIndex = 0;
  setTags[0].centerX = 0;
  setTags[0].centerY = 100;
  setTags[0].width = 120;
  setTags[0].height = 120;

  // Set tag 1, type is Boundary, range is Rectangle, center is (100, 220), width/height is (120, 120)
  setTags[1].tagIndex = 1;
  setTags[1].tagType = eTagBoundary;
  setTags[1].scopeType = eTagRangeRectangle;
  setTags[1].ioIndex = 0;
  setTags[1].centerX = 100;
  setTags[1].centerY = 220;
  setTags[1].width = 120;
  setTags[1].height = 120;

  // Set tag 2, type is ApproachAway, range is Circle, center is (-80, 350), radius is 80
  setTags[2].tagIndex = 2;
  setTags[2].tagType = eTagApproachAway;
  setTags[2].scopeType = eTagRangeCircle;
  setTags[2].ioIndex = 0;
  setTags[2].centerX = -80;
  setTags[2].centerY = 350;
  setTags[2].width = 80;
  setTags[2].height = 0;
  /**
   * Note: When the label type is a circle, width is the radius of the circle
   * and height is not used. When the range type is a rectangle, width and
   * height correspond to the rectangle dimensions respectively.
  */

  // Set tag 3, type is PeopleCounting, range is Rectangle, center is (0, 500), width/height is (160, 160)
  setTags[3].tagIndex = 3;
  setTags[3].tagType = eTagPeopleCounting;
  setTags[3].scopeType = eTagRangeRectangle;
  setTags[3].ioIndex = 0;
  setTags[3].centerX = 0;
  setTags[3].centerY = 500;
  setTags[3].width = 160;
  setTags[3].height = 160;

  // Set tag 4, type is Noise, range is Rectangle, center is (-100, 620), width/height is (100, 120)
  setTags[4].tagIndex = 4;
  setTags[4].tagType = eTagNoise;
  setTags[4].scopeType = eTagRangeRectangle;
  setTags[4].ioIndex = 0;
  setTags[4].centerX = -100;
  setTags[4].centerY = 620;
  setTags[4].width = 100;
  setTags[4].height = 120;

  const uint8_t setTagCount = (uint8_t)(sizeof(setTags) / sizeof(setTags[0]));
  if (c4004.setTagsFromConfig(setTags, setTagCount)) {
    Serial.println(F("Set 5 tags from config success."));
  } else {
    Serial.println(F("Set 5 tags from config failed."));
  }

  uint8_t count = c4004.getTags(setTags, setTagCount);
  printTagList(F("Active tag list after setup:"), setTags, count);
}

void loop()
{
  sTagInfo_t tagInfo;
  eReportedEvent_t event = c4004.getReportedInfo(100);
  /*
   * When state or data changes and the corresponding report function is enabled,
   * the module pushes the update immediately as an event via getReportedInfo().
   * Use the matching getter with eGetDataReport to read the cached value
   * updated by that report, without issuing an extra UART query.
   */

  if (event == eEventTag) {
    if (c4004.getTagInfo(&tagInfo)) {
      printTagEvent(tagInfo);
    }
  }
}
