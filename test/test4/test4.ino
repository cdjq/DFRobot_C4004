/*!
 * @file test4.ino
 * @brief Test clearTag() for the tag index written by test3.ino.
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

bool hasTagIndex(sTagConfig_t *tags, uint8_t count, uint8_t tagIndex)
{
  for (uint8_t i = 0; i < count; i++) {
    if (tags[i].tagIndex == tagIndex) {
      return true;
    }
  }
  return false;
}

void setup()
{
  sTagConfig_t tags[8];
  uint8_t tagCount = 0;

  Serial.begin(115200);

  while (!c4004.begin()) {
    Serial.println(F("DFRobot C4004 begin failed, retrying..."));
    delay(1000);
  }
  Serial.println(F("DFRobot C4004 begin success."));
  Serial.println(F("Test4: clearTag"));
  Serial.print(F("Target tag index: "));
  Serial.println(TEST_TAG_INDEX);
  Serial.println();

  tagCount = c4004.getTags(tags, sizeof(tags) / sizeof(tags[0]), eGetDataActive);
  printTagList(F("----- Tag list before clearTag -----"), tags, tagCount);

  if (!hasTagIndex(tags, tagCount, TEST_TAG_INDEX)) {
    Serial.println(F("Warning: target tag not found. Run test3.ino first."));
  }

  Serial.print(F("clearTag("));
  Serial.print(TEST_TAG_INDEX);
  Serial.print(F("): "));
  Serial.println(c4004.clearTag(TEST_TAG_INDEX) ? F("SUCCESS") : F("FAILED"));
  delay(100);

  tagCount = c4004.getTags(tags, sizeof(tags) / sizeof(tags[0]), eGetDataActive);
  printTagList(F("----- Tag list after clearTag -----"), tags, tagCount);

  if (!hasTagIndex(tags, tagCount, TEST_TAG_INDEX)) {
    Serial.println(F("clearTag verify: PASS"));
  } else {
    Serial.println(F("clearTag verify: FAIL"));
  }
}

void loop()
{
}
