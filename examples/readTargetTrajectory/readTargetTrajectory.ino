/*!
 * @file readTargetTrajectory.ino
 * @brief Enable trajectory tracking and print target trajectory information.
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

const __FlashStringHelper *targetFeatureToString(eTargetFeature_t feature)
{
  if (feature == eStatic) {
    return F("Static");
  } else if (feature == eMotion) {
    return F("Motion");
  } else if (feature == eUncertain) {
    return F("Uncertain");
  } else {
    return F("Unknown");
  }
}

void printTrajectoryData(eGetDataMode_t dataMode)
{
  const __FlashStringHelper *title = NULL;
  const __FlashStringHelper *modeText = NULL;
  sTargetInfo_t targets[MAX_TARGETS];
  uint8_t count = c4004.getTargetList(targets, MAX_TARGETS, dataMode);

  if (dataMode == eGetDataActive) {
    title = F("======================TrajectoryActive=======================");
    modeText = F("Active Query");
  } else {
    title = F("======================TrajectoryReport=======================");
    modeText = F("Passive Report");
  }
  Serial.println(title);
  Serial.print(F("Mode: "));
  Serial.println(modeText);
  Serial.print(F("Target Count: "));
  Serial.println(count);
  if (count == 0) {
    Serial.println(F("No target."));
  } else {
    Serial.println(F("Row\tIndex\tKinesia\tFeature\tX\tY\tSpeed"));
    for (uint8_t i = 0; i < count; i++) {
      Serial.print(i);
      Serial.print(F("\t"));
      Serial.print(targets[i].index);
      Serial.print(F("\t"));
      Serial.print(targets[i].kinesia);
      Serial.print(F("\t"));
      Serial.print(targetFeatureToString(targets[i].targetFeature));
      Serial.print(F("\t"));
      Serial.print(targets[i].x);
      Serial.print(F("\t"));
      Serial.print(targets[i].y);
      Serial.print(F("\t"));
      Serial.println(targets[i].speed);
    }
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

  if (c4004.setTrajectoryTrackEnable(true)) {
    Serial.println(F("Set trajectory track enable success."));
  } else {
    Serial.println(F("Set trajectory track enable failed."));
  }

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
}

void loop()
{
  eReportedEvent_t event = c4004.getReportedInfo(100);
  /*
   * When state or data changes and the corresponding report function is enabled,
   * the module pushes the update immediately as an event via getReportedInfo().
   * Use the matching getter with eGetDataReport to read the cached value
   * updated by that report, without issuing an extra UART query.
   */

  // Passively obtain the trajectory
  if (event == eEventTrajectory) {
    printTrajectoryData(eGetDataReport);
  }

  // Actively obtain the trajectory
  static uint32_t lastQuery = 0;
  if (millis() - lastQuery > 4000) {
    lastQuery = millis();
    printTrajectoryData(eGetDataActive);
  }
}
