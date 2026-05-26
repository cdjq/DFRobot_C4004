/*!
 * @file setInstallInfo.ino
 * @brief Set and read DFRobot C4004 installation mode, height and angle.
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
    Serial.println("DFRobot C4004 begin failed, retrying...");
    delay(1000);
  }

  sInstallInfo_t setInstInfo = {};
  setInstInfo.mode = eInstallModeSide;
  setInstInfo.heightCm = 220;
  setInstInfo.xAngle = 0;
  setInstInfo.yAngle = 0;
  setInstInfo.zAngle = 30; // Only Z-axis angle is valid；Adjustable range: 0-90 degrees.
  Serial.println("==============Set install info:==============");
  Serial.print("Setup Mode: ");
  Serial.println(setInstInfo.mode == eInstallModeTop ? "Top" : "Side");
  Serial.print("Height(cm): ");
  Serial.println(setInstInfo.heightCm);
  Serial.print("Angle z (deg): ");
  Serial.println(setInstInfo.zAngle);

  if (c4004.setInstallInfo(setInstInfo)) {
    Serial.println("Set install info success.");
  } else {
    Serial.println("Set install info failed.");
  }
  Serial.println("==============Get install info:==============");

  delay(1000);

  sInstallInfo_t cursetInstInfo;
  if (c4004.getInstallInfo(&cursetInstInfo)) {
    Serial.print("Setup Mode: ");
    Serial.println(cursetInstInfo.mode == eInstallModeTop ? "Top" : "Side");
    Serial.print("Height(cm): ");
    Serial.println(cursetInstInfo.heightCm);
    Serial.print("Angle z (deg): ");
    Serial.println(cursetInstInfo.zAngle);
  } else {
    Serial.println("Read install info failed.");
  }
  Serial.println("=============================================");
}

void loop()
{
  delay(1000);
}
