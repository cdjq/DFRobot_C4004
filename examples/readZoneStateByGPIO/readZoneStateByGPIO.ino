/*!
 * @file readZoneStateByGPIO.ino
 * @brief Configure tag zones and read local GPIO presence states.
 * @details This demo reads user-selected GPIO pins and prints zone presence every 1 second.
 * @n GPIO 1 is the whole area output. GPIO 2-6 are divided zone outputs.
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

const uint8_t zonePins[6] = {6, 7, 8, 9, 10, 11};

void setup()
{
  Serial.begin(115200);

  for (uint8_t i = 0; i < 6; i++) {
    pinMode(zonePins[i], INPUT);
  }

  while (!c4004.begin()) {
    Serial.println(F("DFRobot C4004 begin failed, retrying..."));
    delay(1000);
  }

  if (c4004.setCheckToActiveFrames(7)) {
    Serial.println(F("Set check-to-active frames success."));
  } else {
    Serial.println(F("Set check-to-active frames failed."));
  }
  delay(50);

  sFourSidedRange range;
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

  sTagConfig_t setTags[5];

  // Set tag 0, bind to IO2, type is PeopleCounting, range is Rectangle, center is (0, 100), width/height is (120, 120)
  setTags[0].tagIndex = 0;
  setTags[0].tagType = eTagPeopleCounting;
  setTags[0].scopeType = eTagRangeRectangle;
  setTags[0].ioIndex = 2;
  setTags[0].centerX = 0;
  setTags[0].centerY = 100;
  setTags[0].width = 120;
  setTags[0].height = 120;

  // Set tag 1, bind to IO3, type is PeopleCounting, range is Rectangle, center is (100, 220), width/height is (120, 120)
  setTags[1].tagIndex = 1;
  setTags[1].tagType = eTagPeopleCounting;
  setTags[1].scopeType = eTagRangeRectangle;
  setTags[1].ioIndex = 3;
  setTags[1].centerX = 100;
  setTags[1].centerY = 220;
  setTags[1].width = 120;
  setTags[1].height = 120;

  // Set tag 2, bind to IO4, type is PeopleCounting, range is Circle, center is (-80, 350), radius is 80
  setTags[2].tagIndex = 2;
  setTags[2].tagType = eTagPeopleCounting;
  setTags[2].scopeType = eTagRangeCircle;
  setTags[2].ioIndex = 4;
  setTags[2].centerX = -80;
  setTags[2].centerY = 350;
  setTags[2].width = 80;
  setTags[2].height = 0;
  /**
   * Note: When the label type is a circle, width is the radius of the circle
   * and height is not used. When the range type is a rectangle, width and
   * height correspond to the rectangle dimensions respectively.
  */

  // Set tag 3, bind to IO5, type is PeopleCounting, range is Rectangle, center is (0, 500), width/height is (160, 160)
  setTags[3].tagIndex = 3;
  setTags[3].tagType = eTagPeopleCounting;
  setTags[3].scopeType = eTagRangeRectangle;
  setTags[3].ioIndex = 5;
  setTags[3].centerX = 0;
  setTags[3].centerY = 500;
  setTags[3].width = 160;
  setTags[3].height = 160;

  // Set tag 4, bind to IO6, type is PeopleCounting, range is Rectangle, center is (-100, 620), width/height is (100, 120)
  setTags[4].tagIndex = 4;
  setTags[4].tagType = eTagPeopleCounting;
  setTags[4].scopeType = eTagRangeRectangle;
  setTags[4].ioIndex = 6;
  setTags[4].centerX = -100;
  setTags[4].centerY = 620;
  setTags[4].width = 100;
  setTags[4].height = 120;

  if (c4004.setTagsFromConfig(setTags, 5)) {
    Serial.println(F("Set 5 tags from config success."));
  } else {
    Serial.println(F("Set 5 tags from config failed."));
  }

  if (c4004.setMotionLed(true)) {
    Serial.println(F("Set motion LED success."));
  } else {
    Serial.println(F("Set motion LED failed."));
  }

  if (c4004.setRealTimePeopleTime(5)) {
    Serial.println(F("Set RealTimePeopleTime success."));
  } else {
    Serial.println(F("Set RealTimePeopleTime failed."));
  }
}

void loop()
{
  c4004.getReportedInfo(50);

  static uint32_t lastPrint = 0;
  if (millis() - lastPrint > 1000) {
    lastPrint = millis();
    Serial.println(F("============================================="));
    Serial.println(F("GPIO presence (HIGH=Presence, LOW=None):"));
    Serial.println(F("GPIO 1 = Whole area, GPIO 2-6 = Divided zones"));
    for (uint8_t i = 0; i < 6; i++) {
      bool hasPresence = (digitalRead(zonePins[i]) == HIGH);
      Serial.print(F("GPIO "));
      Serial.print(i + 1);
      if (i == 0) {
        Serial.print(F(" (Whole area): "));
      } else {
        Serial.print(F(" (Zone "));
        Serial.print(i);
        Serial.print(F("): "));
      }
      Serial.println(hasPresence ? F("Presence") : F("None"));
    }
    Serial.println(F("============================================="));
  }
}
