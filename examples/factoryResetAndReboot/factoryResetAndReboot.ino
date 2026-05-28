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
    Serial.println("DFRobot C4004 begin failed, retrying...");
    delay(1000);
  }

  Serial.print("Product model: ");
  Serial.println(c4004.getProductModel());
  Serial.print("Hardware version: ");
  Serial.println(c4004.getHardwareVersion());
  Serial.print("Firmware version: ");
  Serial.println(c4004.getFirmwareVersion());

  Serial.println("Factory reset...");
  if (c4004.factoryReset()) {
    Serial.println("Factory reset success.");
  } else {
    Serial.println("Factory reset failed.");
  }

  Serial.println("Reboot module...");
  if (c4004.reset()) {
    Serial.println("Reboot command sent.");
  } else {
    Serial.println("Reboot command failed.");
  }
}

void loop()
{
  delay(1000);
}
