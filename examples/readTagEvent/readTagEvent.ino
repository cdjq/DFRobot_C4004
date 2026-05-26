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

const char *tagTypeToString(eTagType_t type)
{
  if (type == eTagTypeNone) {
    return "None";
  }
  if (type == eTagTypeEnterExit) {
    return "EnterExit";
  }
  if (type == eTagTypeApproachAway) {
    return "ApproachAway";
  }
  if (type == eTagTypePeopleCounting) {
    return "PeopleCounting";
  }
  if (type == eTagTypeNoise) {
    return "Noise";
  }
  return "Unknown";
}

void printTagEvent(const sTagInfo_t &info)
{
  char line[96];
  Serial.println("======================================================================");
  Serial.println("============================TagEventReport============================");
  Serial.println("======================================================================");
  snprintf(line, sizeof(line), "Tag Index : %u", info.index);
  Serial.println(line);
  snprintf(line, sizeof(line), "Center XY : %d / %d", info.centerX, info.centerY);
  Serial.println(line);

  if (info.type == eTagTypeEnterExit) {
    snprintf(line, sizeof(line), "Event     : EnterExit (%s)", info.enterExit == 0 ? "Enter" : "Exit");
    Serial.println(line);
  } else if (info.type == eTagTypeApproachAway) {
    snprintf(line, sizeof(line), "Event     : MotionDirection (%s)", info.motionDir == 0 ? "Approach" : "Away");
    Serial.println(line);
  } else if (info.type == eTagTypePeopleCounting) {
    snprintf(line, sizeof(line), "Event     : PeopleCounting (M:%u S:%u)", info.motionNum, info.staticNum);
    Serial.println(line);
  } else {
    snprintf(line, sizeof(line), "Event     : %s", tagTypeToString(info.type));
    Serial.println(line);
  }
  Serial.println();
}

void printTagList(const char *title, sTagConfig_t *tags, uint8_t count)
{
  Serial.println("======================================================================");
  Serial.println(title);
  Serial.println("----------------------------------------------------------------------");
  Serial.print("Tag count: ");
  Serial.println(count);
  if (count == 0) {
    Serial.println("No tag.");
    Serial.println();
    return;
  }

  Serial.println("Idx\tType\t\tRange\t\tCenterX\tCenterY\tWidth\tHeight");
  Serial.println("----------------------------------------------------------------------");
  for (uint8_t i = 0; i < count; i++) {
    const char *typeText = tagTypeToString(tags[i].type);
    const char *rangeText = tags[i].rangeType == eTagRangeCircle ? "Circle" : "Rectangle";
    Serial.print(tags[i].index);
    Serial.print("\t");
    Serial.print(typeText);
    if (strlen(typeText) <= 8) {
      Serial.print("\t\t");
    } else {
      Serial.print("\t");
    }
    Serial.print(rangeText);
    if (strlen(rangeText) <= 8) {
      Serial.print("\t\t");
    } else {
      Serial.print("\t");
    }
    Serial.print(tags[i].centerX);
    Serial.print("\t");
    Serial.print(tags[i].centerY);
    Serial.print("\t");
    Serial.print(tags[i].xSize);
    Serial.print("\t");
    Serial.println(tags[i].ySize);
  }
  Serial.println();
}

void setup()
{
  Serial.begin(115200);

  while (!c4004.begin()) {
    Serial.println("DFRobot C4004 begin failed, retrying...");
    delay(1000);
  }

  sBoundaryDetectionRange_t range;
  range.mode = eRangeFourSideBoundary;
  range.xPositiveCm = 400;
  range.xNegativeCm = -400;
  range.yPositiveCm = 600;
  range.yNegativeCm = 0;
  if (c4004.setBoundaryDetectionRange(range)) {
    Serial.println("Set boundary detection range success.");
  } else {
    Serial.println("Set boundary detection range failed.");
  }

  if (c4004.clearAllTags()) {
    Serial.println("Clear all tags success.");
  } else {
    Serial.println("Clear all tags failed.");
  }

  sTagConfig_t setTags[5];

  // Set tag 0，type is None, range is Rectangle, center is (0, 80), size is (180, 120)
  setTags[0].index = 0;
  setTags[0].type = eTagTypeNone;
  setTags[0].rangeType = eTagRangeRectangle;
  setTags[0].centerX = 0;
  setTags[0].centerY = 80;
  setTags[0].xSize = 180;
  setTags[0].ySize = 120;

  // Set tag 1，type is EnterExit, range is Rectangle, center is (180, 160), size is (200, 140)
  setTags[1].index = 1;
  setTags[1].type = eTagTypeEnterExit;
  setTags[1].rangeType = eTagRangeRectangle;
  setTags[1].centerX = 180;
  setTags[1].centerY = 160;
  setTags[1].xSize = 200;
  setTags[1].ySize = 140;

  // Set tag 2，type is ApproachAway, range is Circle, center is (-180, 240), size is (160, 160)
  setTags[2].index = 2;
  setTags[2].type = eTagTypeApproachAway;
  setTags[2].rangeType = eTagRangeCircle;
  setTags[2].centerX = -180;
  setTags[2].centerY = 240;
  setTags[2].xSize = 160;
  setTags[2].ySize = 160;
  /**
   * Note: When the label type is a circle, X is the radius of the circle 
   * and Y is not used. When the range type is a rectangle, X and Y 
   * correspond to the width and height of the rectangle respectively
  */

  // Set tag 3，type is PeopleCounting, range is Rectangle, center is (80, 260), size is (220, 150)
  setTags[3].index = 3;
  setTags[3].type = eTagTypePeopleCounting;
  setTags[3].rangeType = eTagRangeRectangle;
  setTags[3].centerX = 80;
  setTags[3].centerY = 260;
  setTags[3].xSize = 220;
  setTags[3].ySize = 150;

  // Set tag 4，type is Noise, range is Rectangle, center is (-220, 360), size is (260, 180)
  setTags[4].index = 4;
  setTags[4].type = eTagTypeNoise;
  setTags[4].rangeType = eTagRangeRectangle;
  setTags[4].centerX = -220;
  setTags[4].centerY = 360;
  setTags[4].xSize = 260;
  setTags[4].ySize = 180;

  if (c4004.setTagsFromConfig(setTags, 5)) {
    Serial.println("Set 5 tags from config success.");
  } else {
    Serial.println("Set 5 tags from config failed.");
  }

  sTagConfig_t tags[MAX_TAGS];
  uint8_t count = c4004.getTags(tags, MAX_TAGS);
  printTagList("Active tag list after setup:", tags, count);
}

void loop()
{
  sTagInfo_t tagInfo;
  eReportedEvent_t event = c4004.getReportedInfo(100);

  if (event == eEventTag) {
    if (c4004.getTagInfo(&tagInfo)) {
      printTagEvent(tagInfo);
    }
  }
}

