/*!
 * @file basicPresenceDetection.ino
 * @brief Quickly check whether someone is in range, still or moving, and how many people are counted.
 * @details Use this example to quickly verify that the sensor can detect whether someone is in the
 * @n detection range, whether they are static or moving, and how many people are counted.
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

  if (c4004.setFrameGenerateCount(7)) {
    Serial.println(F("Set check-to-active frames success."));
  } else {
    Serial.println(F("Set check-to-active frames failed."));
  }
  delay(50);

  if (c4004.setOccLED(true)) {
    Serial.println(F("Set occupancy LED success."));
  } else {
    Serial.println(F("Set occupancy LED failed."));
  }

  if (c4004.setTrkLED(true)) {
    Serial.println(F("Set trajectory LED success."));
  } else {
    Serial.println(F("Set trajectory LED failed."));
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

  if (c4004.setRealTimePeopleTime(2)) {
    Serial.println(F("Set RealTimePeopleTime success."));
  } else {
    Serial.println(F("Set RealTimePeopleTime failed."));
  }

  if (c4004.setPresenceEnable(true)) {
    Serial.println(F("Set presence enable success."));
  } else {
    Serial.println(F("Set presence enable failed."));
  }

  // Set to true if you want to clear the current people counter on boot.
  bool clearCountOnBoot = false;
  if (clearCountOnBoot) {
    if (c4004.clearPeopleCount()) {
      Serial.println(F("Clear people count success."));
    } else {
      Serial.println(F("Clear people count failed."));
    }
  }
}

void loop()
{
  DFRobot_C4004::eReportedEvent_t event = c4004.getReportedEvent(30);
  /*
   * timeoutMs=30: wait up to 30 ms for one UART report frame from the sensor.
   * Returns eEventNone if no complete frame arrives within that time.
   * When state or data changes and the corresponding report function is enabled,
   * the module pushes the update immediately as an event via getReportedEvent().
   * Use the matching getter with DFRobot_C4004::eGetDataReport to read the cached value
   * updated by that report, without issuing an extra UART query.
   */

  if (event == DFRobot_C4004::eEventPresence) {
    DFRobot_C4004::ePresenceState_t presence = c4004.getPresenceState(DFRobot_C4004::eGetDataReport);
    Serial.print(F("Presence state: "));
    if (presence == DFRobot_C4004::eNoPresence) {
      Serial.println(F("No Person Detected"));
    } else if (presence == DFRobot_C4004::ePresence) {
      Serial.println(F("Presence"));
    }
  } else if (event == DFRobot_C4004::eEventMotion) {
    DFRobot_C4004::eMotionState_t motion = c4004.getMotionState(DFRobot_C4004::eGetDataReport);
    Serial.print(F("Motion state: "));
    if (motion == DFRobot_C4004::eMotionStatic) {
      Serial.println(F("Static"));
    } else if (motion == DFRobot_C4004::eMotionActive) {
      Serial.println(F("Motion"));
    } else {
      Serial.println(F("No Target Detected"));
    }
  } else if (event == DFRobot_C4004::eEventPeopleCount) {
    uint8_t count = c4004.getPeopleCount(DFRobot_C4004::eGetDataReport);
    Serial.print(F("People count: "));
    Serial.println(count);
  }

  static uint32_t lastQuery = 0;
  // Every 3000 ms, actively poll and print people count / presence / motion (not event-driven).
  if (millis() - lastQuery > 3000) {
    lastQuery = millis();
    Serial.print(F("People count: "));
    //Serial.println(c4004.getPeopleCount(DFRobot_C4004::eGetDataActive)); // Query active data
    Serial.println(c4004.getPeopleCount(DFRobot_C4004::eGetDataReport));    // Query report data

    DFRobot_C4004::ePresenceState_t queryPresence = c4004.getPresenceState(DFRobot_C4004::eGetDataActive);
    Serial.print(F("Presence state: "));
    if (queryPresence == DFRobot_C4004::eNoPresence) {
      Serial.println(F("No Person Detected"));
    } else if (queryPresence == DFRobot_C4004::ePresence) {
      Serial.println(F("Presence"));
    }

    DFRobot_C4004::eMotionState_t queryMotion = c4004.getMotionState(DFRobot_C4004::eGetDataActive);
    Serial.print(F("Motion state: "));
    if (queryMotion == DFRobot_C4004::eMotionNone) {
      Serial.println(F("No Target Detected"));
    } else if (queryMotion == DFRobot_C4004::eMotionStatic) {
      Serial.println(F("Static"));
    } else if (queryMotion == DFRobot_C4004::eMotionActive) {
      Serial.println(F("Motion"));
    }
  }
}
