/*!
 * @file getTargetTracking.ino
 * @brief Print live tracked targets in range: position, motion type, speed, and related data.
 * @details Use this example to view tracked targets in the detection range, including position,
 * @n motion feature, speed and related tracking data printed over Serial.
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

const __FlashStringHelper *targetFeatureToString(DFRobot_C4004::eTargetFeature_t feature)
{
  if (feature == DFRobot_C4004::eStatic) {
    return F("Static");
  } else if (feature == DFRobot_C4004::eMotion) {
    return F("Motion");
  } else if (feature == DFRobot_C4004::eUncertain) {
    return F("Unsure");
  } else {
    return F("Unknown");
  }
}

void printTrajectoryData(DFRobot_C4004::eGetDataMode_t dataMode)
{
  const __FlashStringHelper   *title    = NULL;
  const __FlashStringHelper   *modeText = NULL;
  DFRobot_C4004::sTargetInfo_t targets[C4004_MAX_TARGETS];
  uint8_t                      count = c4004.getTargetList(targets, C4004_MAX_TARGETS, dataMode);

  if (dataMode == DFRobot_C4004::eGetDataActive) {
    title    = F("======================TrajectoryActive=======================");
    modeText = F("Active Query");
  } else {
    title    = F("======================TrajectoryReport=======================");
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
    Serial.println(F("Row\tIndex\tKinesia\tFeature\tX(cm)\tY(cm)\tSpeed(cm/s)"));
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

  if (c4004.setTrajectoryTrackEnable(true)) {
    Serial.println(F("Set trajectory track enable success."));
  } else {
    Serial.println(F("Set trajectory track enable failed."));
  }

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
}

void loop()
{
  DFRobot_C4004::eReportedEvent_t event = c4004.getReportedEvent(100);
  /*
   * When state or data changes and the corresponding report function is enabled,
   * the module pushes the update immediately as an event via getReportedEvent().
   * Use the matching getter with DFRobot_C4004::eGetDataReport to read the cached value
   * updated by that report, without issuing an extra UART query.
   */

  // Passively obtain target tracking data
  if (event == DFRobot_C4004::eEventTrajectory) {
    printTrajectoryData(DFRobot_C4004::eGetDataReport);
  }

  // Actively obtain target tracking data
  static uint32_t lastQuery = 0;
  if (millis() - lastQuery > 4000) {
    lastQuery = millis();
    printTrajectoryData(DFRobot_C4004::eGetDataActive);
  }
}
