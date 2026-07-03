/*!
 * @file factoryResetAndReboot.ino
 * @brief Restore factory settings and reboot the module.
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

  

  Serial.print(F("Hardware version: "));
  Serial.println(c4004.getHardwareVersion());
  Serial.print(F("Firmware version: "));
  Serial.println(c4004.getFirmwareVersion());

  Serial.println(F("Module factory resetting..."));
  if (c4004.factoryReset()) {
    Serial.println(F("Factory reset success."));
  } else {
    Serial.println(F("Factory reset failed."));
  }

  delay(1000);

  Serial.println(F("Module rebooting ..."));
  if (c4004.reset()) {
    Serial.println(F("Reboot success."));
  } else {
    Serial.println(F("Reboot failed."));
  }
}

void loop()
{
  delay(1000);
}
