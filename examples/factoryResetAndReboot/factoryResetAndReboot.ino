/*!
 * @file factoryResetAndReboot.ino
 * @brief Factory reset and reboot demo.
 * @details Use this example when you need to restore the module to factory settings and reboot it
 * @n (for example after a wrong configuration, or before starting a clean setup).
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
