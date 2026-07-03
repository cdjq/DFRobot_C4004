/*!
 * @file test3.ino
 * @brief Test setTag() after single-target confirmation.
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

static const uint8_t TEST_TAG_INDEX = 1;
static const uint8_t SINGLE_PERSON_CONFIRM_TIMES = 5;
static const uint16_t PEOPLE_QUERY_INTERVAL_MS = 1000;

const __FlashStringHelper *tagSetStatusToString(eTagSetStatus_t status)
{
  if (status == eTagSetSuccess) {
    return F("Success");
  }
  if (status == eTagSetTrackCountError) {
    return F("TrackCountError");
  }
  if (status == eTagSetAlreadyUsed) {
    return F("AlreadyUsed");
  }
  if (status == eTagSetIndexOutOfRange) {
    return F("IndexOutOfRange");
  }
  return F("CommError");
}

void printTagList(const __FlashStringHelper *title, sTagConfig_t *tags, uint8_t count)
{
  char line[128];

  Serial.println(title);
  Serial.print(F("Tag count: "));
  Serial.println(count);
  if (count == 0) {
    Serial.println(F("No tag."));
    Serial.println();
    return;
  }

  Serial.println(F("Idx  Type             Range      IO  CenterX  CenterY  Width  Height"));
  Serial.println(F("---- ---------------- ---------- --  -------  -------  -----  ------"));
  for (uint8_t i = 0; i < count; i++) {
    const char *typeText = "Unknown";
    const char *rangeText = tags[i].scopeType == eTagRangeCircle ? "Circle" : "Rectangle";

    if (tags[i].tagType == eTagNone) {
      typeText = "None";
    } else if (tags[i].tagType == eTagBoundary) {
      typeText = "Boundary";
    } else if (tags[i].tagType == eTagApproachAway) {
      typeText = "ApproachAway";
    } else if (tags[i].tagType == eTagPeopleCounting) {
      typeText = "PeopleCounting";
    } else if (tags[i].tagType == eTagNoise) {
      typeText = "Noise";
    }

    snprintf(line, sizeof(line), "%3u  %-16s %-10s %2u  %7d  %7d  %5u  %6u",
             tags[i].tagIndex, typeText, rangeText, tags[i].ioIndex,
             tags[i].centerX, tags[i].centerY, tags[i].width, tags[i].height);
    Serial.println(line);
  }
  Serial.println();
}

bool waitForSingleTarget(void)
{
  uint8_t confirmCount = 0;

  Serial.println(F("Wait until active target count stays at 1 for 5 checks..."));

  while (confirmCount < SINGLE_PERSON_CONFIRM_TIMES) {
    uint32_t startTime = millis();
    uint8_t targetCount = c4004.getTargetList(NULL, 0, eGetDataActive);

    Serial.print(F("Active target count: "));
    Serial.print(targetCount);

    if (targetCount == 1) {
      confirmCount++;
    } else {
      confirmCount = 0;
    }

    Serial.print(F("  confirm: "));
    Serial.print(confirmCount);
    Serial.print(F("/"));
    Serial.println(SINGLE_PERSON_CONFIRM_TIMES);

    while ((uint32_t)(millis() - startTime) < PEOPLE_QUERY_INTERVAL_MS) {
      c4004.getReportedInfo(10);
    }
  }

  return true;
}

void setup()
{
  sTagConfig_t tag;
  sTagConfig_t tags[8];
  uint8_t tagCount = 0;

  Serial.begin(115200);

  while (!c4004.begin()) {
    Serial.println(F("DFRobot C4004 begin failed, retrying..."));
    delay(1000);
  }
  Serial.println(F("DFRobot C4004 begin success."));
  Serial.println(F("Test3: setTag"));
  Serial.println();

  if (c4004.setCheckToActiveFrames(7)) {
    Serial.println(F("Set check-to-active frames success."));
  } else {
    Serial.println(F("Set check-to-active frames failed."));
  }
  delay(50);

  sFourSidedRange_t range;
  range.mode = eRangeFourSide;
  range.xPositiveCm = 500;
  range.xNegativeCm = -500;
  range.yPositiveCm = 800;
  range.yNegativeCm = 0;
  if (c4004.setFourSidedRangeMode(range)) {
    Serial.println(F("Set four sided range success."));
  } else {
    Serial.println(F("Set four sided range failed."));
  }
  delay(50);

  if (c4004.setTrajectoryTrackEnable(true)) {
    Serial.println(F("Set trajectory track enable success."));
  } else {
    Serial.println(F("Set trajectory track enable failed."));
  }
  delay(50);

  if (c4004.clearAllTags()) {
    Serial.println(F("clearAllTags(): SUCCESS"));
  } else {
    Serial.println(F("clearAllTags(): FAILED"));
  }
  delay(100);

  if (!waitForSingleTarget()) {
    Serial.println(F("Single target confirmation failed."));
    return;
  }

  memset(&tag, 0, sizeof(tag));
  tag.tagIndex = TEST_TAG_INDEX;
  tag.tagType = eTagPeopleCounting;
  tag.scopeType = eTagRangeRectangle;
  tag.ioIndex = 0;
  tag.width = 120;
  tag.height = 120;

  Serial.print(F("setTag() tagIndex="));
  Serial.print(TEST_TAG_INDEX);
  Serial.print(F(" width="));
  Serial.print(tag.width);
  Serial.print(F(" height="));
  Serial.println(tag.height);

  eTagSetStatus_t status = c4004.setTag(tag);
  Serial.print(F("setTag() status: "));
  Serial.println(tagSetStatusToString(status));

  if (status != eTagSetSuccess) {
    Serial.println(F("setTag test stopped."));
    return;
  }

  delay(100);
  tagCount = c4004.getTags(tags, sizeof(tags) / sizeof(tags[0]), eGetDataActive);
  printTagList(F("----- Tag list after setTag -----"), tags, tagCount);
}

void loop()
{
}
