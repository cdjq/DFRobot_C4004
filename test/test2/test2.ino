/*!
 * @file test2.ino
 * @brief Test setInstallInfo() and getInstallInfo().
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

const __FlashStringHelper *installModeToString(eInstallMode_t mode)
{
  if (mode == eInstallModeSide) {
    return F("Side");
  }
  if (mode == eInstallModeTop) {
    return F("Top");
  }
  return F("Unknown");
}

void printInstallInfo(const __FlashStringHelper *title, const sInstallInfo_t &info)
{
  Serial.println(title);
  Serial.print(F("Mode      : "));
  Serial.println(installModeToString(info.mode));
  Serial.print(F("Height(cm): "));
  Serial.println(info.heightCm);
  Serial.print(F("X angle   : "));
  Serial.println(info.xAngle);
  Serial.print(F("Y angle   : "));
  Serial.println(info.yAngle);
  Serial.print(F("Z angle   : "));
  Serial.println(info.zAngle);
  Serial.println();
}

void setup()
{
  sInstallInfo_t writeInfo;
  sInstallInfo_t readInfo;

  Serial.begin(115200);

  while (!c4004.begin()) {
    Serial.println(F("DFRobot C4004 begin failed, retrying..."));
    delay(1000);
  }
  Serial.println(F("DFRobot C4004 begin success."));
  Serial.println(F("Test2: setInstallInfo / getInstallInfo"));
  Serial.println();

  writeInfo.mode = eInstallModeTop;
  writeInfo.heightCm = 220;
  writeInfo.xAngle = 0;
  writeInfo.yAngle = 0;
  writeInfo.zAngle = 15;

  printInstallInfo(F("----- Write install info -----"), writeInfo);

  if (c4004.setInstallInfo(writeInfo)) {
    Serial.println(F("setInstallInfo(): SUCCESS"));
  } else {
    Serial.println(F("setInstallInfo(): FAILED"));
  }
  delay(100);

  memset(&readInfo, 0, sizeof(readInfo));
  if (c4004.getInstallInfo(&readInfo)) {
    Serial.println(F("getInstallInfo(): SUCCESS"));
    printInstallInfo(F("----- Read install info -----"), readInfo);

    if (readInfo.mode == writeInfo.mode &&
        readInfo.heightCm == writeInfo.heightCm &&
        readInfo.xAngle == writeInfo.xAngle &&
        readInfo.yAngle == writeInfo.yAngle &&
        readInfo.zAngle == writeInfo.zAngle) {
      Serial.println(F("Install info verify: PASS"));
    } else {
      Serial.println(F("Install info verify: FAIL"));
    }
  } else {
    Serial.println(F("getInstallInfo(): FAILED"));
  }
}

void loop()
{
}
