/*!
 * @file setAllParam.ino
 * @brief Configure and read back major DFRobot C4004 parameters.
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
  Serial.println(F("===================Product Info==================="));

  Serial.print(F("Current product model: "));
  Serial.println(c4004.getProductModel());
  Serial.print(F("Current hardware version: "));
  Serial.println(c4004.getHardwareVersion());
  Serial.print(F("Current firmware version: "));
  Serial.println(c4004.getFirmwareVersion());

  Serial.println(F("=================Set install info================="));
  if (c4004.setInstallHigh(180)) {
      Serial.println(F("Set install high success!"));
  } else {
    Serial.println(F("Set install high failed!"));
  }
  delay(50);

  int deviceHigh = 0;
  if (c4004.getInstallHigh(&deviceHigh)) {
    Serial.print(F("Current install high(cm): "));
    Serial.println(deviceHigh);
  } else {
    Serial.println(F("Read current install high failed."));
  }

  Serial.println(F("==================Feature Switch=================="));
  if (c4004.setPresenceEnable(true)) {
    Serial.println(F("Set presence enable success!"));
  } else {
    Serial.println(F("Set presence enable failed!"));
  }
  delay(50);

  bool presenceEnable = false;
  if (c4004.getPresenceEnable(&presenceEnable)) {
    Serial.print(F("Current presence enable: "));
    Serial.println(presenceEnable ? F("ON") : F("OFF"));
  } else {
    Serial.println(F("Read current presence enable failed."));
  }

  if (c4004.setTrajectoryTrackEnable(true)) {
    Serial.println(F("Set trajectory track enable success!"));
  } else {
    Serial.println(F("Set trajectory track enable failed!"));
  }
  delay(50);

  bool trackEnable = false;
  if (c4004.getTrajectoryTrackEnable(&trackEnable)) {
    Serial.print(F("Current trajectory tracking function enable: "));
    Serial.println(trackEnable ? F("ON") : F("OFF"));
  } else {
    Serial.println(F("Read current trajectory tracking function enable failed."));
  }

  if (c4004.setCheckToActiveFrames(3)) {
    Serial.println(F("Set check-to-active frames success!"));
  } else {
    Serial.println(F("Set check-to-active frames failed!"));
  }
  delay(50);

  uint8_t checkToActiveFrames = 0;
  if (c4004.getCheckToActiveFrames(&checkToActiveFrames)) {
    Serial.print(F("Current check-to-active frames: "));
    Serial.println(checkToActiveFrames);
  } else {
    Serial.println(F("Read current check-to-active frames failed."));
  }

  if (c4004.setMotionLed(true)) {
    Serial.println(F("Set motion LED success!"));
  } else {
    Serial.println(F("Set motion LED failed!"));
  }
  delay(50);

  if (c4004.setTrajectoryLed(true)) {
    Serial.println(F("Set trajectory LED success!"));
  } else {
    Serial.println(F("Set trajectory LED failed!"));
  }
  delay(50);

  Serial.print(F("Current motion LED: "));
  Serial.println(c4004.getMotionLed() ? F("ON") : F("OFF"));
  Serial.print(F("Current trajectory LED: "));
  Serial.println(c4004.getTrajectoryLed() ? F("ON") : F("OFF"));

  Serial.println(F("====================Range Param==================="));
  sFourSidedRange range;
  range.mode = eRangeFourSide;
  range.xPositiveCm = 200;
  range.xNegativeCm = -200;
  range.yPositiveCm = 700;
  range.yNegativeCm = 0;

  // Set the boundary detection range
  if (c4004.setFourSidedRangeMode(range)) {
    Serial.println(F("Set four sided range success!"));
  } else {
    Serial.println(F("Set four sided range failed!"));
  }
  delay(50);

  eDetectionRangeMode_t mode = c4004.getDetectionRangeMode();
  Serial.print(F("Current detection mode: "));
  if (mode == eRangeFourSide) {
    Serial.println(F("Four-side boundary"));
  } else if (mode == eRangeTrajectory) {
    Serial.println(F("Trajectory"));
  } else {
    Serial.println(F("Other"));
  }

  if (mode == eRangeFourSide) {
    sFourSidedRange currentRange;
    if (c4004.getFourSidedRangeMode(&currentRange)) {
      Serial.print(F("Current boundary x+/x-/y+/y- (cm): "));
      Serial.print(currentRange.xPositiveCm);
      Serial.print(F("/"));
      Serial.print(currentRange.xNegativeCm);
      Serial.print(F("/"));
      Serial.print(currentRange.yPositiveCm);
      Serial.print(F("/"));
      Serial.println(currentRange.yNegativeCm);
    } else {
      Serial.println(F("Read current boundary range failed."));
    }
  } else {
    Serial.println(F("Current mode is not four-side boundary, skip boundary range check."));
  }

  // Set the trajectory detection range mode
  // if (c4004.setTrajectoryRangeMode(false)) { // Setting it to false means using this mode and not performing trajectory learning
  //   Serial.println(F("Set trajectory detection range mode success!"));
  // } else {
  //   Serial.println(F("Set trajectory detection range mode failed!"));
  // }
  // delay(50);

  // sPoint_t points[MAX_POINTS];
  // uint16_t pointCount = 0;
  // if (c4004.getTrajectoryRangeMode(points, &pointCount)) {
  //   Serial.println(F("Current trajectory range query success."));
  //   Serial.print(F("Current trajectory points: "));
  //   Serial.println(pointCount);
  //   for (uint16_t i = 0; i < pointCount; i++) {
  //     Serial.print(F("#"));
  //     Serial.print(i);
  //     Serial.print(F(" x/y="));
  //     Serial.print(points[i].x);
  //     Serial.print(F("/"));
  //     Serial.println(points[i].y);
  //   }
  // } else {
  //   Serial.println(F("Current trajectory range query failed."));
  // }

  // Set multi-point config by config-file mode points
  // sPoint_t cfgPoints[4];
  // cfgPoints[0].x = 200;  cfgPoints[0].y = 0;
  // cfgPoints[1].x = 200;  cfgPoints[1].y = 400;
  // cfgPoints[2].x = -200; cfgPoints[2].y = 400;
  // cfgPoints[3].x = -200; cfgPoints[3].y = 0;
  // if (c4004.setConfigFileModePoints(cfgPoints, 4)) {
  //   Serial.println(F("Set multi-point config points success!"));
  // } else {
  //   Serial.println(F("Set multi-point config points failed!"));
  // }
  // delay(50);

  // sPoint_t points[MAX_POINTS];
  // uint16_t pointCount = 0;
  // if (c4004.getConfigFileModePoints(points, &pointCount)) {
  //   Serial.println(F("Current multi-point config query success."));
  //   Serial.print(F("Current multi-point config points: "));
  //   Serial.println(pointCount);
  //   for (uint16_t i = 0; i < pointCount; i++) {
  //     Serial.print(F("#"));
  //     Serial.print(i);
  //     Serial.print(F(" x/y="));
  //     Serial.print(points[i].x);
  //     Serial.print(F("/"));
  //     Serial.println(points[i].y);
  //   }
  // } else {
  //   Serial.println(F("Current multi-point config query failed."));
  // }

  Serial.println(F("================People Count Param================"));
  if (c4004.setRealTimePeopleTime(5)) {
    Serial.println(F("Set RealTimePeopleTime success!"));
  } else {
    Serial.println(F("Set RealTimePeopleTime failed!"));
  }
  delay(50);

  if (c4004.setTrackMeters(50)) {
    Serial.println(F("Set TrackMeters success!"));
  } else {
    Serial.println(F("Set TrackMeters failed!"));
  }
  delay(50);

  if (c4004.setTrackExistsTime(10)) {
    Serial.println(F("Set TrackExistsTime success!"));
  } else {
    Serial.println(F("Set TrackExistsTime failed!"));
  }
  delay(50);

  if (c4004.setUnmannedTime(30)) {
    Serial.println(F("Set UnmannedTime success!"));
  } else {
    Serial.println(F("Set UnmannedTime failed!"));
  }
  delay(50);

  if (c4004.clearPeopleCount()) {
    Serial.println(F("Clear people count success!"));
  } else {
    Serial.println(F("Clear people count failed!"));
  }
  delay(50);

  Serial.print(F("Current RealTimePeopleTime(s): "));
  uint32_t peopleInterval = 0;
  if (c4004.getRealTimePeopleTime(&peopleInterval)) {
    Serial.println(peopleInterval);
  } else {
    Serial.println(F("Read current RealTimePeopleTime failed."));
  }
  Serial.print(F("Current TrackMeters(cm): "));
  uint32_t trajectoryDistance = 0;
  if (c4004.getTrackMeters(&trajectoryDistance)) {
    Serial.println(trajectoryDistance);
  } else {
    Serial.println(F("Read current TrackMeters failed."));
  }
  Serial.print(F("Current TrackExistsTime(s): "));
  uint32_t time = 0;
  if (c4004.getTrackExistsTime(&time)) {
    Serial.println(time);
  } else {
    Serial.println(F("Read current TrackExistsTime failed."));
  }
  Serial.print(F("Current UnmannedTime(s): "));
  uint32_t noPersonDelay = 0;
  if (c4004.getUnmannedTime(&noPersonDelay)) {
    Serial.println(noPersonDelay);
  } else {
    Serial.println(F("Read current UnmannedTime failed."));
  }

  Serial.print(F("Current people count(active): "));
  Serial.println(c4004.getPeopleTime(eGetDataActive));

  Serial.println(F("=======================Done======================="));
}

void loop()
{
  delay(1000);
}
