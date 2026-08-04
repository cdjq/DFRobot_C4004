/*!
 * @file setAllParam.ino
 * @brief Configure and read back major DFRobot C4004 parameters.
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
  Serial.println(F("===================Product Info==================="));

  Serial.print(F("Current hardware version: "));
  Serial.println(c4004.getHardwareVersion());
  Serial.print(F("Current firmware version: "));
  Serial.println(c4004.getFirmwareVersion());

  Serial.println(F("=================Set install info================="));
  // Side mount: default 180 cm, recommended 180±20 cm. Top mount: recommended 220-280 cm.
  if (c4004.setInstallHeight(180)) {
    Serial.println(F("Set install height success!"));
  } else {
    Serial.println(F("Set install height failed!"));
  }
  delay(50);

  int deviceHeight = 0;
  if (c4004.getInstallHeight(&deviceHeight)) {
    Serial.print(F("Current install height(cm): "));
    Serial.println(deviceHeight);
  } else {
    Serial.println(F("Read current install height failed."));
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

  if (c4004.setFrameGenerateCount(7)) {
    Serial.println(F("Set check-to-active frames success!"));
  } else {
    Serial.println(F("Set check-to-active frames failed!"));
  }
  delay(50);

  uint8_t checkToActiveFrames = 0;
  if (c4004.getFrameGenerateCount(&checkToActiveFrames)) {
    Serial.print(F("Current check-to-active frames: "));
    Serial.println(checkToActiveFrames);
  } else {
    Serial.println(F("Read current check-to-active frames failed."));
  }

  if (c4004.setOccLED(true)) {
    Serial.println(F("Set occupancy LED success!"));
  } else {
    Serial.println(F("Set occupancy LED failed!"));
  }
  delay(50);

  if (c4004.setTrkLED(true)) {
    Serial.println(F("Set trajectory LED success!"));
  } else {
    Serial.println(F("Set trajectory LED failed!"));
  }
  delay(50);

  Serial.print(F("Current occupancy LED: "));
  Serial.println(c4004.getOccLED() ? F("ON") : F("OFF"));
  Serial.print(F("Current trajectory LED: "));
  Serial.println(c4004.getTrkLED() ? F("ON") : F("OFF"));

  Serial.println(F("====================Range Param==================="));
  /* Set the four-side boundary detection range */
  DFRobot_C4004::sFourSidedRange_t range;
  range.xMax = 200;
  range.xMin = -200;
  range.yMax = 700;
  range.yMin = 0;

  // Set the boundary detection range
  if (c4004.setFourSidedRangeMode(range)) {
    Serial.println(F("Set four sided range success!"));
  } else {
    Serial.println(F("Set four sided range failed!"));
  }
  delay(50);

  DFRobot_C4004::eDetectionRangeMode_t mode = c4004.getDetectionRangeMode();
  Serial.print(F("Current detection mode: "));
  if (mode == DFRobot_C4004::eRangeFourSide) {
    Serial.println(F("Four-side boundary"));
  } else if (mode == DFRobot_C4004::eRangeTrajectory) {
    Serial.println(F("Trajectory"));
  } else {
    Serial.println(F("Other"));
  }

  if (mode == DFRobot_C4004::eRangeFourSide) {
    DFRobot_C4004::sFourSidedRange_t currentRange;
    if (c4004.getFourSidedRangeMode(&currentRange)) {
      Serial.print(F("Current boundary x+/x-/y+/y- (cm): "));
      Serial.print(currentRange.xMax);
      Serial.print(F("/"));
      Serial.print(currentRange.xMin);
      Serial.print(F("/"));
      Serial.print(currentRange.yMax);
      Serial.print(F("/"));
      Serial.println(currentRange.yMin);
    } else {
      Serial.println(F("Read current boundary range failed."));
    }
  } else {
    Serial.println(F("Current mode is not four-side boundary, skip boundary range check."));
  }

  /*
  * Set the trajectory detection range mode
  * Setting it to false means using this mode and not performing trajectory learning
  */
  // c4004.setTrajectoryRangeMode(false);
  // delay(50);

  // DFRobot_C4004::sPoint_t points[C4004_MAX_POINTS];
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

  /*
   * Set multi-point detection range by config-file mode (custom points).
   * Points are connected in array order: #0 -> #1 -> ... -> #N-1 -> #0 (closed polygon).
   * Example below (clockwise rectangle):
   *   #0 (200, 0) -> #1 (200, 400) -> #2 (-200, 400) -> #3 (-200, 0) -> #0
   */
  // DFRobot_C4004::sPoint_t cfgPoints[4];
  // cfgPoints[0].x = 200;  cfgPoints[0].y = 0;    // #0
  // cfgPoints[1].x = 200;  cfgPoints[1].y = 400;  // #1
  // cfgPoints[2].x = -200; cfgPoints[2].y = 400;  // #2
  // cfgPoints[3].x = -200; cfgPoints[3].y = 0;    // #3
  // if (c4004.setConfigFileModePoints(cfgPoints, 4)) {
  //   Serial.println(F("Set multi-point config points success!"));
  // } else {
  //   Serial.println(F("Set multi-point config points failed!"));
  // }
  // delay(50);

  // DFRobot_C4004::sPoint_t points[C4004_MAX_POINTS];
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
  if (c4004.setRealTimeReportInterval(5)) {
    Serial.println(F("Set RealTimeReportInterval success!"));
  } else {
    Serial.println(F("Set RealTimeReportInterval failed!"));
  }
  delay(50);

  if (c4004.setTrajectoryGenerationDistance(50)) {
    Serial.println(F("Set TrajectoryGenerationDistance success!"));
  } else {
    Serial.println(F("Set TrajectoryGenerationDistance failed!"));
  }
  delay(50);

  if (c4004.setTrajectoryLifetime(10)) {
    Serial.println(F("Set TrajectoryLifetime success!"));
  } else {
    Serial.println(F("Set TrajectoryLifetime failed!"));
  }
  delay(50);

  if (c4004.setUnoccupiedTime(30)) {
    Serial.println(F("Set UnoccupiedTime success!"));
  } else {
    Serial.println(F("Set UnoccupiedTime failed!"));
  }
  delay(50);

  if (c4004.clearLiveCount()) {
    Serial.println(F("Clear people count success!"));
  } else {
    Serial.println(F("Clear people count failed!"));
  }
  delay(50);

  Serial.print(F("Current RealTimeReportInterval(s): "));
  uint32_t peopleInterval = 0;
  if (c4004.getRealTimeReportInterval(&peopleInterval)) {
    Serial.println(peopleInterval);
  } else {
    Serial.println(F("Read current RealTimeReportInterval failed."));
  }
  Serial.print(F("Current TrajectoryGenerationDistance(cm): "));
  uint32_t trajectoryDistance = 0;
  if (c4004.getTrajectoryGenerationDistance(&trajectoryDistance)) {
    Serial.println(trajectoryDistance);
  } else {
    Serial.println(F("Read current TrajectoryGenerationDistance failed."));
  }
  Serial.print(F("Current TrajectoryLifetime(s): "));
  uint32_t time = 0;
  if (c4004.getTrajectoryLifetime(&time)) {
    Serial.println(time);
  } else {
    Serial.println(F("Read current TrajectoryLifetime failed."));
  }
  Serial.print(F("Current UnoccupiedTime(s): "));
  uint32_t noPersonDelay = 0;
  if (c4004.getUnoccupiedTime(&noPersonDelay)) {
    Serial.println(noPersonDelay);
  } else {
    Serial.println(F("Read current UnoccupiedTime failed."));
  }

  Serial.print(F("Current people count(active): "));
  Serial.println(c4004.getLiveCount(DFRobot_C4004::eGetDataActive));

  Serial.println(F("=======================Done======================="));
}

void loop()
{
  delay(1000);
}
