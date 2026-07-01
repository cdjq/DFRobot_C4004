/*!
 * @file basicPresenceDetection.ino
 * @brief Enable presence detection and print presence, motion and people-count reports.
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

void setup()
{
  Serial.begin(115200);

  while (!c4004.begin()) {
    Serial.println(F("DFRobot C4004 begin failed, retrying..."));
    delay(1000);
  }
  Serial.println(F("DFRobot C4004 begin success."));

  if (c4004.setCheckToActiveFrames(7)) {
    Serial.println(F("Set check-to-active frames success."));
  } else {
    Serial.println(F("Set check-to-active frames failed."));
  }
  delay(50);

  if (c4004.setMotionLed(true)) {
    Serial.println(F("Set motion LED success."));
  } else {
    Serial.println(F("Set motion LED failed."));
  }

  if (c4004.setTrajectoryLed(true)) {
    Serial.println(F("Set trajectory LED success."));
  } else {
    Serial.println(F("Set trajectory LED failed."));
  }

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
  eReportedEvent_t event = c4004.getReportedInfo(30);
  /*
   * When state or data changes and the corresponding report function is enabled,
   * the module pushes the update immediately as an event via getReportedInfo().
   * Use the matching getter with eGetDataReport to read the cached value
   * updated by that report, without issuing an extra UART query.
   */

  if (event == eEventPresence) {
    ePresenceState_t presence = c4004.getPresenceState(eGetDataReport);
    Serial.print(F("Presence state: "));
    if (presence == eNoPresence) {
      Serial.println(F("None"));
    } else if (presence == ePresence) {
      Serial.println(F("Presence"));
    }
  } else if (event == eEventMotion) {
    eMotionState_t motion = c4004.getMotionState(eGetDataReport);
    Serial.print(F("Motion state: "));
    if (motion == eMotionStatic) {
      Serial.println(F("Static"));
    } else if (motion == eMotionActive) {
      Serial.println(F("Motion"));
    } else {
      Serial.println(F("None"));
    }
  } else if (event == eEventPeopleCount) {
    uint8_t count = c4004.getPeopleTime(eGetDataReport);
    Serial.print(F("People count: "));
    Serial.println(count);
  }

  static uint32_t lastQuery = 0;
  if (millis() - lastQuery > 3000) {
    lastQuery = millis();
    Serial.print(F("People count: "));
    //Serial.println(c4004.getPeopleTime(eGetDataActive)); // Query active data
    Serial.println(c4004.getPeopleTime(eGetDataReport)); // Query report data

    ePresenceState_t queryPresence = c4004.getPresenceState(eGetDataActive);
    Serial.print(F("Presence state: "));
    if (queryPresence == eNoPresence) {
      Serial.println(F("None"));
    } else if (queryPresence == ePresence) {
      Serial.println(F("Presence"));
    }

    eMotionState_t queryMotion = c4004.getMotionState(eGetDataActive);
    Serial.print(F("Motion state: "));
    if (queryMotion == eMotionNone) {
      Serial.println(F("None"));
    } else if (queryMotion == eMotionStatic) {
      Serial.println(F("Static"));
    } else if (queryMotion == eMotionActive) {
      Serial.println(F("Motion"));
    }
  }
}
