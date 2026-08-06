/*!
 * @file configureTag.ino
 * @brief Interactively set, clear, and query detection tag zones via a serial menu.
 * @details Serial-menu demo: wait for a single track, then set one tag by size mode;
 * @n also clear one/all tags, query tag list, and watch live tag events.
 * @n Usage environment:
 * @n - Please install the sensor at a height of 180 cm for use.
 * @n - Detection range uses the common four-sided boundary (x +/-200 cm, y 0~700 cm),It can be modified according to one's own needs.
 * @n - Setting a tag requires exactly ONE track in view; stand where the tag center should be.
 *
 * @copyright Copyright (c) 2026 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @license The MIT License (MIT)
 * @author JiaLi(jia.li@dfrobot.com)
 * @version V1.0.0
 * @date 2026-07-30
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

static const uint8_t  singleTrackConfirmTimes = 5;
static const uint16_t trackQueryIntervalMs    = 1000;
static const uint8_t  kMaxTags                = 32;

void                       printMenu(void);
void                       handleSetTag(void);
void                       handleClearTag(void);
void                       handleClearAllTags(void);
void                       handleGetTags(void);
void                       handleWatchTagEvents(void);
bool                       waitForSingleTrack(void);
bool                       readUint16Value(const __FlashStringHelper *prompt, uint16_t *pValue, bool allowCancel);
bool                       readChoice(const __FlashStringHelper *prompt, char minChoice, char maxChoice, char *pChoice, bool allowCancel);
char                       readCommand(void);
char                       waitCommand(void);
void                       flushSerialInput(void);
void                       printCol(const __FlashStringHelper *text, uint8_t width);
void                       printCol(long value, uint8_t width);
const __FlashStringHelper *tagTypeText(DFRobot_C4004::eTagType_t type);
const __FlashStringHelper *tagSetStatusText(DFRobot_C4004::eTagSetStatus_t status);
void                       printTagList(const __FlashStringHelper *title, DFRobot_C4004::sTagConfig_t *tags, uint8_t count);
void                       printTagEvent(const DFRobot_C4004::sTagInfo_t &info);

void setup()
{
  Serial.begin(115200);

  while (!c4004.begin()) {
    Serial.println(F("DFRobot C4004 begin failed, retrying..."));
    delay(1000);
  }
  Serial.println(F("DFRobot C4004 begin success."));

  // Side mount: default 180 cm, recommended 180+/-20 cm. Top mount: recommended 220-280 cm.
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
    Serial.println(F("Set boundary detection range success (about 4 m x 7 m)."));
  } else {
    Serial.println(F("Set boundary detection range failed."));
  }
  delay(50);

  if (c4004.setTrajectoryTrackEnable(true)) {
    Serial.println(F("Enabled trajectory tracking."));
  } else {
    Serial.println(F("Failed to enable trajectory tracking."));
  }
  delay(50);

  if (c4004.setOccLED(true)) {
    Serial.println(F("Enabled occupancy LED."));
  } else {
    Serial.println(F("Failed to enable occupancy LED."));
  }
  delay(50);

  printMenu();
}

void loop()
{
  char cmd = readCommand();
  if (cmd == 0) {
    c4004.getReportedEvent(20);
    return;
  }

  switch (cmd) {
    case '1':
      handleSetTag();
      break;
    case '2':
      handleClearTag();
      break;
    case '3':
      handleClearAllTags();
      break;
    case '4':
      handleGetTags();
      break;
    case '5':
      handleWatchTagEvents();
      break;
    case '6':
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
  Serial.println(F(" ====================Configure Tag Menu==================="));
  Serial.println(F("Standard workflow:"));
  Serial.println(F("  1) Init done, then this menu."));
  Serial.println(F("  2) Send '1' to set one tag: stand at the tag center, keep ONE track."));
  Serial.println(F("  3) Follow prompts to choose type/shape/index/size/ioIndex."));
  Serial.println(F("  4) Send '4' to query tags; send '5' to read tag events."));
  Serial.println(F("  5) Send '2'/'3' to clear one tag / all tags."));
  Serial.println(F(" ---------------------------------------------------------"));
  Serial.println(F("|1: Configure one tag (requires one track)               |"));
  Serial.println(F("|2: Clear one tag by index                               |"));
  Serial.println(F("|3: Clear all tags                                       |"));
  Serial.println(F("|4: Query configured tags                                |"));
  Serial.println(F("|5: Read tag events (send q to stop)                     |"));
  Serial.println(F("|6: Print this menu                                      |"));
  Serial.println(F(" ========================================================="));
}

void handleSetTag(void)
{
  DFRobot_C4004::sTagConfig_t tag;
  uint16_t                    value  = 0;
  char                        choice = 0;

  memset(&tag, 0, sizeof(tag));

  Serial.println();
  Serial.println(F(" ========================setTag==========================="));
  Serial.println(F("Stand where the tag center should be."));
  Serial.println(F("Keep exactly ONE track in view."));
  Serial.println(F("Note: setTag uses size mode; centerX/centerY are taken from the current track."));
  Serial.println(F("Send q to cancel while waiting."));

  if (!waitForSingleTrack()) {
    Serial.println(F("setTag canceled before start."));
    printMenu();
    return;
  }

  Serial.println(F("Single-track condition confirmed."));
  Serial.println(F("Please stay still at the current position until setTag finishes."));
  flushSerialInput();

  if (!readUint16Value(F("Enter tagIndex (0-254), then Enter: "), &value, true)) {
    Serial.println(F("setTag canceled."));
    printMenu();
    return;
  }
  tag.tagIndex = (uint8_t)value;

  Serial.println(F("Select tagType:"));
  Serial.println(F("  1: Boundary"));
  Serial.println(F("  2: ApproachAway"));
  Serial.println(F("  3: PeopleCounting"));
  Serial.println(F("  4: Noise"));
  if (!readChoice(F("Enter 1-4 (or q to cancel): "), '1', '4', &choice, true)) {
    Serial.println(F("setTag canceled."));
    printMenu();
    return;
  }
  switch (choice) {
    case '1':
      tag.tagType = DFRobot_C4004::eTagBoundary;
      break;
    case '2':
      tag.tagType = DFRobot_C4004::eTagApproachAway;
      break;
    case '3':
      tag.tagType = DFRobot_C4004::eTagPeopleCounting;
      break;
    default:
      tag.tagType = DFRobot_C4004::eTagNoise;
      break;
  }

  Serial.println(F("Select scopeType:"));
  Serial.println(F("  1: Rectangle (width=X size cm, height=Y size cm)"));
  Serial.println(F("  2: Circle (enter radius cm)"));
  if (!readChoice(F("Enter 1-2 (or q to cancel): "), '1', '2', &choice, true)) {
    Serial.println(F("setTag canceled."));
    printMenu();
    return;
  }
  tag.scopeType = (choice == '2') ? DFRobot_C4004::eCircle : DFRobot_C4004::eRectangle;

  if (tag.scopeType == DFRobot_C4004::eRectangle) {
    if (!readUint16Value(F("Enter width (cm), then Enter: "), &value, true)) {
      Serial.println(F("setTag canceled."));
      printMenu();
      return;
    }
    tag.width = value;

    if (!readUint16Value(F("Enter height (cm), then Enter: "), &value, true)) {
      Serial.println(F("setTag canceled."));
      printMenu();
      return;
    }
    tag.height = value;
  } else {
    if (!readUint16Value(F("Enter radius (cm), then Enter: "), &value, true)) {
      Serial.println(F("setTag canceled."));
      printMenu();
      return;
    }
    tag.width  = value;
    tag.height = 0;
  }

  Serial.println(F("Select ioIndex: 0=unused, 2-6=IO2-IO6 (IO1 is reserved)."));
  if (!readUint16Value(F("Enter ioIndex (0 or 2-6), then Enter: "), &value, true)) {
    Serial.println(F("setTag canceled."));
    printMenu();
    return;
  }
  if (!(value == 0 || (value >= 2 && value <= 6))) {
    Serial.println(F("Invalid ioIndex. setTag canceled."));
    printMenu();
    return;
  }
  tag.ioIndex = (uint8_t)value;

  Serial.println(F("Calling setTag()..."));
  DFRobot_C4004::eTagSetStatus_t status = c4004.setTag(tag);
  Serial.print(F("setTag status: "));
  Serial.println(tagSetStatusText(status));

  if (status == DFRobot_C4004::eTagSetSuccess) {
    handleGetTags();
  } else {
    printMenu();
  }
}

void handleClearTag(void)
{
  uint16_t tagIndex = 0;

  Serial.println();
  Serial.println(F(" =======================clearTag=========================="));
  flushSerialInput();
  if (!readUint16Value(F("Enter tagIndex to clear (0-254), then Enter (q cancel): "), &tagIndex, true)) {
    Serial.println(F("clearTag canceled."));
    printMenu();
    return;
  }

  if (c4004.clearTag(tagIndex)) {
    Serial.println(F("clearTag success."));
  } else {
    Serial.println(F("clearTag failed."));
  }
  printMenu();
}

void handleClearAllTags(void)
{
  Serial.println();
  Serial.println(F(" =====================clearAllTags========================"));
  if (c4004.clearAllTags()) {
    Serial.println(F("clearAllTags success."));
  } else {
    Serial.println(F("clearAllTags failed."));
  }
  printMenu();
}

void handleGetTags(void)
{
  DFRobot_C4004::sTagConfig_t tags[kMaxTags];
  uint8_t                     count = c4004.getTags(tags, kMaxTags);
  printTagList(F("Configured tag list:"), tags, count);
  printMenu();
}

void handleWatchTagEvents(void)
{
  Serial.println();
  Serial.println(F(" =====================Read Tag Events======================"));
  Serial.println(F("Reading tag events. Send q to stop and return to menu."));

  while (true) {
    char cmd = readCommand();
    if (cmd == 'q' || cmd == 'Q') {
      Serial.println(F("Stop reading tag events."));
      printMenu();
      return;
    }

    DFRobot_C4004::eReportedEvent_t event = c4004.getReportedEvent(50);
    if (event == DFRobot_C4004::eEventTag) {
      DFRobot_C4004::sTagInfo_t tagInfo;
      if (c4004.getTagInfo(&tagInfo)) {
        printTagEvent(tagInfo);
      }
    }
  }
}

bool waitForSingleTrack(void)
{
  uint8_t confirmCount = 0;

  while (confirmCount < singleTrackConfirmTimes) {
    uint32_t startTime   = millis();
    uint8_t  targetCount = c4004.getTargetList(NULL, 0, DFRobot_C4004::eGetDataActive);

    Serial.print(F("Active track count: "));
    Serial.print(targetCount);

    if (targetCount == 1) {
      confirmCount++;
    } else {
      confirmCount = 0;
    }

    Serial.print(F("  confirm: "));
    Serial.print(confirmCount);
    Serial.print(F("/"));
    Serial.println(singleTrackConfirmTimes);

    while ((uint32_t)(millis() - startTime) < trackQueryIntervalMs) {
      char cmd = readCommand();
      if (cmd == 'q' || cmd == 'Q') {
        return false;
      }
      c4004.getReportedEvent(10);
    }
  }

  return true;
}

bool readUint16Value(const __FlashStringHelper *prompt, uint16_t *pValue, bool allowCancel)
{
  if (pValue == NULL) {
    return false;
  }

  while (true) {
    Serial.print(prompt);

    String line;
    while (true) {
      while (Serial.available() > 0) {
        char ch = (char)Serial.read();
        if (ch == '\r') {
          continue;
        }
        if (ch == '\n') {
          goto parse_line;
        }
        if ((ch >= '0' && ch <= '9') || ch == 'q' || ch == 'Q') {
          line += ch;
          Serial.print(ch);
        }
      }
      c4004.getReportedEvent(10);
    }

  parse_line:
    Serial.println();
    if (allowCancel && (line == "q" || line == "Q")) {
      return false;
    }
    if (line.length() == 0) {
      Serial.println(F("Empty input, try again."));
      continue;
    }

    bool digitsOnly = true;
    for (uint16_t i = 0; i < line.length(); i++) {
      if (line[i] < '0' || line[i] > '9') {
        digitsOnly = false;
        break;
      }
    }
    if (!digitsOnly) {
      Serial.println(F("Invalid input, try again."));
      continue;
    }

    long value = line.toInt();
    if (value < 0 || value > 65535L) {
      Serial.println(F("Out of range, try again."));
      continue;
    }
    *pValue = (uint16_t)value;
    return true;
  }
}

bool readChoice(const __FlashStringHelper *prompt, char minChoice, char maxChoice, char *pChoice, bool allowCancel)
{
  if (pChoice == NULL) {
    return false;
  }

  while (true) {
    Serial.print(prompt);
    char cmd = waitCommand();
    Serial.println(cmd);

    if (allowCancel && (cmd == 'q' || cmd == 'Q')) {
      return false;
    }
    if (cmd >= minChoice && cmd <= maxChoice) {
      *pChoice = cmd;
      return true;
    }
    Serial.println(F("Invalid choice, try again."));
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

void flushSerialInput(void)
{
  delay(20);
  while (Serial.available() > 0) {
    Serial.read();
  }
}

void printCol(const __FlashStringHelper *text, uint8_t width)
{
  uint8_t len = strlen_P(reinterpret_cast<PGM_P>(text));
  Serial.print(text);
  for (uint8_t i = len; i < width; i++) {
    Serial.print(' ');
  }
}

void printCol(long value, uint8_t width)
{
  char buf[12];
  ltoa(value, buf, 10);
  Serial.print(buf);
  for (uint8_t i = (uint8_t)strlen(buf); i < width; i++) {
    Serial.print(' ');
  }
}

const __FlashStringHelper *tagTypeText(DFRobot_C4004::eTagType_t type)
{
  switch (type) {
    case DFRobot_C4004::eTagNone:
      return F("None");
    case DFRobot_C4004::eTagBoundary:
      return F("Boundary");
    case DFRobot_C4004::eTagApproachAway:
      return F("ApproachAway");
    case DFRobot_C4004::eTagPeopleCounting:
      return F("PeopleCounting");
    case DFRobot_C4004::eTagNoise:
      return F("Noise");
    default:
      return F("Unknown");
  }
}

const __FlashStringHelper *tagSetStatusText(DFRobot_C4004::eTagSetStatus_t status)
{
  switch (status) {
    case DFRobot_C4004::eTagSetSuccess:
      return F("Success");
    case DFRobot_C4004::eTagSetTrackCountError:
      return F("TrackCountError (need exactly 1 track)");
    case DFRobot_C4004::eTagSetAlreadyUsed:
      return F("AlreadyUsed");
    case DFRobot_C4004::eTagSetIndexOutOfRange:
      return F("IndexOutOfRange");
    case DFRobot_C4004::eTagSetCommError:
    default:
      return F("CommError");
  }
}

void printTagList(const __FlashStringHelper *title, DFRobot_C4004::sTagConfig_t *tags, uint8_t count)
{
  Serial.println(F("======================================================================"));
  Serial.println(title);
  Serial.println(F("----------------------------------------------------------------------"));
  Serial.print(F("Tag count: "));
  Serial.println(count);
  if (count == 0) {
    Serial.println(F("No tag."));
    Serial.println();
    return;
  }

  printCol(F("Idx"), 5);
  printCol(F("Type"), 16);
  printCol(F("Range"), 11);
  printCol(F("IO"), 4);
  printCol(F("CenterX"), 9);
  printCol(F("CenterY"), 9);
  printCol(F("Width"), 7);
  Serial.println(F("Height"));
  for (uint8_t i = 0; i < count; i++) {
    printCol((long)tags[i].tagIndex, 5);
    printCol(tagTypeText(tags[i].tagType), 16);
    printCol(tags[i].scopeType == DFRobot_C4004::eCircle ? F("Circle") : F("Rectangle"), 11);
    printCol((long)tags[i].ioIndex, 4);
    printCol((long)tags[i].centerX, 9);
    printCol((long)tags[i].centerY, 9);
    printCol((long)tags[i].width, 7);
    Serial.println(tags[i].height);
  }
  Serial.println();
}

void printTagEvent(const DFRobot_C4004::sTagInfo_t &info)
{
  Serial.println(F("======================================================================"));
  Serial.println(F("============================TagEventReport============================"));
  Serial.println(F("======================================================================"));
  Serial.print(F("Tag Index : "));
  Serial.println(info.tagIndex);
  Serial.print(F("Tag Type  : "));
  Serial.println(tagTypeText(info.tagType));
  Serial.print(F("IO Index  : "));
  Serial.println(info.ioIndex);
  Serial.print(F("Center XY : "));
  Serial.print(info.centerX);
  Serial.print(F(" / "));
  Serial.println(info.centerY);

  if (info.tagType == DFRobot_C4004::eTagBoundary) {
    Serial.print(F("Event     : Boundary ("));
    if (info.enterExit == DFRobot_C4004::eEnter) {
      Serial.print(F("Enter"));
    } else if (info.enterExit == DFRobot_C4004::eExit) {
      Serial.print(F("Exit"));
    } else {
      Serial.print(F("None"));
    }
    Serial.println(F(")"));
  } else if (info.tagType == DFRobot_C4004::eTagApproachAway) {
    Serial.print(F("Event     : MotionDirection ("));
    if (info.motionDir == DFRobot_C4004::eApproach) {
      Serial.print(F("Approach"));
    } else if (info.motionDir == DFRobot_C4004::eAway) {
      Serial.print(F("Away"));
    } else {
      Serial.print(F("None"));
    }
    Serial.println(F(")"));
  } else if (info.tagType == DFRobot_C4004::eTagPeopleCounting) {
    Serial.print(F("Event     : PeopleCounting (M:"));
    Serial.print(info.motionNum);
    Serial.print(F(" S:"));
    Serial.print(info.staticNum);
    Serial.println(F(")"));
  } else {
    Serial.print(F("Event     : "));
    Serial.println(tagTypeText(info.tagType));
  }
  Serial.println();
}
