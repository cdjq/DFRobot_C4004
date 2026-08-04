/*!
 * @file readZoneStateByGPIO.ino
 * @brief Configure tag zones and read local GPIO presence states.
 * @details This demo reads user-selected GPIO pins and prints zone presence every 1 second.
 * @n GPIO 1 is the whole area output. GPIO 2-6 are divided zone outputs.
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

/* ---------------------------------------------------------------------------------------------------------------------
 *  C4004 IO | MCU digital pin | Description
 *    IO1    |        6        | Whole detection area presence output
 *    IO2    |        7        | Zone tag0 presence output
 *    IO3    |        8        | Zone tag1 presence output
 *    IO4    |        9        | Zone tag2 presence output
 *    IO5    |       10        | Zone tag3 presence output
 *    IO6    |       11        | Zone tag4 presence output
 * ----------------------------------------------------------------------------------------------------------------------*/
/* GPIO level: HIGH = presence, LOW = none. */

#if defined(ESP8266) || defined(ARDUINO_AVR_UNO)
SoftwareSerial mySerial(4, 5);
DFRobot_C4004  c4004(&mySerial, 115200);
#elif defined(ESP32)
DFRobot_C4004 c4004(&Serial1, 115200, /*D2*/ D2, /*D3*/ D3);
#else
DFRobot_C4004 c4004(&Serial1, 115200);
#endif

const uint8_t zonePins[6] = { 6, 7, 8, 9, 10, 11 };

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

  DFRobot_C4004::sTagConfig_t setTags[5];

  // Set tag 0, bind to IO2, type is PeopleCounting, range is Rectangle, center is (0, 100), width/height is (120, 120)
  setTags[0].tagIndex  = 0;
  setTags[0].tagType   = DFRobot_C4004::eTagPeopleCounting;
  setTags[0].scopeType = DFRobot_C4004::eRectangle;
  setTags[0].ioIndex   = 2;
  setTags[0].centerX   = 0;
  setTags[0].centerY   = 100;
  setTags[0].width     = 120;
  setTags[0].height    = 120;

  // Set tag 1, bind to IO3, type is PeopleCounting, range is Rectangle, center is (100, 220), width/height is (120, 120)
  setTags[1].tagIndex  = 1;
  setTags[1].tagType   = DFRobot_C4004::eTagPeopleCounting;
  setTags[1].scopeType = DFRobot_C4004::eRectangle;
  setTags[1].ioIndex   = 3;
  setTags[1].centerX   = 100;
  setTags[1].centerY   = 220;
  setTags[1].width     = 120;
  setTags[1].height    = 120;

  // Set tag 2, bind to IO4, type is PeopleCounting, range is Circle, center is (-80, 350), radius is 80
  setTags[2].tagIndex  = 2;
  setTags[2].tagType   = DFRobot_C4004::eTagPeopleCounting;
  setTags[2].scopeType = DFRobot_C4004::eCircle;
  setTags[2].ioIndex   = 4;
  setTags[2].centerX   = -80;
  setTags[2].centerY   = 350;
  setTags[2].width     = 80;
  setTags[2].height    = 0;
  /**
   * Note: For rectangle tags, width is the size along the X-axis and height is
   * the size along the Y-axis (unit: cm), relative to centerX/centerY.
   * For circle tags, width is the radius and height is ignored.
  */

  // Set tag 3, bind to IO5, type is PeopleCounting, range is Rectangle, center is (0, 500), width/height is (160, 160)
  setTags[3].tagIndex  = 3;
  setTags[3].tagType   = DFRobot_C4004::eTagPeopleCounting;
  setTags[3].scopeType = DFRobot_C4004::eRectangle;
  setTags[3].ioIndex   = 5;
  setTags[3].centerX   = 0;
  setTags[3].centerY   = 500;
  setTags[3].width     = 160;
  setTags[3].height    = 160;

  // Set tag 4, bind to IO6, type is PeopleCounting, range is Rectangle, center is (-100, 620), width/height is (100, 120)
  setTags[4].tagIndex  = 4;
  setTags[4].tagType   = DFRobot_C4004::eTagPeopleCounting;
  setTags[4].scopeType = DFRobot_C4004::eRectangle;
  setTags[4].ioIndex   = 6;
  setTags[4].centerX   = -100;
  setTags[4].centerY   = 620;
  setTags[4].width     = 100;
  setTags[4].height    = 120;

  if (c4004.setTagsFromConfig(setTags, 5)) {
    Serial.println(F("Set 5 tags from config success."));
  } else {
    Serial.println(F("Set 5 tags from config failed."));
  }

  if (c4004.setOccLED(true)) {
    Serial.println(F("Set occupancy LED success."));
  } else {
    Serial.println(F("Set occupancy LED failed."));
  }

  if (c4004.setRealTimeReportInterval(5)) {
    Serial.println(F("Set RealTimeReportInterval success."));
  } else {
    Serial.println(F("Set RealTimeReportInterval failed."));
  }
}

void loop()
{
  /*
   * When state or data changes and the corresponding report function is enabled,
   * the module pushes the update immediately as an event via getReportedEvent().
   * Use the matching getter with DFRobot_C4004::eGetDataReport to read the cached value
   * updated by that report, without issuing an extra UART query.
   */
  c4004.getReportedEvent(50);

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
