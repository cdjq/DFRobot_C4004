/*!
 * @file DFRobot_C4004.cpp
 * @brief Implementation of the DFRobot C4004 sensor driver.
 * @copyright Copyright (c) 2026 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @license The MIT License (MIT)
 * @author JiaLi(zhixin.liu@dfrobot.com)
 * @version V1.0.0
 * @date 2026-05-22
 * @url https://github.com/DFRobot/DFRobot_C4004
 */

#include "DFRobot_C4004.h"

#ifdef ENABLE_DBG
static void appendHexByte(String &text, uint8_t value)
{
  if (value < 0x10) {
    text += "0";
  }
  text += String(value, HEX);
}
#endif

#if defined(ESP8266) || defined(ARDUINO_AVR_UNO)
DFRobot_C4004::DFRobot_C4004(SoftwareSerial *sSerial, uint32_t baud)
{
  _serial = sSerial;
  _s = sSerial;
  _baud = baud;
  _rxpin = 0;
  _txpin = 0;
  initObject();
}
#else
DFRobot_C4004::DFRobot_C4004(HardwareSerial *hSerial, uint32_t baud, uint8_t rxpin, uint8_t txpin)
{
  _serial = hSerial;
  _s = hSerial;
  _baud = baud;
  _rxpin = rxpin;
  _txpin = txpin;
  initObject();
}
#endif

void DFRobot_C4004::initObject(void)
{
  _timeoutMs = DEFAULT_TIMEOUT;
  _lastHeartbeatMs = 0;
  _heartbeat = false;
  _initFinished = false;
  _presenceEnable = 0xFF;
  _presence = ePresenceUnknown;
  _motionState = eMotionUnknown;
  _trajectoryLed = 0xFF;
  _motionLed = 0xFF;
  _targetCount = 0;
  _tagCount = 0;
  memset(_targets, 0, sizeof(_targets));
  memset(_tags, 0, sizeof(_tags));
  memset(&_tagInfo, 0, sizeof(_tagInfo));
  _tagInfoValid = false;
  memset(&_rangeInfo, 0, sizeof(_rangeInfo));
  _peopleCount = 0;
  _rangeInfo.mode = eRangeUnknown;
}

bool DFRobot_C4004::begin(void)
{
  uint32_t startTime = millis();

  if (_s == NULL) {
    return false;
  }

#if defined(ESP32)
  _serial->begin(_baud, SERIAL_8N1, _rxpin, _txpin);
#else
  _serial->begin(_baud);
#endif

  delay(50);
  while ((uint32_t)(millis() - startTime) < 1200UL) {
    if (isInitFinished()) {
      return true;
    }
    delay(20);
  }

  return isConnected();
}

bool DFRobot_C4004::isInitFinished(void)
{
  uint8_t data = QUERY_DATA;
  sPacket_t packet;

  if (requestFrame(CTRL_WORK_STATUS, CMD_WORK_STATUS_INIT_FINISHED_QUERY, &data, 1, &packet)) {
    if (packet.len > 0) {
      _initFinished = (packet.data[0] == 0x01);
    }
  }
  return _initFinished;
}

bool DFRobot_C4004::isConnected(void)
{
  if (getHeartbeat()) {
    return true;
  }

  if (_lastHeartbeatMs == 0) {
    return false;
  }
  return ((uint32_t)(millis() - _lastHeartbeatMs) < HEARTBEAT_TIMEOUT);
}

bool DFRobot_C4004::reset(void)
{
  uint8_t data = QUERY_DATA;
  sPacket_t packet;
  bool ret = requestFrame(CTRL_SYSTEM, CMD_SYSTEM_RESET, &data, 1, &packet);
  delay(100);
  return ret;
}

bool DFRobot_C4004::factoryReset(void)
{
  uint8_t data = QUERY_DATA;
  sPacket_t packet;
  bool ret = requestFrame(CTRL_SYSTEM, CMD_SYSTEM_FACTORY_RESET, &data, 1, &packet);
  delay(100);
  return ret;
}

bool DFRobot_C4004::getHeartbeat(eGetDataMode_t mode)
{
  uint8_t data = QUERY_DATA;
  sPacket_t packet;

  if (mode == eGetDataReport) {
    return _heartbeat;
  }

  if (!requestFrame(CTRL_SYSTEM, CMD_SYSTEM_HEARTBEAT_QUERY, &data, 1, &packet)) {
    return false;
  }
  if (packet.len > 0 && packet.data[0] != QUERY_DATA) {
    return false;
  }
  _lastHeartbeatMs = millis();
  _heartbeat = true;
  return true;
}

eReportedEvent_t DFRobot_C4004::getReportedInfo(uint16_t timeoutMs)
{
  sPacket_t packet;

  if (!readFrame(&packet, timeoutMs)) {
    return eEventNone;
  }
  return handlePacket(&packet);
}

String DFRobot_C4004::getProductModel(void)
{
  return queryString(CTRL_PRODUCT_INFO, CMD_PRODUCT_MODEL_QUERY);
}

uint16_t DFRobot_C4004::getProductID(void)
{
  uint8_t data = QUERY_DATA;
  sPacket_t packet;

  if (!requestFrame(CTRL_PRODUCT_INFO, CMD_PRODUCT_ID_QUERY, &data, 1, &packet)) {
    return 0;
  }
  if (packet.len >= 2) {
    return readUint16(packet.data);
  }
  if (packet.len == 1) {
    return packet.data[0];
  }
  return 0;
}

String DFRobot_C4004::getHardwareVersion(void)
{
  return queryString(CTRL_PRODUCT_INFO, CMD_PRODUCT_HARDWARE_VERSION_QUERY);
}

String DFRobot_C4004::getFirmwareVersion(void)
{
  return queryString(CTRL_PRODUCT_INFO, CMD_PRODUCT_FIRMWARE_VERSION_QUERY);
}

bool DFRobot_C4004::setInstallInfo(sInstallInfo_t &info)
{
  uint8_t angleData[6];
  uint8_t heightData[2];
  uint8_t modeData = (uint8_t)info.mode;
  sPacket_t packet;
  int32_t xAngleProto = (int32_t)info.xAngle * 100;
  int32_t yAngleProto = (int32_t)info.yAngle * 100;
  int32_t zAngleProto = (int32_t)info.zAngle * 100;

  if (xAngleProto > 18000) {
    xAngleProto = 18000;
  } else if (xAngleProto < -18000) {
    xAngleProto = -18000;
  }
  if (yAngleProto > 18000) {
    yAngleProto = 18000;
  } else if (yAngleProto < -18000) {
    yAngleProto = -18000;
  }
  if (zAngleProto > 18000) {
    zAngleProto = 18000;
  } else if (zAngleProto < -18000) {
    zAngleProto = -18000;
  }

  writeInt16(&angleData[0], (int16_t)xAngleProto);
  writeInt16(&angleData[2], (int16_t)yAngleProto);
  writeInt16(&angleData[4], (int16_t)zAngleProto);
  writeUint16(heightData, info.heightCm);

  if (!requestFrame(CTRL_INSTALL_INFO, CMD_INSTALL_SET_MODE, &modeData, 1, &packet)) {
    return false;
  }
  if (!requestFrame(CTRL_INSTALL_INFO, CMD_INSTALL_SET_ANGLE, angleData, sizeof(angleData), &packet)) {
    return false;
  }
  if (!requestFrame(CTRL_INSTALL_INFO, CMD_INSTALL_SET_HEIGHT, heightData, sizeof(heightData), &packet)) {
    return false;
  }
  return true;
}

bool DFRobot_C4004::getInstallInfo(sInstallInfo_t *info)
{
  uint8_t data = QUERY_DATA;
  sPacket_t packet;

  if (info == NULL) {
    return false;
  }
  memset(info, 0, sizeof(sInstallInfo_t));
  info->mode = eInstallModeUnknown;

  if (!requestFrame(CTRL_INSTALL_INFO, CMD_INSTALL_QUERY_ANGLE, &data, 1, &packet) || packet.len < 6) {
    return false;
  }
  info->xAngle = readInt16(&packet.data[0]) / 100;
  info->yAngle = readInt16(&packet.data[2]) / 100;
  info->zAngle = readInt16(&packet.data[4]) / 100;

  if (!requestFrame(CTRL_INSTALL_INFO, CMD_INSTALL_QUERY_HEIGHT, &data, 1, &packet) || packet.len < 2) {
    return false;
  }
  info->heightCm = readUint16(packet.data);

  if (!requestFrame(CTRL_INSTALL_INFO, CMD_INSTALL_QUERY_MODE, &data, 1, &packet) || packet.len < 1) {
    return false;
  }
  info->mode = (eInstallMode_t)packet.data[0];
  return true;
}

bool DFRobot_C4004::setInstallHigh(int32_t hight)
{
  uint8_t heightData[2];
  sPacket_t packet;

  if (hight < 0 || hight > 0xFFFFL) {
    return false;
  }

  writeUint16(heightData, (uint16_t)hight);
  return requestFrame(CTRL_INSTALL_INFO, CMD_INSTALL_SET_HEIGHT, heightData, sizeof(heightData), &packet);
}

bool DFRobot_C4004::getInstallHigh(int *hight)
{
  uint8_t data = QUERY_DATA;
  sPacket_t packet;

  if (hight == NULL) {
    return false;
  }

  if (!requestFrame(CTRL_INSTALL_INFO, CMD_INSTALL_QUERY_HEIGHT, &data, 1, &packet) || packet.len < 2) {
    return false;
  }

  *hight = (int)readUint16(packet.data);
  return true;
}

bool DFRobot_C4004::setPresenceEnable(bool enable)
{
  if (setByte(CTRL_PRESENCE, CMD_PRESENCE_SET_ENABLE, enable ? 1 : 0)) {
    _presenceEnable = enable ? 1 : 0;
    return true;
  }
  return false;
}

bool DFRobot_C4004::getPresenceEnable(bool *enable)
{
  uint8_t value = 0;

  if (enable == NULL) {
    return false;
  }
  if (!queryByte(CTRL_PRESENCE, CMD_PRESENCE_QUERY_ENABLE, &value)) {
    return false;
  }
  _presenceEnable = value;
  *enable = (value != 0);
  return true;
}

ePresenceState_t DFRobot_C4004::getPresenceState(void)
{
  uint8_t value = ePresenceUnknown;
  if (queryByte(CTRL_PRESENCE, CMD_PRESENCE_QUERY_STATE, &value)) {
    _presence = (ePresenceState_t)value;
  }
  return _presence;
}

eMotionState_t DFRobot_C4004::getMotionState(void)
{
  uint8_t value = eMotionUnknown;
  if (queryByte(CTRL_PRESENCE, CMD_PRESENCE_QUERY_MOTION, &value)) {
    _motionState = (eMotionState_t)value;
  }
  return _motionState;
}

bool DFRobot_C4004::setTrajectoryTrackEnable(bool enable)
{
  return setByte(CTRL_TRAJECTORY, CMD_TRAJECTORY_SET_ENABLE, enable ? 1 : 0);
}

bool DFRobot_C4004::getTrajectoryTrackEnable(bool *enable)
{
  uint8_t value = 0;

  if (enable == NULL) {
    return false;
  }
  if (!queryByte(CTRL_TRAJECTORY, CMD_TRAJECTORY_QUERY_ENABLE, &value)) {
    return false;
  }
  *enable = (value != 0);
  return true;
}

bool DFRobot_C4004::getTargetInfo(uint8_t index, sTargetInfo_t *targetInfo, eGetDataMode_t mode)
{
  uint8_t data = QUERY_DATA;
  sPacket_t packet;

  if (targetInfo == NULL) {
    return false;
  }
  if (mode == eGetDataActive) {
    requestFrame(CTRL_TRAJECTORY, CMD_TRAJECTORY_QUERY_TARGET, &data, 1, &packet);
  }

  for (uint8_t i = 0; i < _targetCount; i++) {
    if (_targets[i].index == index) {
      *targetInfo = _targets[i];
      return true;
    }
  }

  if (index < _targetCount) {
    *targetInfo = _targets[index];
    return true;
  }
  return false;
}

uint8_t DFRobot_C4004::getTargetList(sTargetInfo_t *targetBuf, uint8_t maxCount, eGetDataMode_t mode)
{
  uint8_t data = QUERY_DATA;
  sPacket_t packet;
  uint8_t copyCount = 0;

  if (mode == eGetDataActive) {
    requestFrame(CTRL_TRAJECTORY, CMD_TRAJECTORY_QUERY_TARGET, &data, 1, &packet);
  }

  if (targetBuf == NULL || maxCount == 0) {
    return _targetCount;
  }

  copyCount = _targetCount;
  if (copyCount > maxCount) {
    copyCount = maxCount;
  }
  for (uint8_t i = 0; i < copyCount; i++) {
    targetBuf[i] = _targets[i];
  }
  return copyCount;
}

uint8_t DFRobot_C4004::getTargetCount(void)
{
  return _targetCount;
}

bool DFRobot_C4004::setTrajectoryLed(bool enable)
{
  if (setByte(CTRL_TRAJECTORY, CMD_TRAJECTORY_SET_TRAJECTORY_LED, enable ? 1 : 0)) {
    _trajectoryLed = enable ? 1 : 0;
    return true;
  }
  return false;
}

bool DFRobot_C4004::setMotionLed(bool enable)
{
  if (setByte(CTRL_TRAJECTORY, CMD_TRAJECTORY_SET_MOTION_LED, enable ? 1 : 0)) {
    _motionLed = enable ? 1 : 0;
    return true;
  }
  return false;
}

bool DFRobot_C4004::getTrajectoryLed(void)
{
  uint8_t value = 0;
  if (queryByte(CTRL_TRAJECTORY, CMD_TRAJECTORY_QUERY_TRAJECTORY_LED, &value)) {
    _trajectoryLed = value;
  }
  return (_trajectoryLed != 0);
}

bool DFRobot_C4004::getMotionLed(void)
{
  uint8_t value = 0;
  if (queryByte(CTRL_TRAJECTORY, CMD_TRAJECTORY_QUERY_MOTION_LED, &value)) {
    _motionLed = value;
  }
  return (_motionLed != 0);
}

uint8_t DFRobot_C4004::getTags(sTagConfig_t *tags, uint8_t maxTags, eGetDataMode_t mode)
{
  uint8_t data = QUERY_DATA;
  sPacket_t packet;
  uint8_t copyCount = 0;

  if (mode == eGetDataActive) {
    if (!requestFrame(CTRL_DETECTION_RANGE, CMD_DETECTION_RANGE_QUERY_TAGS, &data, 1, &packet)) {
      return 0;
    }
  }

  copyCount = _tagCount;
  if (copyCount > maxTags) {
    copyCount = maxTags;
  }
  if (tags != NULL) {
    for (uint8_t i = 0; i < copyCount; i++) {
      tags[i] = _tags[i];
    }
  }
  return copyCount;
}

bool DFRobot_C4004::getTagInfo(sTagInfo_t *tagInfo)
{
  if (tagInfo == NULL) {
    return false;
  }
  if (!_tagInfoValid) {
    return false;
  }
  *tagInfo = _tagInfo;
  return true;
}

eTagSetStatus_t DFRobot_C4004::setTag(const sTagConfig_t &tag)
{
  uint8_t data[7];
  sPacket_t packet;

  data[0] = tag.tagIndex;
  data[1] = (uint8_t)tag.tagType;
  data[2] = (uint8_t)tag.scopeType;
  writeUint16(&data[3], tag.width);
  writeUint16(&data[5], tag.height);

  if (!requestFrame(CTRL_DETECTION_RANGE, CMD_DETECTION_RANGE_SET_TAG, data, sizeof(data), &packet)) {
    return eTagSetCommError;
  }
  if (packet.len < 4) {
    return eTagSetCommError;
  }
  if (packet.data[0] != tag.tagIndex) {
    return eTagSetCommError;
  }

  if (packet.data[3] >= eTagSetSuccess && packet.data[3] <= eTagSetIndexOutOfRange) {
    return (eTagSetStatus_t)packet.data[3];
  }
  return eTagSetCommError;
}

bool DFRobot_C4004::clearTag(uint16_t tagIndex)
{
  uint8_t data[2];
  sPacket_t packet;
  uint16_t respIndex = 0;

  writeUint16(data, tagIndex);
  if (!requestFrame(CTRL_DETECTION_RANGE, CMD_DETECTION_RANGE_CLEAR_TAG, data, sizeof(data), &packet)) {
    return false;
  }
  if (packet.len > 0 && packet.data[0] == 0xFE) {
    return false;
  }
  if (packet.len < 2) {
    return false;
  }
  respIndex = readUint16(packet.data);
  return (respIndex == tagIndex);
}

bool DFRobot_C4004::clearAllTags(void)
{
  uint8_t clearAll = 0xFF;
  sPacket_t packet;

  if (!requestFrame(CTRL_DETECTION_RANGE, CMD_DETECTION_RANGE_CLEAR_TAG, &clearAll, 1, &packet)) {
    return false;
  }
  if (packet.len > 0) {
    if (packet.data[0] == 0xFE) {
      return false;
    }
    if (packet.data[0] != 0xFF) {
      return false;
    }
  }
  return true;
}

bool DFRobot_C4004::setTagsFromConfig(const sTagConfig_t *tags, uint8_t tagCount)
{
  uint8_t data[MAX_PAYLOAD];
  uint16_t offset = 0;
  sPacket_t packet;

  if (tags == NULL && tagCount > 0) {
    return false;
  }
  if (tagCount > MAX_TAGS) {
    tagCount = MAX_TAGS;
  }

  writeUint16(&data[offset], tagCount);
  offset += 2;
  for (uint8_t i = 0; i < tagCount; i++) {
    data[offset++] = tags[i].tagIndex;
    data[offset++] = (uint8_t)tags[i].tagType;
    data[offset++] = (uint8_t)tags[i].scopeType;
    writeSignBitInt16(&data[offset], tags[i].centerX);
    offset += 2;
    writeSignBitInt16(&data[offset], tags[i].centerY);
    offset += 2;
    writeUint16(&data[offset], tags[i].width);
    offset += 2;
    writeUint16(&data[offset], tags[i].height);
    offset += 2;
  }
  return requestFrame(CTRL_DETECTION_RANGE, CMD_DETECTION_RANGE_SET_TAGS_FROM_CONFIG, data, offset, &packet);
}

bool DFRobot_C4004::setFourSidedRangeMode(sFourSidedRange &range)
{
  uint8_t data[9];
  sPacket_t packet;

  data[0] = eRangeFourSide;
  writeSignBitInt16(&data[1], range.xPositiveCm);
  writeSignBitInt16(&data[3], range.xNegativeCm);
  writeSignBitInt16(&data[5], range.yPositiveCm);
  writeSignBitInt16(&data[7], range.yNegativeCm);

  if (!requestFrame(CTRL_DETECTION_RANGE, CMD_DETECTION_RANGE_SET_RANGE, data, sizeof(data), &packet)) {
    return false;
  }
  _rangeInfo = range;
  _rangeInfo.mode = eRangeFourSide;
  return true;
}

bool DFRobot_C4004::getFourSidedRangeMode(sFourSidedRange *range)
{
  uint8_t data = QUERY_DATA;
  sPacket_t packet;

  if (range == NULL) {
    return false;
  }
  if (!requestFrame(CTRL_DETECTION_RANGE, CMD_DETECTION_RANGE_QUERY_RANGE, &data, 1, &packet)) {
    return false;
  }
  *range = _rangeInfo;
  return true;
}

bool DFRobot_C4004::setTrajectoryRangeMode(bool learning)
{
  uint8_t data[2];
  sPacket_t packet;
  uint32_t startTime = 0;

  data[0] = eRangeTrajectory;
  data[1] = learning ? 1 : 0;

  flushInput();
  if (!sendCommand(CTRL_DETECTION_RANGE, CMD_DETECTION_RANGE_SET_RANGE, data, sizeof(data))) {
    return false;
  }

  startTime = millis();
  while ((uint32_t)(millis() - startTime) < DEFAULT_TIMEOUT) {
    uint16_t elapsed = (uint16_t)(millis() - startTime);
    uint16_t leftTime = DEFAULT_TIMEOUT - elapsed;

    if (!readFrame(&packet, leftTime)) {
      continue;
    }
    handlePacket(&packet);

    if (packet.control != CTRL_DETECTION_RANGE) {
      continue;
    }
    if (packet.cmd == CMD_DETECTION_RANGE_SET_RANGE) {
      _rangeInfo.mode = eRangeTrajectory;
      return true;
    }
    if (packet.cmd == CMD_DETECTION_RANGE_QUERY_RANGE && packet.len > 0 && (eDetectionRangeMode_t)packet.data[0] == eRangeTrajectory) {
      _rangeInfo.mode = eRangeTrajectory;
      return true;
    }
  }
  return false;
}

bool DFRobot_C4004::setConfigFileModePoints(const sPoint_t *points, uint16_t pointCount)
{
  uint8_t data[3 + MAX_POINTS * 4];
  uint16_t offset = 0;
  uint16_t respCount = 0;
  sPacket_t packet;

  if (points == NULL && pointCount > 0) {
    return false;
  }
  if (pointCount > MAX_POINTS) {
    pointCount = MAX_POINTS;
  }

  data[offset++] = eRangeConfigFile;
  writeUint16(&data[offset], pointCount);
  offset += 2;

  for (uint16_t i = 0; i < pointCount; i++) {
    writeSignBitInt16(&data[offset], points[i].x);
    offset += 2;
    writeSignBitInt16(&data[offset], points[i].y);
    offset += 2;
  }

  if (!requestFrame(CTRL_DETECTION_RANGE, CMD_DETECTION_RANGE_SET_RANGE, data, offset, &packet)) {
    return false;
  }
  if (packet.len < 3 || packet.data[0] != (uint8_t)eRangeConfigFile) {
    return false;
  }

  respCount = readUint16(&packet.data[1]);
  if (respCount != pointCount || packet.len < (uint16_t)(3 + respCount * 4)) {
    return false;
  }

  _rangeInfo.mode = eRangeConfigFile;
  return true;
}

bool DFRobot_C4004::getTrajectoryRangeMode(sPoint_t *points, uint16_t *pointCount)
{
  uint8_t data = QUERY_DATA;
  sPacket_t packet;
  uint16_t count = 0;

  if (points == NULL || pointCount == NULL) {
    return false;
  }
  *pointCount = 0;

  if (!requestFrame(CTRL_DETECTION_RANGE, CMD_DETECTION_RANGE_QUERY_RANGE, &data, 1, &packet)) {
    return false;
  }
  if (packet.len < 3) {
    return false;
  }
  if ((eDetectionRangeMode_t)packet.data[0] != eRangeTrajectory) {
    return false;
  }

  count = readUint16(&packet.data[1]);
  if (count > MAX_POINTS) {
    return false;
  }
  if (packet.len < (uint16_t)(3 + count * 4)) {
    return false;
  }

  for (uint16_t i = 0; i < count; i++) {
    uint16_t offset = (uint16_t)(3 + i * 4);
    points[i].x = readSignBitInt16(&packet.data[offset]);
    points[i].y = readSignBitInt16(&packet.data[offset + 2]);
  }
  *pointCount = count;
  return true;
}

bool DFRobot_C4004::getConfigFileModePoints(sPoint_t *points, uint16_t *pointCount)
{
  uint8_t data = QUERY_DATA;
  sPacket_t packet;
  uint16_t count = 0;

  if (points == NULL || pointCount == NULL) {
    return false;
  }
  *pointCount = 0;

  if (!requestFrame(CTRL_DETECTION_RANGE, CMD_DETECTION_RANGE_QUERY_RANGE, &data, 1, &packet)) {
    return false;
  }
  if (packet.len < 3) {
    return false;
  }
  if ((eDetectionRangeMode_t)packet.data[0] != eRangeConfigFile) {
    return false;
  }

  count = readUint16(&packet.data[1]);
  if (count > MAX_POINTS) {
    return false;
  }
  if (packet.len < (uint16_t)(3 + count * 4)) {
    return false;
  }

  for (uint16_t i = 0; i < count; i++) {
    uint16_t offset = (uint16_t)(3 + i * 4);
    points[i].x = readSignBitInt16(&packet.data[offset]);
    points[i].y = readSignBitInt16(&packet.data[offset + 2]);
  }
  *pointCount = count;
  return true;
}

eDetectionRangeMode_t DFRobot_C4004::getDetectionRangeMode(void)
{
  uint8_t data = QUERY_DATA;
  sPacket_t packet;

  if (!requestFrame(CTRL_DETECTION_RANGE, CMD_DETECTION_RANGE_QUERY_RANGE, &data, 1, &packet)) {
    return _rangeInfo.mode;
  }
  return _rangeInfo.mode;
}

uint8_t DFRobot_C4004::getPeopleTime(eGetDataMode_t mode)
{
  uint8_t data = QUERY_DATA;
  sPacket_t packet;

  if (mode == eGetDataActive) {
    requestFrame(CTRL_PEOPLE_COUNT, CMD_PEOPLE_COUNT_QUERY_COUNT, &data, 1, &packet);
  }
  return _peopleCount;
}

bool DFRobot_C4004::setRealTimePeopleTime(uint32_t time)
{
  return setUint32(CTRL_PEOPLE_COUNT, CMD_PEOPLE_COUNT_SET_REPORT_INTERVAL, time);
}

bool DFRobot_C4004::getRealTimePeopleTime(uint32_t *time)
{
  return queryUint32(CTRL_PEOPLE_COUNT, CMD_PEOPLE_COUNT_QUERY_REPORT_INTERVAL, time);
}

bool DFRobot_C4004::clearPeopleCount(void)
{
  uint8_t data = QUERY_DATA;
  sPacket_t packet;
  return requestFrame(CTRL_PEOPLE_COUNT, CMD_PEOPLE_COUNT_CLEAR_COUNT, &data, 1, &packet);
}

bool DFRobot_C4004::setTrackMeters(uint32_t distanceCm)
{
  return setUint32(CTRL_PEOPLE_COUNT, CMD_PEOPLE_COUNT_SET_TRAJECTORY_DISTANCE, distanceCm);
}

bool DFRobot_C4004::getTrackMeters(uint32_t *distanceCm)
{
  return queryUint32(CTRL_PEOPLE_COUNT, CMD_PEOPLE_COUNT_QUERY_TRAJECTORY_DISTANCE, distanceCm);
}

bool DFRobot_C4004::setTrackExistsTime(uint32_t time)
{
  return setUint32(CTRL_PEOPLE_COUNT, CMD_PEOPLE_COUNT_SET_TRAJECTORY_HOLD_TIME, time);
}

bool DFRobot_C4004::getTrackExistsTime(uint32_t *time)
{
  return queryUint32(CTRL_PEOPLE_COUNT, CMD_PEOPLE_COUNT_QUERY_TRAJECTORY_HOLD_TIME, time);
}

bool DFRobot_C4004::setUnmannedTime(uint32_t delayTime)
{
  return setUint32(CTRL_PEOPLE_COUNT, CMD_PEOPLE_COUNT_SET_NO_PERSON_DELAY, delayTime);
}

bool DFRobot_C4004::getUnmannedTime(uint32_t *delayTime)
{
  return queryUint32(CTRL_PEOPLE_COUNT, CMD_PEOPLE_COUNT_QUERY_NO_PERSON_DELAY, delayTime);
}

void DFRobot_C4004::setTimeout(uint16_t timeoutMs)
{
  _timeoutMs = timeoutMs;
}

bool DFRobot_C4004::sendCommand(uint8_t control, uint8_t cmd, const uint8_t *data, uint16_t len)
{
  uint8_t checksum = 0;
  uint8_t value = 0;

  if (_s == NULL || len > MAX_PAYLOAD) {
    return false;
  }

  value = FRAME_HEAD1;
  checksum += value;
  _s->write(value);
  value = FRAME_HEAD2;
  checksum += value;
  _s->write(value);
  checksum += control;
  _s->write(control);
  checksum += cmd;
  _s->write(cmd);
  value = (uint8_t)(len >> 8);
  checksum += value;
  _s->write(value);
  value = (uint8_t)(len & 0xFF);
  checksum += value;
  _s->write(value);

  for (uint16_t i = 0; i < len; i++) {
    value = (data == NULL) ? 0 : data[i];
    checksum += value;
    _s->write(value);
  }

  _s->write(checksum);
  _s->write((uint8_t)FRAME_TAIL1);
  _s->write((uint8_t)FRAME_TAIL2);

#ifdef ENABLE_DBG
  {
    String meta = "TX ctrl=0x";
    appendHexByte(meta, control);
    meta += " cmd=0x";
    appendHexByte(meta, cmd);
    meta += " len=";
    meta += String(len);
    meta += " checksum=0x";
    appendHexByte(meta, checksum);
    DBG(meta);

    if (len > 0) {
      const uint16_t dumpLen = (len > 24) ? 24 : len;
      String dataLog = "TX data: ";
      for (uint16_t i = 0; i < dumpLen; i++) {
        appendHexByte(dataLog, (data == NULL) ? 0 : data[i]);
        if (i + 1 < dumpLen) {
          dataLog += " ";
        }
      }
      DBG(dataLog);
      if (len > dumpLen) {
        DBG(String("TX data truncated, total len=") + String(len));
      }
    } else {
      DBG("TX data: (none)");
    }
  }
#endif
  return true;
}

bool DFRobot_C4004::requestFrame(uint8_t control, uint8_t cmd, const uint8_t *data, uint16_t len, sPacket_t *response, uint16_t timeoutMs)
{
  uint32_t startTime = 0;
  eReportedEvent_t event = eEventNone;

  if (response == NULL) {
    return false;
  }

  flushInput();
  if (!sendCommand(control, cmd, data, len)) {
    return false;
  }
  startTime = millis();

  while ((uint32_t)(millis() - startTime) < timeoutMs) {
    uint16_t elapsed = (uint16_t)(millis() - startTime);
    if (elapsed >= timeoutMs) {
      break;
    }
    uint16_t leftTime = timeoutMs - elapsed;
    if (!readFrame(response, leftTime)) {
      continue;
    }
    event = handlePacket(response);
    if (response->control == control && response->cmd == cmd) {
      return true;
    }
    (void)event;
  }
  return false;
}

bool DFRobot_C4004::readFrame(sPacket_t *packet, uint16_t timeoutMs)
{
  uint32_t startTime = millis();
  uint8_t value = 0;
  uint8_t checksum = 0;
  uint8_t recvChecksum = 0;
  uint8_t tail1 = 0;
  uint8_t tail2 = 0;

  if (packet == NULL || _s == NULL) {
    return false;
  }

  while ((uint32_t)(millis() - startTime) < timeoutMs) {
    if (!readByte(&value, 1)) {
      continue;
    }
    if (value != FRAME_HEAD1) {
      continue;
    }
    checksum = value;

    if (!readByte(&value, timeoutMs)) {
      return false;
    }
    if (value != FRAME_HEAD2) {
      continue;
    }
    checksum += value;

    if (!readByte(&packet->control, timeoutMs)) {
      return false;
    }
    checksum += packet->control;
    if (!readByte(&packet->cmd, timeoutMs)) {
      return false;
    }
    checksum += packet->cmd;
    if (!readByte(&value, timeoutMs)) {
      return false;
    }
    checksum += value;
    packet->len = ((uint16_t)value << 8);
    if (!readByte(&value, timeoutMs)) {
      return false;
    }
    checksum += value;
    packet->len |= value;

    if (packet->len > MAX_PAYLOAD) {
#ifdef ENABLE_DBG
      DBG(String("payload too long, len=") + String(packet->len) + String(" max=") + String(MAX_PAYLOAD));
#endif
      flushInput();
      return false;
    }

    for (uint16_t i = 0; i < packet->len; i++) {
      if (!readByte(&packet->data[i], timeoutMs)) {
        return false;
      }
      checksum += packet->data[i];
    }

    if (!readByte(&recvChecksum, timeoutMs)) {
      return false;
    }
    if (!readByte(&tail1, timeoutMs) || !readByte(&tail2, timeoutMs)) {
      return false;
    }
    if (tail1 != FRAME_TAIL1 || tail2 != FRAME_TAIL2) {
      return false;
    }
    if (checksum != recvChecksum) {
      DBG("checksum error");
      return false;
    }

#ifdef ENABLE_DBG
    {
      String meta = "RX ctrl=0x";
      appendHexByte(meta, packet->control);
      meta += " cmd=0x";
      appendHexByte(meta, packet->cmd);
      meta += " len=";
      meta += String(packet->len);
      meta += " checksum=0x";
      appendHexByte(meta, recvChecksum);
      DBG(meta);

      if (packet->len > 0) {
        const uint16_t dumpLen = (packet->len > 24) ? 24 : packet->len;
        String dataLog = "RX data: ";
        for (uint16_t i = 0; i < dumpLen; i++) {
          appendHexByte(dataLog, packet->data[i]);
          if (i + 1 < dumpLen) {
            dataLog += " ";
          }
        }
        DBG(dataLog);
        if (packet->len > dumpLen) {
          DBG(String("RX data truncated, total len=") + String(packet->len));
        }
      }
    }
#endif
    return true;
  }
  return false;
}

bool DFRobot_C4004::readByte(uint8_t *value, uint16_t timeoutMs)
{
  uint32_t startTime = millis();

  if (value == NULL || _s == NULL) {
    return false;
  }

  do {
    if (_s->available() > 0) {
      *value = (uint8_t)_s->read();
      return true;
    }
  } while ((uint32_t)(millis() - startTime) < timeoutMs);

  return false;
}

void DFRobot_C4004::flushInput(void)
{
  if (_s == NULL) {
    return;
  }
  while (_s->available() > 0) {
    _s->read();
  }
}

eReportedEvent_t DFRobot_C4004::handlePacket(const sPacket_t *packet)
{
  eReportedEvent_t event = eEventUnknown;

  if (packet == NULL) {
    return eEventError;
  }

  if ((packet->control == CTRL_SYSTEM && packet->cmd == CMD_SYSTEM_HEARTBEAT_REPORT) || (packet->control == CTRL_SYSTEM && packet->cmd == CMD_SYSTEM_HEARTBEAT_QUERY)) {
    _lastHeartbeatMs = millis();
    _heartbeat = true;
  } else if ((packet->control == CTRL_WORK_STATUS && packet->cmd == CMD_WORK_STATUS_INIT_FINISHED_REPORT) || (packet->control == CTRL_WORK_STATUS && packet->cmd == CMD_WORK_STATUS_INIT_FINISHED_QUERY)) {
    if (packet->len > 0) {
      _initFinished = (packet->data[0] == 0x01 || packet->cmd == CMD_WORK_STATUS_INIT_FINISHED_REPORT);
    }
  } else if (packet->control == CTRL_PRESENCE && packet->cmd == CMD_PRESENCE_QUERY_ENABLE && packet->len > 0) {
    _presenceEnable = packet->data[0];
  } else if (packet->control == CTRL_PRESENCE && (packet->cmd == CMD_PRESENCE_REPORT || packet->cmd == CMD_PRESENCE_QUERY_STATE) && packet->len > 0) {
    _presence = (ePresenceState_t)packet->data[0];
  } else if (packet->control == CTRL_PRESENCE && (packet->cmd == CMD_PRESENCE_MOTION_REPORT || packet->cmd == CMD_PRESENCE_QUERY_MOTION) && packet->len > 0) {
    _motionState = (eMotionState_t)packet->data[0];
  } else if (packet->control == CTRL_TRAJECTORY && (packet->cmd == CMD_TRAJECTORY_TARGET_REPORT || packet->cmd == CMD_TRAJECTORY_QUERY_TARGET)) {
    parseTargets(packet->data, packet->len);
  } else if (packet->control == CTRL_TRAJECTORY && packet->cmd == CMD_TRAJECTORY_QUERY_TRAJECTORY_LED && packet->len > 0) {
    _trajectoryLed = packet->data[0];
  } else if (packet->control == CTRL_TRAJECTORY && packet->cmd == CMD_TRAJECTORY_QUERY_MOTION_LED && packet->len > 0) {
    _motionLed = packet->data[0];
  } else if (packet->control == CTRL_DETECTION_RANGE && packet->cmd == CMD_DETECTION_RANGE_QUERY_TAGS) {
    parseTagList(packet->data, packet->len);
  } else if (packet->control == CTRL_DETECTION_RANGE && packet->cmd == CMD_DETECTION_RANGE_TAG_REPORT) {
    parseTagEvent(packet->data, packet->len);
  } else if (packet->control == CTRL_DETECTION_RANGE && packet->cmd == CMD_DETECTION_RANGE_QUERY_RANGE) {
    parseBoundaryRange(packet->data, packet->len);
  } else if (packet->control == CTRL_PEOPLE_COUNT && (packet->cmd == CMD_PEOPLE_COUNT_REPORT || packet->cmd == CMD_PEOPLE_COUNT_QUERY_COUNT)) {
    parsePeopleCount(packet->data, packet->len);
  }

  event = classifyPacket(packet);
  return event;
}

eReportedEvent_t DFRobot_C4004::classifyPacket(const sPacket_t *packet)
{
  if (packet == NULL) {
    return eEventError;
  }
  if ((packet->control == CTRL_SYSTEM && packet->cmd == CMD_SYSTEM_HEARTBEAT_REPORT) || (packet->control == CTRL_SYSTEM && packet->cmd == CMD_SYSTEM_HEARTBEAT_QUERY)) {
    return eEventHeartbeat;
  }
  if (packet->control == CTRL_WORK_STATUS && packet->cmd == CMD_WORK_STATUS_INIT_FINISHED_REPORT) {
    return eEventInitFinished;
  }
  if (packet->control == CTRL_PRESENCE && (packet->cmd == CMD_PRESENCE_REPORT || packet->cmd == CMD_PRESENCE_QUERY_STATE)) {
    return eEventPresence;
  }
  if (packet->control == CTRL_PRESENCE && (packet->cmd == CMD_PRESENCE_MOTION_REPORT || packet->cmd == CMD_PRESENCE_QUERY_MOTION)) {
    return eEventMotion;
  }
  if (packet->control == CTRL_TRAJECTORY && (packet->cmd == CMD_TRAJECTORY_TARGET_REPORT || packet->cmd == CMD_TRAJECTORY_QUERY_TARGET)) {
    return eEventTrajectory;
  }
  if (packet->control == CTRL_DETECTION_RANGE && packet->cmd == CMD_DETECTION_RANGE_TAG_REPORT) {
    return eEventTag;
  }
  if (packet->control == CTRL_PEOPLE_COUNT && (packet->cmd == CMD_PEOPLE_COUNT_REPORT || packet->cmd == CMD_PEOPLE_COUNT_QUERY_COUNT)) {
    return eEventPeopleCount;
  }
  return eEventUnknown;
}

bool DFRobot_C4004::queryByte(uint8_t control, uint8_t cmd, uint8_t *value)
{
  uint8_t data = QUERY_DATA;
  sPacket_t packet;

  if (value == NULL) {
    return false;
  }
  if (!requestFrame(control, cmd, &data, 1, &packet) || packet.len < 1) {
    return false;
  }
  *value = packet.data[0];
  return true;
}

bool DFRobot_C4004::setByte(uint8_t control, uint8_t cmd, uint8_t value)
{
  sPacket_t packet;
  return requestFrame(control, cmd, &value, 1, &packet);
}

bool DFRobot_C4004::queryUint32(uint8_t control, uint8_t cmd, uint32_t *value)
{
  uint8_t data = QUERY_DATA;
  sPacket_t packet;

  if (value == NULL) {
    return false;
  }
  if (!requestFrame(control, cmd, &data, 1, &packet) || packet.len < 4) {
    return false;
  }
  *value = readUint32(packet.data);
  return true;
}

bool DFRobot_C4004::setUint32(uint8_t control, uint8_t cmd, uint32_t value)
{
  uint8_t data[4];
  sPacket_t packet;

  writeUint32(data, value);
  return requestFrame(control, cmd, data, sizeof(data), &packet);
}

String DFRobot_C4004::queryString(uint8_t control, uint8_t cmd)
{
  uint8_t data = QUERY_DATA;
  sPacket_t packet;
  String ret = "";

  if (!requestFrame(control, cmd, &data, 1, &packet)) {
    return ret;
  }
  for (uint16_t i = 0; i < packet.len; i++) {
    if (packet.data[i] != 0) {
      ret += (char)packet.data[i];
    }
  }
  return ret;
}

void DFRobot_C4004::parseTargets(const uint8_t *data, uint16_t len)
{
  const uint8_t targetLen = 11;
  uint8_t count = 0;

  if (data == NULL) {
    return;
  }
  count = len / targetLen;
  if (count > MAX_TARGETS) {
    count = MAX_TARGETS;
  }
  _targetCount = count;

  for (uint8_t i = 0; i < count; i++) {
    uint16_t offset = i * targetLen;
    _targets[i].index = data[offset];
    _targets[i].kinesia = data[offset + 1];
    _targets[i].targetFeature = (eTargetFeature_t)data[offset + 2];
    _targets[i].x = readSignBitInt16(&data[offset + 3]);
    _targets[i].y = readSignBitInt16(&data[offset + 5]);
    _targets[i].height = readSignBitInt16(&data[offset + 7]);
    _targets[i].speed = readSignBitInt16(&data[offset + 9]);
  }
}

void DFRobot_C4004::parseTagList(const uint8_t *data, uint16_t len)
{
  uint16_t total = 0;
  uint8_t count = 0;

  if (data == NULL || len < 2) {
    _tagCount = 0;
    return;
  }

  total = readUint16(data);
  count = (uint8_t)total;
  if (count > MAX_TAGS) {
    count = MAX_TAGS;
  }
  if (len < (uint16_t)(2 + count * 11)) {
    count = (len - 2) / 11;
  }
  _tagCount = count;

  for (uint8_t i = 0; i < count; i++) {
    uint16_t offset = 2 + i * 11;
    _tags[i].tagIndex = data[offset];
    _tags[i].tagType = (eTagType_t)data[offset + 1];
    _tags[i].scopeType = (eTagRangeType_t)data[offset + 2];
    _tags[i].centerX = readSignBitInt16(&data[offset + 3]);
    _tags[i].centerY = readSignBitInt16(&data[offset + 5]);
    _tags[i].width = readUint16(&data[offset + 7]);
    _tags[i].height = readUint16(&data[offset + 9]);
  }
}

void DFRobot_C4004::parseTagEvent(const uint8_t *data, uint16_t len)
{
  if (data == NULL || len < 7) {
    _tagInfoValid = false;
    return;
  }

  memset(&_tagInfo, 0, sizeof(_tagInfo));
  _tagInfo.tagIndex = data[0];
  _tagInfo.tagType = (eTagType_t)data[1];
  _tagInfo.centerX = readSignBitInt16(&data[2]);
  _tagInfo.centerY = readSignBitInt16(&data[4]);
  if (_tagInfo.tagType == eTagTypeEnterExit) {
    _tagInfo.enterExit = data[6];
  } else if (_tagInfo.tagType == eTagTypeApproachAway) {
    _tagInfo.motionDir = data[6];
  } else if (_tagInfo.tagType == eTagTypePeopleCounting) {
    _tagInfo.motionNum = (data[6] >> 4) & 0x0F;
    _tagInfo.staticNum = data[6] & 0x0F;
  }
  _tagInfoValid = true;
}

void DFRobot_C4004::parseBoundaryRange(const uint8_t *data, uint16_t len)
{
  uint8_t offset = 1;

  if (data == NULL || len < 1) {
    return;
  }
  _rangeInfo.mode = (eDetectionRangeMode_t)data[0];

  if (_rangeInfo.mode == eRangeFourSide) {
    if (len >= 10 && data[1] == 0x00) {
      offset = 2;
    }
    if (len >= (uint16_t)(offset + 8)) {
      _rangeInfo.xPositiveCm = readSignBitInt16(&data[offset]);
      _rangeInfo.xNegativeCm = readSignBitInt16(&data[offset + 2]);
      _rangeInfo.yPositiveCm = readSignBitInt16(&data[offset + 4]);
      _rangeInfo.yNegativeCm = readSignBitInt16(&data[offset + 6]);
    }
  }
}

void DFRobot_C4004::parsePeopleCount(const uint8_t *data, uint16_t len)
{
  if (data == NULL || len == 0) {
    _peopleCount = 0;
    return;
  }
  if (len >= 2) {
    _peopleCount = data[1];
  } else {
    _peopleCount = data[0];
  }
}

uint16_t DFRobot_C4004::readUint16(const uint8_t *data) const
{
  return ((uint16_t)data[0] << 8) | data[1];
}

int16_t DFRobot_C4004::readInt16(const uint8_t *data) const
{
  return (int16_t)readUint16(data);
}

int16_t DFRobot_C4004::readSignBitInt16(const uint8_t *data) const
{
  uint16_t raw = readUint16(data);
  int16_t magnitude = (int16_t)(raw & 0x7FFF);

  if ((raw & 0x8000) != 0) {
    return (int16_t)(-magnitude);
  }
  return magnitude;
}

uint32_t DFRobot_C4004::readUint32(const uint8_t *data) const
{
  return ((uint32_t)data[0] << 24) | ((uint32_t)data[1] << 16) | ((uint32_t)data[2] << 8) | data[3];
}

void DFRobot_C4004::writeUint16(uint8_t *data, uint16_t value) const
{
  data[0] = (uint8_t)(value >> 8);
  data[1] = (uint8_t)(value & 0xFF);
}

void DFRobot_C4004::writeInt16(uint8_t *data, int16_t value) const
{
  writeUint16(data, (uint16_t)value);
}

void DFRobot_C4004::writeSignBitInt16(uint8_t *data, int16_t value) const
{
  int32_t magnitude = value;
  uint16_t raw = 0;

  if (magnitude < 0) {
    magnitude = -magnitude;
    raw = 0x8000;
  }
  if (magnitude > 0x7FFF) {
    magnitude = 0x7FFF;
  }
  raw |= (uint16_t)magnitude;
  writeUint16(data, raw);
}

void DFRobot_C4004::writeUint32(uint8_t *data, uint32_t value) const
{
  data[0] = (uint8_t)(value >> 24);
  data[1] = (uint8_t)((value >> 16) & 0xFF);
  data[2] = (uint8_t)((value >> 8) & 0xFF);
  data[3] = (uint8_t)(value & 0xFF);
}
