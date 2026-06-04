/*!
 * @file test1.ino
 * @brief DFRobot_C4004 test code.
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

const int16_t TARGET_Y_DISABLE_CM = 200;
bool trajectoryDisabled = false;

const char *targetFeatureToString(eTargetFeature_t feature)
{
  if (feature == eStatic) {
    return "Static";
  } else if (feature == eMotion) {
    return "Motion";
  } else if (feature == eUncertain) {
    return "Uncertain";
  }
  return "Unknown";
}

void printTrajectoryData(const sTargetInfo_t *targets, uint8_t count)
{
  Serial.println("===============Trajectory Data===============");
  Serial.print("Target Count: ");
  Serial.println(count);
  if (count == 0) {
    Serial.println("No target.");
    return;
  }

  Serial.println("Row\tID\tSize\tFeature\tX\tY\tSpeed");
  for (uint8_t i = 0; i < count; i++) {
    Serial.print(i);
    Serial.print("\t");
    Serial.print(targets[i].index);
    Serial.print("\t");
    Serial.print(targets[i].targetSize);
    Serial.print("\t");
    Serial.print(targetFeatureToString(targets[i].targetFeature));
    Serial.print("\t");
    Serial.print(targets[i].x);
    Serial.print("\t");
    Serial.print(targets[i].y);
    Serial.print("\t");
    Serial.println(targets[i].speed);
  }
}

void disableTrajectoryAndLeds(void)
{
  if (trajectoryDisabled) {
    return;
  }

  trajectoryDisabled = true;
  Serial.println("Target Y is greater than 2m, disable trajectory and LEDs.");

  if (c4004.setTrajectoryTrackEnable(false)) {
    Serial.println("Disable trajectory track success.");
  } else {
    Serial.println("Disable trajectory track failed.");
  }

  if (c4004.setTrajectoryLed(false)) {
    Serial.println("Disable trajectory LED success.");
  } else {
    Serial.println("Disable trajectory LED failed.");
  }

  if (c4004.setMotionLed(false)) {
    Serial.println("Disable motion LED success.");
  } else {
    Serial.println("Disable motion LED failed.");
  }
}

void setup()
{
  Serial.begin(115200);

  while (!c4004.begin()) {
    Serial.println("DFRobot C4004 begin failed, retrying...");
    delay(1000);
  }
  Serial.println("DFRobot C4004 begin success!");

  Serial.print("Current hardware version: ");
  Serial.println(c4004.getHardwareVersion());
  Serial.print("Current firmware version: ");
  Serial.println(c4004.getFirmwareVersion());

  sFourSidedRange range;
  range.mode = eRangeFourSide;
  range.xPositiveCm = 500;
  range.xNegativeCm = -500;
  range.yPositiveCm = 800;
  range.yNegativeCm = 0;
  if (c4004.setFourSidedRangeMode(range)) {
    Serial.println("Set boundary detection range success.");
  } else {
    Serial.println("Set boundary detection range failed.");
  }

  if (c4004.setTrajectoryLed(true)) {
    Serial.println("Set trajectory LED success.");
  } else {
    Serial.println("Set trajectory LED failed.");
  }

  if (c4004.setMotionLed(true)) {
    Serial.println("Set motion LED success.");
  } else {
    Serial.println("Set motion LED failed.");
  }

  if (c4004.setTrajectoryTrackEnable(true)) {
    Serial.println("Set trajectory track enable success.");
  } else {
    Serial.println("Set trajectory track enable failed.");
  }

}

void loop()
{
  c4004.getReportedInfo(50);

  static uint32_t lastQuery = 0;
  if ((uint32_t)(millis() - lastQuery) >= 1000) {
    lastQuery = millis();

    sTargetInfo_t targets[MAX_TARGETS];
    uint8_t count = c4004.getTargetList(targets, MAX_TARGETS, eGetDataActive);
    int16_t maxY = 0;
    bool hasTargetOver2m = false;

    printTrajectoryData(targets, count);

    for (uint8_t i = 0; i < count; i++) {
      if (i == 0 || targets[i].y > maxY) {
        maxY = targets[i].y;
      }
      if (targets[i].y > TARGET_Y_DISABLE_CM) {
        hasTargetOver2m = true;
      }
    }

    Serial.print("Max target Y(cm): ");
    Serial.println(count > 0 ? maxY : 0);

    if (hasTargetOver2m) {
      disableTrajectoryAndLeds();
    }
  }
}
