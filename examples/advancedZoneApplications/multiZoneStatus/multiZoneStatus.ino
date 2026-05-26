/*! 
 * @file multiZoneStatus.ino
 * @brief Configure multi-zone tags and print tag event status for living-room scene linkage.
 * @details This routine configures 5 monitoring tags and 2 noise tags, keeps the last event
 * @n result for each tag, prints a summary table every 3 seconds or when a tag event arrives,
 * @n and drives IO based on game-area and sofa-area people counting results.
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

const uint32_t GAME_NO_PERSON_DELAY_MS = 60000;
const uint32_t SOFA_STATIC_DELAY_MS = 30000;
const uint32_t SOFA_MOTION_DELAY_MS = 10000;
const uint32_t SOFA_EMPTY_DELAY_MS = 30000;
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
  if (type == eTagTypeNone) {
    return "None";
  } else if (type == eTagTypeEnterExit) {
    return "EnterExit";
  } else if (type == eTagTypeApproachAway) {
    return "ApproachAway";
  } else if (type == eTagTypePeopleCounting) {
    return "PeopleCount";
  } else if (type == eTagTypeNoise) {
    return "Noise";
  }
  return "Unknown";
}

void initTagCacheFromConfig(const sTagConfig_t *tags, uint8_t count)
{
  for (uint8_t i = 0; i < TAG_TOTAL; i++) {
    memset(&tagCache[i], 0, sizeof(sTagInfo_t));
    tagCache[i].index = i;
    tagCache[i].type = eTagTypeNone;
  }

  for (uint8_t i = 0; i < count && i < TAG_TOTAL; i++) {
    uint8_t index = tags[i].index;
    if (index >= TAG_TOTAL) {
      continue;
    }
    tagCache[index].index = tags[i].index;
    tagCache[index].type = tags[i].type;
    tagCache[index].centerX = tags[i].centerX;
    tagCache[index].centerY = tags[i].centerY;
    if (index == TAG_HOME_DOOR || index == TAG_KITCHEN_DOOR) {
      tagCache[index].motionDir = 1;
    }
  }
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

  if (c4004.setPresenceEnable(true)) {
    Serial.println(F("Set presence enable success."));
  } else {
    Serial.println(F("Set presence enable failed."));
  }

  sBoundaryDetectionRange_t range;
  range.mode = eRangeFourSideBoundary;
  range.xPositiveCm = 500;
  range.xNegativeCm = -500;
  range.yPositiveCm = 800;
  range.yNegativeCm = 0;
  if (c4004.setBoundaryDetectionRange(range)) {
    Serial.println(F("Set boundary detection range success."));
  } else {
    Serial.println(F("Set boundary detection range failed."));
  }

  if (c4004.clearAllTags()) {
    Serial.println(F("Clear all tags success."));
  } else {
    Serial.println(F("Clear all tags failed."));
  }

  sTagConfig_t tags[TAG_TOTAL];

  tags[0].index = TAG_GAME;
  tags[0].type = eTagTypePeopleCounting;
  tags[0].rangeType = eTagRangeCircle;
  tags[0].centerX = 50;
  tags[0].centerY = 450;
  tags[0].xSize = 100;
  tags[0].ySize = 0;

  tags[1].index = TAG_SOFA;
  tags[1].type = eTagTypePeopleCounting;
  tags[1].rangeType = eTagRangeRectangle;
  tags[1].centerX = 300;
  tags[1].centerY = 550;
  tags[1].xSize = 100;
  tags[1].ySize = 300;

  tags[2].index = TAG_HOME_DOOR;
  tags[2].type = eTagTypeApproachAway;
  tags[2].rangeType = eTagRangeRectangle;
  tags[2].centerX = 100;
  tags[2].centerY = 700;
  tags[2].xSize = 80;
  tags[2].ySize = 40;

  tags[3].index = TAG_KITCHEN_DOOR;
  tags[3].type = eTagTypeApproachAway;
  tags[3].rangeType = eTagRangeRectangle;
  tags[3].centerX = -100;
  tags[3].centerY = 700;
  tags[3].xSize = 80;
  tags[3].ySize = 40;

  tags[4].index = TAG_DINING;
  tags[4].type = eTagTypePeopleCounting;
  tags[4].rangeType = eTagRangeRectangle;
  tags[4].centerX = 150;
  tags[4].centerY = 200;
  tags[4].xSize = 400;
  tags[4].ySize = 200;

  tags[5].index = TAG_CURTAIN;
  tags[5].type = eTagTypeNoise;
  tags[5].rangeType = eTagRangeRectangle;
  tags[5].centerX = -250;
  tags[5].centerY = 400;
  tags[5].xSize = 50;
  tags[5].ySize = 400;

  tags[6].index = TAG_PLANT;
  tags[6].type = eTagTypeNoise;
  tags[6].rangeType = eTagRangeCircle;
  tags[6].centerX = -200;
  tags[6].centerY = 650;
  tags[6].xSize = 40;
  tags[6].ySize = 0;

  if (c4004.setTagsFromConfig(tags, TAG_TOTAL)) {
    Serial.println(F("Set 7 tags from config success."));
  } else {
    Serial.println(F("Set 7 tags from config failed."));
  }

  initTagCacheFromConfig(tags, TAG_TOTAL);

  Serial.println(F("==================================================================="));
  Serial.println(F("Room occupancy inference started."));
  Serial.println(F("Rule 1: Game area has person -> TV IO HIGH immediately; no person for 60s -> LOW."));
  Serial.println(F("Rule 2: Sofa static-only for 30s -> Light PWM 150; motion for 10s -> 0; no person for 30s -> 255."));
  Serial.println(F("==================================================================="));
}

void loop()
{
  eReportedEvent_t event = c4004.getReportedInfo(100);
  uint32_t nowMs = millis();

  if (event == eEventTag) {
    sTagInfo_t tagInfo;
    if (c4004.getTagInfo(&tagInfo) && tagInfo.index < TAG_TOTAL) {
      tagCache[tagInfo.index].index = tagInfo.index;
      tagCache[tagInfo.index].type = tagInfo.type;
      tagCache[tagInfo.index].enterExit = tagInfo.enterExit;
      tagCache[tagInfo.index].motionDir = tagInfo.motionDir;
      tagCache[tagInfo.index].motionNum = tagInfo.motionNum;
      tagCache[tagInfo.index].staticNum = tagInfo.staticNum;
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
    Serial.println(F("Idx\tName\t\tType\t\tCenterX\tCenterY\tMotion\tStatic\tDir\tEnterExit"));
    for (uint8_t i = 0; i < TAG_TOTAL; i++) {
      Serial.print(i);
      Serial.print(F("\t"));
      Serial.print(tagNames[i]);
      if (strlen(tagNames[i]) < 8) {
        Serial.print(F("\t"));
      }
      Serial.print(F("\t"));

      const char *typeText = tagTypeToText(tagCache[i].type);
      Serial.print(typeText);
      if (strlen(typeText) < 8) {
        Serial.print(F("\t"));
      }
      Serial.print(F("\t"));
      Serial.print(tagCache[i].centerX);
      Serial.print(F("\t"));
      Serial.print(tagCache[i].centerY);
      Serial.print(F("\t"));

      if (tagCache[i].type == eTagTypePeopleCounting) {
        Serial.print(tagCache[i].motionNum);
      } else {
        Serial.print(0);
      }
      Serial.print(F("\t"));

      if (tagCache[i].type == eTagTypePeopleCounting) {
        Serial.print(tagCache[i].staticNum);
      } else {
        Serial.print(0);
      }
      Serial.print(F("\t"));

      if (tagCache[i].type == eTagTypeApproachAway) {
        Serial.print(tagCache[i].motionDir);
      } else {
        Serial.print(F("-"));
      }
      Serial.print(F("\t"));

      if (tagCache[i].type == eTagTypeEnterExit) {
        Serial.println(tagCache[i].enterExit);
      } else {
        Serial.println(F("-"));
      }
    }

    Serial.print(F("TV IO:\t"));
    Serial.println(tvOutputHigh ? F("HIGH") : F("LOW"));
    Serial.print(F("Light PWM:\t"));
    Serial.println(lightOutputValue);
  }
}
