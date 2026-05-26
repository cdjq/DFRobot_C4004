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
    Serial.println("DFRobot C4004 begin failed, retrying...");
    delay(1000);
  }
  Serial.println("DFRobot C4004 begin success.");

  if (c4004.setMotionLed(true)) {
    Serial.println("Set motion LED success.");
  } else {
    Serial.println("Set motion LED failed.");
  }

  if (c4004.setTrajectoryLed(true)) {
    Serial.println("Set trajectory LED success.");
  } else {
    Serial.println("Set trajectory LED failed.");
  }

  sBoundaryDetectionRange_t range;
  range.mode = eRangeFourSideBoundary;
  range.xPositiveCm = 300;
  range.xNegativeCm = -300;
  range.yPositiveCm = 500;
  range.yNegativeCm = 0;
  if (c4004.setBoundaryDetectionRange(range)) {
    Serial.println("Set boundary detection range success.");
  } else {
    Serial.println("Set boundary detection range failed.");
  }

  if (c4004.setPeopleReportInterval(2)) {
    Serial.println("Set people report interval success.");
  } else {
    Serial.println("Set people report interval failed.");
  }

  if (c4004.setPresenceEnable(true)) {
    Serial.println("Set presence enable success.");
  } else {
    Serial.println("Set presence enable failed.");
  }

  // Set to true if you want to clear the current people counter on boot.
  bool clearCountOnBoot = false;
  if (clearCountOnBoot) {
    if (c4004.clearPeopleCount()) {
      Serial.println("Clear people count success.");
    } else {
      Serial.println("Clear people count failed.");
    }
  }
}

void loop()
{
  eReportedEvent_t event = c4004.getReportedInfo(100);

  if (event == eEventPresence) {
    ePresenceState_t presence = c4004.getPresenceState();
    Serial.print("Target presence state: ");
    if (presence == eNoPresence) {
      Serial.println("None");
    } else if (presence == ePresence) {
      Serial.println("Presence");
    }
  } else if (event == eEventMotion) {
    uint8_t motion = c4004.getMotionState();
    Serial.print("Target motion state: ");
    if (motion == eMotionStatic) {
      Serial.println("Static");
    } else if (motion == eMotionActive) {
      Serial.println("Motion");
    } else {
      Serial.println("None");
    }
  } else if (event == eEventPeopleCount) {
    uint8_t count = c4004.getPeopleCountInfo(eGetDataReport);
    Serial.print("People count: ");
    Serial.println(count);
  }

  static uint32_t lastQuery = 0;
  if (millis() - lastQuery > 2000) {
    lastQuery = millis();
    Serial.print("Query people count: ");
    //Serial.println(c4004.getPeopleCountInfo(eGetDataActive)); // Query active data
    Serial.println(c4004.getPeopleCountInfo(eGetDataReport)); // Query report data

    ePresenceState_t queryPresence = c4004.getPresenceState();
    Serial.print("Query target presence state: ");
    if (queryPresence == eNoPresence) {
      Serial.println("None");
    } else if (queryPresence == ePresence) {
      Serial.println("Presence");
    }

    eMotionState_t queryMotion = c4004.getMotionState();
    Serial.print("Query target motion state: ");
    if (queryMotion == eMotionNone) {
      Serial.println("None");
    } else if (queryMotion == eMotionStatic) {
      Serial.println("Static");
    } else if (queryMotion == eMotionActive) {
      Serial.println("Motion");
    }
  }
}
