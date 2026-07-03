/*!
 * @file test1.ino
 * @brief Test isConnected() and getHeartbeat().
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

void printHeartbeatResult(const __FlashStringHelper *modeText, eGetDataMode_t mode)
{
  bool heartbeat = c4004.getHeartbeat(mode);

  Serial.print(F("getHeartbeat("));
  Serial.print(modeText);
  Serial.print(F("): "));
  Serial.println(heartbeat ? F("OK") : F("FAIL"));
}

void runConnectionTest(void)
{
  bool connected = c4004.isConnected();

  Serial.println(F("====================ConnectionTest===================="));
  Serial.print(F("isConnected(): "));
  Serial.println(connected ? F("CONNECTED") : F("NOT CONNECTED"));

  printHeartbeatResult(F("eGetDataActive"), eGetDataActive);
  printHeartbeatResult(F("eGetDataReport"), eGetDataReport);
  Serial.println();
}

void setup()
{
  Serial.begin(115200);

  while (!c4004.begin()) {
    Serial.println(F("DFRobot C4004 begin failed, retrying..."));
    delay(1000);
  }
  Serial.println(F("DFRobot C4004 begin success."));
  Serial.println(F("Test1: isConnected / getHeartbeat"));
  Serial.println();
}

void loop()
{
  runConnectionTest();
  delay(3000);
}
