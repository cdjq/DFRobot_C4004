/*! 
 * @file multiZoneStatus.ino
 * @brief Read multi-zone tag events and print living-room scene linkage status.
 * @details This routine can use tags configured by the PC tool or optionally configure
 * @n tags in code. It keeps the last event result for each tag, prints a summary table
 * @n every 3 seconds or when a tag event arrives, and drives outputs based on
 * @n game-area and sofa-area people counting results.
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

const uint8_t TV_CTRL_PIN = 2;      // HIGH: turn off TV
const uint8_t LIGHT_CTRL_PIN = 3;   // HIGH: dim lights

const uint8_t TAG_GAME = 0;
const uint8_t TAG_SOFA = 1;
const uint8_t TAG_HOME_DOOR = 2;
const uint8_t TAG_KITCHEN_DOOR = 3;
const uint8_t TAG_DINING = 4;
const uint8_t TAG_CURTAIN = 5;
const uint8_t TAG_PLANT = 6;
const uint8_t TAG_TOTAL = 7;

// Users can adjust these times according to their own preferences, needs, application scenarios, etc. The default time is 5 seconds
const uint32_t GAME_NO_PERSON_DELAY_MS = 5000;   // The game area becomes a time for no one to delay
const uint32_t SOFA_STATIC_DELAY_MS = 5000;      // The sofa area becomes a static delay area
const uint32_t SOFA_MOTION_DELAY_MS = 5000;      // The sofa area becomes a motion delay area
const uint32_t SOFA_EMPTY_DELAY_MS = 5000;       // The sofa area becomes an empty delay area

const uint8_t LIGHT_PWM_LOW = 0;
const uint8_t LIGHT_PWM_DIM = 150;
const uint8_t LIGHT_PWM_HIGH = 255;

const char *tagNames[TAG_TOTAL] = {"Game", "Sofa", "HomeDoor", "KitchenDoor", "Dining", "CurtainNoise", "PlantNoise"};

sTagInfo_t tagCache[TAG_TOTAL];
bool tagPrintPending = false;
bool tvOutputHigh = false;
uint8_t lightOutputValue = LIGHT_PWM_LOW;
uint32_t gameNoPersonStartMs = 0;
uint32_t sofaStaticStartMs = 0;
uint32_t sofaMotionStartMs = 0;
uint32_t sofaEmptyStartMs = 0;

const char *tagTypeToText(eTagType_t type)
{
  if (type == eTagNone) {
    return "None";
  } else if (type == eTagBoundary) {
    return "Boundary";
  } else if (type == eTagApproachAway) {
    return "ApproachAway";
  } else if (type == eTagPeopleCounting) {
    return "PeopleCount";
  } else if (type == eTagNoise) {
    return "Noise";
  }
  return "Unknown";
}

void initTagCacheFromConfig(const sTagConfig_t *tags, uint8_t count)
{
  for (uint8_t i = 0; i < TAG_TOTAL; i++) {
    memset(&tagCache[i], 0, sizeof(sTagInfo_t));
    tagCache[i].tagIndex = i;
    tagCache[i].tagType = eTagNone;
    tagCache[i].enterExit = eBoundaryDirection_t::eNone;
    tagCache[i].motionDir = eApproachAwayDirection_t::eNone;
  }

  for (uint8_t i = 0; i < count; i++) {
    uint8_t index = tags[i].tagIndex;
    if (index >= TAG_TOTAL) {
      continue;
    }
    tagCache[index].tagIndex = tags[i].tagIndex;
    tagCache[index].tagType = tags[i].tagType;
    tagCache[index].ioIndex = tags[i].ioIndex;
    tagCache[index].centerX = tags[i].centerX;
    tagCache[index].centerY = tags[i].centerY;
  }
}

bool initTagCacheFromDevice()
{
  sTagConfig_t tags[TAG_TOTAL];
  uint8_t count = c4004.getTags(tags, (uint8_t)(sizeof(tags) / sizeof(tags[0])));

  initTagCacheFromConfig(tags, count);

  Serial.print(F("Read tag config count: "));
  Serial.println(count);
  return count > 0;
}

void setup()
{
  Serial.begin(115200);

  pinMode(TV_CTRL_PIN, OUTPUT);
  pinMode(LIGHT_CTRL_PIN, OUTPUT);
  digitalWrite(TV_CTRL_PIN, LOW);
  analogWrite(LIGHT_CTRL_PIN, LIGHT_PWM_LOW);

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

  if (c4004.setPresenceEnable(true)) {
    Serial.println(F("Set presence enable success."));
  } else {
    Serial.println(F("Set presence enable failed."));
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

  sTagConfig_t tags[TAG_TOTAL] = {};

/**
 * Tag configuration note:
 * If the tags have already been configured by the PC tool, you do not need
 * to configure them again here. In that case, keep the following tag setup
 * code commented out.
 *
 * If you want this example to configure tags automatically, uncomment the
 * clearAllTags(), sTagConfig_t tags[], and setTagsFromConfig() code below.
 *
 * Field meaning:
 *   tagIndex : Tag index. It must be unique for each tag.
 *   tagType  : Tag function, such as PeopleCounting, ApproachAway, or Noise.
 *   scopeType: Tag shape. Use eTagRangeRectangle or eTagRangeCircle.
 *   ioIndex  : IO linkage index. 0 means unused; 2-6 maps to IO2-IO6.
 *   centerX  : Tag center X coordinate, in cm.
 *   centerY  : Tag center Y coordinate, in cm.
 *   width    : Rectangle width, or circle radius, in cm.
 *   height   : Rectangle height, in cm. Not used for circle tags.
 */
  if (c4004.clearAllTags()) {
    Serial.println(F("Clear all tags success."));
  } else {
    Serial.println(F("Clear all tags failed."));
  }

  tags[0].tagIndex = TAG_GAME;
  tags[0].tagType = eTagPeopleCounting;
  tags[0].scopeType = eTagRangeCircle;
  tags[0].ioIndex = 0;
  tags[0].centerX = -100;
  tags[0].centerY = 550;
  tags[0].width = 80;
  tags[0].height = 0;

  tags[1].tagIndex = TAG_SOFA;
  tags[1].tagType = eTagPeopleCounting;
  tags[1].scopeType = eTagRangeRectangle;
  tags[1].ioIndex = 0;
  tags[1].centerX = 100;
  tags[1].centerY = 450;
  tags[1].width = 100;
  tags[1].height = 300;

  tags[2].tagIndex = TAG_HOME_DOOR;
  tags[2].tagType = eTagBoundary;
  tags[2].scopeType = eTagRangeRectangle;
  tags[2].ioIndex = 0;
  tags[2].centerX = 100;
  tags[2].centerY = 700;
  tags[2].width = 80;
  tags[2].height = 40;

  tags[3].tagIndex = TAG_KITCHEN_DOOR;
  tags[3].tagType = eTagBoundary;
  tags[3].scopeType = eTagRangeRectangle;
  tags[3].ioIndex = 0;
  tags[3].centerX = -100;
  tags[3].centerY = 700;
  tags[3].width = 80;
  tags[3].height = 40;

  tags[4].tagIndex = TAG_DINING;
  tags[4].tagType = eTagPeopleCounting;
  tags[4].scopeType = eTagRangeRectangle;
  tags[4].ioIndex = 0;
  tags[4].centerX = 50;
  tags[4].centerY = 150;
  tags[4].width = 300;
  tags[4].height = 150;

  tags[5].tagIndex = TAG_CURTAIN;
  tags[5].tagType = eTagNoise;
  tags[5].scopeType = eTagRangeRectangle;
  tags[5].ioIndex = 0;
  tags[5].centerX = -150;
  tags[5].centerY = 300;
  tags[5].width = 50;
  tags[5].height = 300;

  tags[6].tagIndex = TAG_PLANT;
  tags[6].tagType = eTagNoise;
  tags[6].scopeType = eTagRangeCircle;
  tags[6].ioIndex = 0;
  tags[6].centerX = -50;
  tags[6].centerY = 400;
  tags[6].width = 40;
  tags[6].height = 0;

  if (c4004.setTagsFromConfig(tags, TAG_TOTAL)) {
    Serial.println(F("Set 7 tags from config success."));
  } else {
    Serial.println(F("Set 7 tags from config failed."));
  }

  if (initTagCacheFromDevice()) {
    Serial.println(F("Init tag cache from device config success."));
  } else {
    Serial.println(F("No device tag config read, tag cache uses default empty values."));
  }

  Serial.println(F("==================================================================="));
  Serial.println(F("Room occupancy inference started."));
  Serial.println(F("Rule 1: Game area has person -> TV IO HIGH immediately; no person for 5s -> LOW."));
  Serial.println(F("Rule 2: Sofa static-only for 5s -> Light PWM 150; motion for 5s -> 0; no person for 5s -> 255."));
  Serial.println(F("==================================================================="));
}

void loop()
{
  eReportedEvent_t event = c4004.getReportedInfo(100);

  uint32_t nowMs = millis();

  if (event == eEventTag) {
    sTagInfo_t tagInfo;
    if (c4004.getTagInfo(&tagInfo) && tagInfo.tagIndex < TAG_TOTAL) {
      tagCache[tagInfo.tagIndex].tagIndex = tagInfo.tagIndex;
      tagCache[tagInfo.tagIndex].tagType = tagInfo.tagType;
      tagCache[tagInfo.tagIndex].ioIndex = tagInfo.ioIndex;
      tagCache[tagInfo.tagIndex].centerX = tagInfo.centerX;
      tagCache[tagInfo.tagIndex].centerY = tagInfo.centerY;
      tagCache[tagInfo.tagIndex].enterExit = tagInfo.enterExit;
      tagCache[tagInfo.tagIndex].motionDir = tagInfo.motionDir;
      tagCache[tagInfo.tagIndex].motionNum = tagInfo.motionNum;
      tagCache[tagInfo.tagIndex].staticNum = tagInfo.staticNum;
      tagPrintPending = true;
    }
  }

  bool gameHasPerson = (tagCache[TAG_GAME].motionNum + tagCache[TAG_GAME].staticNum) > 0;
  if (gameHasPerson) {
    tvOutputHigh = true;
    gameNoPersonStartMs = 0;
  } else if (tvOutputHigh) {
    if (gameNoPersonStartMs == 0) {
      gameNoPersonStartMs = nowMs;
    } else if ((uint32_t)(nowMs - gameNoPersonStartMs) >= GAME_NO_PERSON_DELAY_MS) {
      tvOutputHigh = false;
    }
  } else {
    gameNoPersonStartMs = 0;
  }

  bool sofaStaticOnly = (tagCache[TAG_SOFA].staticNum > 0 && tagCache[TAG_SOFA].motionNum == 0);
  bool sofaHasMotion = (tagCache[TAG_SOFA].motionNum > 0);
  bool sofaNoPerson = ((tagCache[TAG_SOFA].staticNum + tagCache[TAG_SOFA].motionNum) == 0);
  if (sofaStaticOnly) {
    if (sofaStaticStartMs == 0) {
      sofaStaticStartMs = nowMs;
    } else if ((uint32_t)(nowMs - sofaStaticStartMs) >= SOFA_STATIC_DELAY_MS) {
      lightOutputValue = LIGHT_PWM_DIM;
    }
    sofaMotionStartMs = 0;
    sofaEmptyStartMs = 0;
  } else if (sofaHasMotion) {
    if (sofaMotionStartMs == 0) {
      sofaMotionStartMs = nowMs;
    } else if ((uint32_t)(nowMs - sofaMotionStartMs) >= SOFA_MOTION_DELAY_MS) {
      lightOutputValue = LIGHT_PWM_LOW;
    }
    sofaStaticStartMs = 0;
    sofaEmptyStartMs = 0;
  } else if (sofaNoPerson) {
    if (sofaEmptyStartMs == 0) {
      sofaEmptyStartMs = nowMs;
    } else if ((uint32_t)(nowMs - sofaEmptyStartMs) >= SOFA_EMPTY_DELAY_MS) {
      lightOutputValue = LIGHT_PWM_HIGH;
    }
    sofaStaticStartMs = 0;
    sofaMotionStartMs = 0;
  } else {
    sofaStaticStartMs = 0;
    sofaMotionStartMs = 0;
    sofaEmptyStartMs = 0;
  }

  digitalWrite(TV_CTRL_PIN, tvOutputHigh ? HIGH : LOW);
  analogWrite(LIGHT_CTRL_PIN, lightOutputValue);

  static uint32_t lastPrintMs = 0;
  if (tagPrintPending || (uint32_t)(nowMs - lastPrintMs) >= 3000) {
    tagPrintPending = false;
    lastPrintMs = nowMs;
    Serial.println(F("==================================================================="));
    Serial.println(F("Tag Cache Table"));
    Serial.println(F("Idx\tName\t\tType\t\tIO\tCenterX\tCenterY\tMotion\tStatic\tDir\tBoundary"));
    for (uint8_t i = 0; i < TAG_TOTAL; i++) {
      Serial.print(i);
      Serial.print(F("\t"));
      Serial.print(tagNames[i]);
      if (strlen(tagNames[i]) < 8) {
        Serial.print(F("\t"));
      }
      Serial.print(F("\t"));

      const char *typeText = tagTypeToText(tagCache[i].tagType);
      Serial.print(typeText);
      if (strlen(typeText) < 8) {
        Serial.print(F("\t"));
      }
      Serial.print(F("\t"));
      Serial.print(tagCache[i].ioIndex);
      Serial.print(F("\t"));
      Serial.print(tagCache[i].centerX);
      Serial.print(F("\t"));
      Serial.print(tagCache[i].centerY);
      Serial.print(F("\t"));

      if (tagCache[i].tagType == eTagPeopleCounting) {
        Serial.print(tagCache[i].motionNum);
      } else {
        Serial.print(0);
      }
      Serial.print(F("\t"));

      if (tagCache[i].tagType == eTagPeopleCounting) {
        Serial.print(tagCache[i].staticNum);
      } else {
        Serial.print(0);
      }
      Serial.print(F("\t"));

      if (tagCache[i].tagType == eTagApproachAway) {
        if (tagCache[i].motionDir == eApproachAwayDirection_t::eApproach) {
          Serial.print(F("Approach"));
        } else if (tagCache[i].motionDir == eApproachAwayDirection_t::eAway) {
          Serial.print(F("Away"));
        } else {
          Serial.print(F("None"));
        }
      } else {
        Serial.print(F("-"));
      }
      Serial.print(F("\t"));

      if (tagCache[i].tagType == eTagBoundary) {
        if (tagCache[i].enterExit == eBoundaryDirection_t::eEnter) {
          Serial.println(F("Enter"));
        } else if (tagCache[i].enterExit == eBoundaryDirection_t::eExit) {
          Serial.println(F("Exit"));
        } else {
          Serial.println(F("None"));
        }
      } else {
        Serial.println(F("-"));
      }
    }

    Serial.print(F("TV IO level:\t"));
    Serial.println(tvOutputHigh ? F("HIGH") : F("LOW"));
    Serial.print(F("Light PWM value:\t"));
    Serial.println(lightOutputValue);
  }
}
