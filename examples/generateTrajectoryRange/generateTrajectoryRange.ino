/*!
 * @file generateTrajectoryRange.ino
 * @brief Generate/learn a trajectory-based detection range and apply it.
 * @details Interactive Serial demo: learn a custom detection polygon by walking a path,
 * @n then switch to trajectory-range mode and query the learned points.
 * @n Usage environment:
 * @n - Use this example when you need a custom detection boundary learned from a real walk path
 * @n   (instead of only a four-sided rectangle).
 * @n - Please install the sensor at a height of 180 cm for use.
 * @n - Keep the sensor FOV open and free of strong multipath interference.
 * @n - During learning, keep exactly ONE person in the FOV (the demo waits for this condition).
 * @n - Open Serial Monitor at 115200 baud; send single-character commands (1/2/3/4/s/e/q).
 *
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

static const uint8_t  singlePersonConfirmTimes = 5;
static const uint16_t peopleQueryIntervalMs    = 1000;

void printMenu(void);
void learnTrajectoryRange(void);
bool waitForSinglePerson(void);
void setUseTrajectoryRangeMode(void);
void queryTrajectoryRange(void);
void printTrajectoryPoints(const DFRobot_C4004::sPoint_t *points, uint16_t pointCount);
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

  Serial.println(F(" ====================Init Params==================="));

  DFRobot_C4004::sFourSidedRange_t range;
  range.xMax = 500;
  range.xMin = -500;
  range.yMax = 800;
  range.yMin = 0;

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

  if (c4004.setRealTimeReportInterval(0)) {
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

  if (c4004.setOccLED(true)) {
    Serial.println(F("Turned on occupancy LED."));
  } else {
    Serial.println(F("Failed to turn on occupancy LED."));
  }
  delay(50);

  if (c4004.setTrkLED(true)) {
    Serial.println(F("Turned on trajectory LED."));
  } else {
    Serial.println(F("Failed to turn on trajectory LED."));
  }
  delay(50);

  Serial.println();
  Serial.println(F("Usage: open Serial Monitor @115200, keep ONE person in FOV while learning."));
  printMenu();
}

void loop()
{
  /*
   * When state or data changes and the corresponding report function is enabled,
   * the module pushes the update immediately as an event via getReportedEvent().
   * Use the matching getter with DFRobot_C4004::eGetDataReport to read the cached value
   * updated by that report, without issuing an extra UART query.
   */
  char cmd = readCommand();

  if (cmd == 0) {
    c4004.getReportedEvent(10);
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
  Serial.println(F("Standard workflow:"));
  Serial.println(F("  1) Wait for begin success and init, then this menu."));
  Serial.println(F("  2) Send '1' to start learning."));
  Serial.println(F("  3) Keep only one person in view until confirm 5/5 (or send 'q' to cancel)."));
  Serial.println(F("  4) Send 's' to start trajectory learning."));
  Serial.println(F("  5) Walk along the intended boundary path."));
  Serial.println(F("  6) Send 'e' to stop learning (or 'q'); points are queried."));
  Serial.println(F("  7) Send '2' later to use trajectory-range mode again."));
  Serial.println(F("  8) Send '3' to query points; send '4' to reprint menu."));
  Serial.println(F(" --------------------------------------------------"));
  Serial.println(F("|1: Learn trajectory range                         |"));
  Serial.println(F("|2: Use trajectory range mode                      |"));
  Serial.println(F("|3: Query trajectory range points                  |"));
  Serial.println(F("|4: Print this menu                                |"));
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

  Serial.println(F("Start trajectory learning..."));
  c4004.setTrajectoryRangeMode(true);
  Serial.println(F("OK"));

  Serial.println(F("Learning is running. Walk the required boundary path."));
  Serial.println(F("Send e to stop learning."));

  while (true) {
    char cmd = readCommand();

    if (cmd == 'e' || cmd == 'E' || cmd == 'q' || cmd == 'Q') {
      break;
    }

    c4004.getReportedEvent(50);
  }

  Serial.println(F("Set trajectory range mode (learning off)."));
  c4004.setTrajectoryRangeMode(false);

  delay(200);
  queryTrajectoryRange();
  printMenu();
}

bool waitForSinglePerson(void)
{
  uint8_t confirmCount = 0;

  while (confirmCount < singlePersonConfirmTimes) {
    uint32_t startTime   = millis();
    uint8_t  targetCount = c4004.getTargetList(NULL, 0, DFRobot_C4004::eGetDataActive);

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
    Serial.println(singlePersonConfirmTimes);

    while ((uint32_t)(millis() - startTime) < peopleQueryIntervalMs) {
      char cmd = readCommand();

      if (cmd == 'q' || cmd == 'Q') {
        return false;
      }
      c4004.getReportedEvent(10);
    }
  }

  return true;
}

void setUseTrajectoryRangeMode(void)
{
  Serial.println();
  Serial.println(F(" ===============Use Trajectory Range==============="));
  Serial.println(F("Set trajectory range mode (learning off)."));
  c4004.setTrajectoryRangeMode(false);
}

void queryTrajectoryRange(void)
{
  DFRobot_C4004::sPoint_t points[C4004_MAX_POINTS];
  uint16_t pointCount = 0;

  Serial.println();
  Serial.println(F(" ==============Query Trajectory Range=============="));

  if (!c4004.getTrajectoryRangeMode(points, &pointCount)) {
    Serial.println(F("Query trajectory range failed."));
    Serial.println(F("Possible causes:"));
    Serial.println(F("  1) Trajectory range mode not enabled. Send '2' in the menu to enable it."));
    Serial.println(F("  2) Hardware wiring issue or hardware damage."));
    Serial.println(F("  3) Large data volume; SoftSerial is prone to packet loss."));
    return;
  }

  Serial.print(F("Trajectory point count: "));
  Serial.println(pointCount);
  printTrajectoryPoints(points, pointCount);
}

void printTrajectoryPoints(const DFRobot_C4004::sPoint_t *points, uint16_t pointCount)
{
  const uint8_t pointsPerLine = 4;

  if (points == NULL || pointCount == 0) {
    Serial.println(F("(no points)"));
    return;
  }

  Serial.println(F("Point coordinates (x, y):"));
  for (uint16_t i = 0; i < pointCount; i++) {
    char buf[20];
    snprintf(buf, sizeof(buf), "(%5d,%5d)", points[i].x, points[i].y);
    Serial.print(buf);

    if ((i + 1) % pointsPerLine == 0 || i + 1 == pointCount) {
      Serial.println();
    } else {
      Serial.print(F("  "));
    }
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

    c4004.getReportedEvent(20);
  }
}
