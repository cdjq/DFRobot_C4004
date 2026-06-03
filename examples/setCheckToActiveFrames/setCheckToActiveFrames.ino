/*!
 * @file setCheckToActiveFrames.ino
 * @brief Set and read back the check-to-active frame confirmation count.
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

  if (c4004.setCheckToActiveFrames(7)) {
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
}

void loop()
{
  delay(1000);
}
