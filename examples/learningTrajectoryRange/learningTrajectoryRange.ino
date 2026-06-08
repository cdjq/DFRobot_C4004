/*!
 * @file learningTrajectoryRange.ino
 * @brief Example for learning trajectory range
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

static const uint8_t SINGLE_PERSON_CONFIRM_TIMES = 5;
static const uint16_t PEOPLE_QUERY_INTERVAL_MS = 1000;

void printMenu(void);
void learnTrajectoryRange(void);
bool waitForSinglePerson(void);
void setUseTrajectoryRangeMode(void);
void queryTrajectoryRange(void);
void printTrajectoryPoints(const sPoint_t *points, uint16_t pointCount);
char readCommand(void);
char waitCommand(void);

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

  Serial.println(F(" ====================Init Params==================="));

  sFourSidedRange_t range;
  range.mode = eRangeFourSide;
  range.xPositiveCm = 500;
  range.xNegativeCm = -500;
  range.yPositiveCm = 800;
  range.yNegativeCm = 0;

  if (c4004.setFourSidedRangeMode(range)) {
    Serial.println(F("Set four sided range success!"));
  } else {
    Serial.println(F("Set four sided range failed!"));
  }
  delay(50);

  if (c4004.setPresenceEnable(false)) {
    Serial.println(F("Disabled presence report."));
  } else {
    Serial.println(F("Failed to disable presence report."));
  }
  delay(50);

  if (c4004.setRealTimePeopleTime(0)) {
    Serial.println(F("Disabled people count report."));
  } else {
    Serial.println(F("Failed to disable people count report."));
  }
  delay(50);

  if (c4004.setTrajectoryTrackEnable(true)) {
    Serial.println(F("Enabled trajectory tracking."));
  } else {
    Serial.println(F("Failed to enable trajectory tracking."));
  }
  delay(50);

  if (c4004.setMotionLed(true)) {
    Serial.println(F("Turned on motion LED."));
  } else {
    Serial.println(F("Failed to turn on motion LED."));
  }
  delay(50);

  if (c4004.setTrajectoryLed(true)) {
    Serial.println(F("Turned on trajectory LED."));
  } else {
    Serial.println(F("Failed to turn on trajectory LED."));
  }
  delay(50);

  printMenu();
}

void loop()
{
  char cmd = readCommand();

  if (cmd == 0) {
    c4004.getReportedInfo(10);
    return;
  }
  switch (cmd) {
    case '1':
      learnTrajectoryRange();
      break;
    case '2':
      setUseTrajectoryRangeMode();
      printMenu();
      break;
    case '3':
      queryTrajectoryRange();
      printMenu();
      break;
    case '4':
      printMenu();
      break;
    default:
      Serial.println(F("Unknown command."));
      printMenu();
      break;
  }
}

void printMenu(void)
{
  Serial.println();
  Serial.println(F(" ================Trajectory Range Menu============="));
  Serial.println(F("|1: Learn trajectory range                         |"));
  Serial.println(F("|2: Use trajectory range mode                      |"));
  Serial.println(F("|3: Query trajectory range points                  |"));
  Serial.println(F("|4: Print this menu                                |"));
  Serial.println(F("|During learning: send e to stop, q to cancel      |"));
  Serial.println(F(" =================================================="));
}

void learnTrajectoryRange(void)
{
  Serial.println();
  Serial.println(F(" =================Learn Trajectory================="));
  Serial.println(F("Waiting until active people count is 1 for 5 times."));
  Serial.println(F("Send q to cancel."));

  if (!waitForSinglePerson()) {
    Serial.println(F("Learning canceled before start."));
    printMenu();
    return;
  }

  Serial.println(F("Single-person condition confirmed."));
  Serial.println(F("Send s to start trajectory learning, or q to cancel."));

  while (true) {
    char cmd = waitCommand();

    if (cmd == 'q' || cmd == 'Q') {
      Serial.println(F("Learning canceled before start."));
      printMenu();
      return;
    }
    if (cmd == 's' || cmd == 'S') {
      break;
    }

    Serial.println(F("Send s to start, or q to cancel."));
  }

  Serial.print(F("Start trajectory learning: "));
  if (!c4004.setTrajectoryRangeMode(true)) {
    Serial.println(F("FAILED"));
    printMenu();
    return;
  }
  Serial.println(F("OK"));

  Serial.println(F("Learning is running. Walk the required boundary path."));
  Serial.println(F("Send e to stop learning."));

  while (true) {
    char cmd = readCommand();

    if (cmd == 'e' || cmd == 'E' || cmd == 'q' || cmd == 'Q') {
      break;
    }

    c4004.getReportedInfo(50);
  }

  Serial.print(F("Set trajectory range mode (learning off): "));
  if (c4004.setTrajectoryRangeMode(false)) {
    Serial.println(F("OK"));
  } else {
    Serial.println(F("FAILED"));
  }

  delay(200);
  queryTrajectoryRange();
  printMenu();
}

bool waitForSinglePerson(void)
{
  uint8_t confirmCount = 0;

  while (confirmCount < SINGLE_PERSON_CONFIRM_TIMES) {
    uint32_t startTime = millis();
    sTargetInfo_t targets[MAX_TARGETS];
    uint8_t targetCount = c4004.getTargetList(targets, MAX_TARGETS, eGetDataActive);

    Serial.print(F("Active target count: "));
    Serial.print(targetCount);

    if (targetCount == 1) {
      confirmCount++;
    } else {
      confirmCount = 0;
    }

    Serial.print(F("  confirm: "));
    Serial.print(confirmCount);
    Serial.print(F("/"));
    Serial.println(SINGLE_PERSON_CONFIRM_TIMES);

    while ((uint32_t)(millis() - startTime) < PEOPLE_QUERY_INTERVAL_MS) {
      char cmd = readCommand();

      if (cmd == 'q' || cmd == 'Q') {
        return false;
      }
      c4004.getReportedInfo(10);
    }
  }

  return true;
}

void setUseTrajectoryRangeMode(void)
{
  Serial.println();
  Serial.println(F(" ===============Use Trajectory Range==============="));
  Serial.print(F("Set trajectory range mode (learning off): "));
  Serial.println(c4004.setTrajectoryRangeMode(false) ? F("OK") : F("FAILED"));
}

void queryTrajectoryRange(void)
{
  sPoint_t points[MAX_POINTS];
  uint16_t pointCount = 0;

  Serial.println();
  Serial.println(F(" ==============Query Trajectory Range=============="));

  if (!c4004.getTrajectoryRangeMode(points, &pointCount)) {
    Serial.println(F("Query trajectory range failed."));
    return;
  }

  Serial.print(F("Trajectory point count: "));
  Serial.println(pointCount);
  printTrajectoryPoints(points, pointCount);
}

void printTrajectoryPoints(const sPoint_t *points, uint16_t pointCount)
{
  for (uint16_t i = 0; i < pointCount; i++) {
    Serial.print(F("#"));
    Serial.print(i);
    Serial.print(F(" x/y="));
    Serial.print(points[i].x);
    Serial.print(F("/"));
    Serial.println(points[i].y);
  }
}

char readCommand(void)
{
  while (Serial.available() > 0) {
    char cmd = (char)Serial.read();

    if (cmd == '\r' || cmd == '\n' || cmd == ' ') {
      continue;
    }
    return cmd;
  }

  return 0;
}

char waitCommand(void)
{
  while (true) {
    char cmd = readCommand();

    if (cmd != 0) {
      return cmd;
    }

    c4004.getReportedInfo(20);
  }
}
