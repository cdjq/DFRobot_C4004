/*!
 * @file DFRobot_C4004.cpp
 * @brief Implementation of the DFRobot C4004 sensor driver.
 * @copyright Copyright (c) 2026 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @license The MIT License (MIT)
 * @author JiaLi(jia.li@dfrobot.com)
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
DFRobot_C4004::DFRobot_C4004(SoftwareSerial *pSerial, uint32_t baud)
{
  _serial = pSerial;
  _s      = pSerial;
  _baud   = baud;
  _rxpin  = 0;
  _txpin  = 0;
  initObject();
}
#else
DFRobot_C4004::DFRobot_C4004(HardwareSerial *pSerial, uint32_t baud, uint8_t rxpin, uint8_t txpin)
{
  _serial = pSerial;
  _s      = pSerial;
  _baud   = baud;
  _rxpin  = rxpin;
  _txpin  = txpin;
  initObject();
}
#endif

void DFRobot_C4004::initObject(void)
{
  _heartbeat     = false;
  _initFinished  = false;
  _presence      = eNoPresence;
  _motionState   = eMotionNone;
  _trajectoryLed = 0xFF;
  _motionLed     = 0xFF;
  _targetCount   = 0;
  memset(_targets, 0, sizeof(_targets));
  memset(&_tagInfo, 0, sizeof(_tagInfo));
  _tagInfoValid = false;
  memset(&_rangeInfo, 0, sizeof(_rangeInfo));
  _peopleCount = 0;
  _rangeMode   = eRangeUnknown;
  _rxHead      = 0;
  _rxTail         = 0;
  _pendingValid   = false;
  memset(&_pendingPacket, 0, sizeof(_pendingPacket));
  resetRxParser();
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
  uint8_t    data   = C4004_QUERY_DATA;
  sPacket_t &packet = _rxPacket;

  if (requestFrame(C4004_CTRL_WORK_STATUS, C4004_CMD_WORK_STATUS_INIT_FINISHED_QUERY, &data, 1, &packet)) {
    if (packet.len > 0) {
      _initFinished = (packet.data[0] == 0x01);
    }
  }
  return _initFinished;
}

bool DFRobot_C4004::isConnected(void)
{
  return getHeartbeat(eGetDataActive);
}

bool DFRobot_C4004::reset(void)
{
  uint8_t    data   = C4004_QUERY_DATA;
  sPacket_t &packet = _rxPacket;
  bool       ret    = requestFrame(C4004_CTRL_SYSTEM, C4004_CMD_SYSTEM_RESET, &data, 1, &packet, C4004_RESET_TIMEOUT);
  delay(100);
  return ret;
}

bool DFRobot_C4004::factoryReset(void)
{
  uint8_t    data   = C4004_QUERY_DATA;
  sPacket_t &packet = _rxPacket;
  bool       ret    = requestFrame(C4004_CTRL_SYSTEM, C4004_CMD_SYSTEM_FACTORY_RESET, &data, 1, &packet, C4004_FACTORY_RESET_TIMEOUT);
  delay(100);
  return ret;
}

bool DFRobot_C4004::getHeartbeat(eGetDataMode_t mode)
{
  uint8_t    data   = C4004_QUERY_DATA;
  sPacket_t &packet = _rxPacket;

  if (mode == eGetDataReport) {
    return _heartbeat;
  }

  if (!requestFrame(C4004_CTRL_SYSTEM, C4004_CMD_SYSTEM_HEARTBEAT_QUERY, &data, 1, &packet)) {
    _heartbeat = false;
    return false;
  }
  if (packet.len > 0 && packet.data[0] != C4004_QUERY_DATA) {
    _heartbeat = false;
    return false;
  }
  _heartbeat = true;
  return true;
}

DFRobot_C4004::eReportedEvent_t DFRobot_C4004::getReportedEvent(uint16_t timeoutMs)
{
  sPacket_t &packet = _rxPacket;

  if (!readFrame(&packet, timeoutMs)) {
    return eEventNone;
  }
  return handlePacket(&packet);
}

String DFRobot_C4004::getProductModel(void)
{
  return queryString(C4004_CTRL_PRODUCT_INFO, C4004_CMD_PRODUCT_MODEL_QUERY);
}

uint16_t DFRobot_C4004::getProductID(void)
{
  uint8_t    data   = C4004_QUERY_DATA;
  sPacket_t &packet = _rxPacket;

  if (!requestFrame(C4004_CTRL_PRODUCT_INFO, C4004_CMD_PRODUCT_ID_QUERY, &data, 1, &packet)) {
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
  return queryString(C4004_CTRL_PRODUCT_INFO, C4004_CMD_PRODUCT_HARDWARE_VERSION_QUERY);
}

String DFRobot_C4004::getFirmwareVersion(void)
{
  return queryString(C4004_CTRL_PRODUCT_INFO, C4004_CMD_PRODUCT_FIRMWARE_VERSION_QUERY);
}

bool DFRobot_C4004::setInstallInfo(sInstallInfo_t &info)
{
  uint8_t    angleData[6];
  uint8_t    heightData[2];
  uint8_t    modeData    = (uint8_t)info.mode;
  sPacket_t &packet      = _rxPacket;
  int32_t    xAngleProto = (int32_t)info.xAngle * 100;
  int32_t    yAngleProto = (int32_t)info.yAngle * 100;
  int32_t    zAngleProto = (int32_t)info.zAngle * 100;

  if (info.mode != eSide && info.mode != eTop) {
    return false;
  }

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

  if (!requestFrame(C4004_CTRL_INSTALL_INFO, C4004_CMD_INSTALL_SET_MODE, &modeData, 1, &packet)) {
    return false;
  }
  if (!requestFrame(C4004_CTRL_INSTALL_INFO, C4004_CMD_INSTALL_SET_ANGLE, angleData, sizeof(angleData), &packet)) {
    return false;
  }
  if (!requestFrame(C4004_CTRL_INSTALL_INFO, C4004_CMD_INSTALL_SET_HEIGHT, heightData, sizeof(heightData), &packet)) {
    return false;
  }
  return true;
}

bool DFRobot_C4004::getInstallInfo(sInstallInfo_t *pInfo)
{
  uint8_t    data   = C4004_QUERY_DATA;
  sPacket_t &packet = _rxPacket;

  if (pInfo == NULL) {
    return false;
  }
  memset(pInfo, 0, sizeof(sInstallInfo_t));
  pInfo->mode = eUnknown;

  if (!requestFrame(C4004_CTRL_INSTALL_INFO, C4004_CMD_INSTALL_QUERY_ANGLE, &data, 1, &packet) || packet.len < 6) {
    return false;
  }
  pInfo->xAngle = readInt16(&packet.data[0]) / 100;
  pInfo->yAngle = readInt16(&packet.data[2]) / 100;
  pInfo->zAngle = readInt16(&packet.data[4]) / 100;

  if (!requestFrame(C4004_CTRL_INSTALL_INFO, C4004_CMD_INSTALL_QUERY_HEIGHT, &data, 1, &packet) || packet.len < 2) {
    return false;
  }
  pInfo->heightCm = readUint16(packet.data);

  if (!requestFrame(C4004_CTRL_INSTALL_INFO, C4004_CMD_INSTALL_QUERY_MODE, &data, 1, &packet) || packet.len < 1) {
    return false;
  }
  pInfo->mode = (eInstallMode_t)packet.data[0];
  return true;
}

bool DFRobot_C4004::setInstallHeight(int32_t height)
{
  uint8_t    heightData[2];
  sPacket_t &packet = _rxPacket;

  if (height < 0 || height > 0xFFFFL) {
    return false;
  }

  writeUint16(heightData, (uint16_t)height);
  return requestFrame(C4004_CTRL_INSTALL_INFO, C4004_CMD_INSTALL_SET_HEIGHT, heightData, sizeof(heightData), &packet);
}

bool DFRobot_C4004::getInstallHeight(int *pHeight)
{
  uint8_t    data   = C4004_QUERY_DATA;
  sPacket_t &packet = _rxPacket;

  if (pHeight == NULL) {
    return false;
  }

  if (!requestFrame(C4004_CTRL_INSTALL_INFO, C4004_CMD_INSTALL_QUERY_HEIGHT, &data, 1, &packet) || packet.len < 2) {
    return false;
  }

  *pHeight = (int)readUint16(packet.data);
  return true;
}

bool DFRobot_C4004::setPresenceEnable(bool enable)
{
  return setByte(C4004_CTRL_PRESENCE, C4004_CMD_PRESENCE_SET_ENABLE, enable ? 1 : 0);
}

bool DFRobot_C4004::getPresenceEnable(bool *pEnable)
{
  uint8_t value = 0;

  if (pEnable == NULL) {
    return false;
  }
  if (!queryByte(C4004_CTRL_PRESENCE, C4004_CMD_PRESENCE_QUERY_ENABLE, &value)) {
    return false;
  }
  *pEnable = (value != 0);
  return true;
}

DFRobot_C4004::ePresenceState_t DFRobot_C4004::getPresenceState(eGetDataMode_t mode)
{
  uint8_t value = 0;
  if (mode == eGetDataActive) {
    if (queryByte(C4004_CTRL_PRESENCE, C4004_CMD_PRESENCE_QUERY_STATE, &value)) {
      _presence = (ePresenceState_t)value;
    }
  }
  return _presence;
}

DFRobot_C4004::eMotionState_t DFRobot_C4004::getMotionState(eGetDataMode_t mode)
{
  uint8_t value = 0;
  if (mode == eGetDataActive) {
    if (queryByte(C4004_CTRL_PRESENCE, C4004_CMD_PRESENCE_QUERY_MOTION, &value)) {
      _motionState = (eMotionState_t)value;
    }
  }
  return _motionState;
}

bool DFRobot_C4004::setTrajectoryTrackEnable(bool enable)
{
  return setByte(C4004_CTRL_TRAJECTORY, C4004_CMD_TRAJECTORY_SET_ENABLE, enable ? 1 : 0);
}

bool DFRobot_C4004::getTrajectoryTrackEnable(bool *pEnable)
{
  uint8_t value = 0;

  if (pEnable == NULL) {
    return false;
  }
  if (!queryByte(C4004_CTRL_TRAJECTORY, C4004_CMD_TRAJECTORY_QUERY_ENABLE, &value)) {
    return false;
  }
  *pEnable = (value != 0);
  return true;
}

bool DFRobot_C4004::setFrameGenerateCount(uint8_t frames)
{
  if (frames < 1 || frames > 7) {
    return false;
  }
  return setByte(C4004_CTRL_TRAJECTORY, C4004_CMD_TRAJECTORY_SET_CHECK_TO_ACTIVE_FRAMES, frames);
}

bool DFRobot_C4004::getFrameGenerateCount(uint8_t *pFrames)
{
  return queryByte(C4004_CTRL_TRAJECTORY, C4004_CMD_TRAJECTORY_QUERY_CHECK_TO_ACTIVE_FRAMES, pFrames);
}

uint8_t DFRobot_C4004::getTargetList(sTargetInfo_t *pTargetBuf, uint8_t maxCount, eGetDataMode_t mode)
{
  uint8_t    data      = C4004_QUERY_DATA;
  sPacket_t &packet    = _rxPacket;
  uint8_t    copyCount = 0;

  if (mode == eGetDataActive) {
    requestFrame(C4004_CTRL_TRAJECTORY, C4004_CMD_TRAJECTORY_QUERY_TARGET, &data, 1, &packet);
  }

  if (pTargetBuf == NULL || maxCount == 0) {
    return _targetCount;
  }

  copyCount = _targetCount;
  if (copyCount > maxCount) {
    copyCount = maxCount;
  }
  for (uint8_t i = 0; i < copyCount; i++) {
    pTargetBuf[i] = _targets[i];
  }
  return copyCount;
}

bool DFRobot_C4004::setTrkLED(bool enable)
{
  if (setByte(C4004_CTRL_TRAJECTORY, C4004_CMD_TRAJECTORY_SET_TRAJECTORY_LED, enable ? 1 : 0)) {
    _trajectoryLed = enable ? 1 : 0;
    return true;
  }
  return false;
}

bool DFRobot_C4004::setOccLED(bool enable)
{
  if (setByte(C4004_CTRL_TRAJECTORY, C4004_CMD_TRAJECTORY_SET_MOTION_LED, enable ? 1 : 0)) {
    _motionLed = enable ? 1 : 0;
    return true;
  }
  return false;
}

bool DFRobot_C4004::getTrkLED(void)
{
  uint8_t value = 0;
  if (queryByte(C4004_CTRL_TRAJECTORY, C4004_CMD_TRAJECTORY_QUERY_TRAJECTORY_LED, &value)) {
    _trajectoryLed = value;
  }
  return (_trajectoryLed != 0);
}

bool DFRobot_C4004::getOccLED(void)
{
  uint8_t value = 0;
  if (queryByte(C4004_CTRL_TRAJECTORY, C4004_CMD_TRAJECTORY_QUERY_MOTION_LED, &value)) {
    _motionLed = value;
  }
  return (_motionLed != 0);
}

uint8_t DFRobot_C4004::getTags(sTagConfig_t *pTags, uint8_t maxTags, eGetDataMode_t mode)
{
  uint8_t    data   = C4004_QUERY_DATA;
  sPacket_t &packet = _rxPacket;
  (void)mode;

  if (!requestFrame(C4004_CTRL_DETECTION_RANGE, C4004_CMD_DETECTION_RANGE_QUERY_TAGS, &data, 1, &packet)) {
    return 0;
  }
  return parseTagList(packet.data, packet.len, pTags, maxTags);
}

bool DFRobot_C4004::getTagInfo(sTagInfo_t *pTagInfo)
{
  if (pTagInfo == NULL) {
    return false;
  }
  if (!_tagInfoValid) {
    return false;
  }
  *pTagInfo = _tagInfo;
  return true;
}

DFRobot_C4004::eTagSetStatus_t DFRobot_C4004::setTag(const sTagConfig_t &tag)
{
  uint8_t    data[8];
  sPacket_t &packet = _rxPacket;

  if (tag.tagType > eTagNoise || tag.scopeType > eTagRangeRectangle) {
    return eTagSetCommError;
  }
  if (tag.ioIndex == 1 || tag.ioIndex > 6) {
    return eTagSetCommError;
  }

  data[0] = tag.tagIndex;
  data[1] = (uint8_t)tag.tagType;
  data[2] = (uint8_t)tag.scopeType;
  data[3] = tag.ioIndex;
  writeUint16(&data[4], tag.width);
  writeUint16(&data[6], tag.height);

  if (!requestFrame(C4004_CTRL_DETECTION_RANGE, C4004_CMD_DETECTION_RANGE_SET_TAG, data, sizeof(data), &packet, C4004_TAG_SET_TIMEOUT)) {
    return eTagSetCommError;
  }
  // Response payload: tagIndex(1) tagType(1) scopeType(1) ioIndex(1) status(1) + center(4) + size(4).
  if (packet.len < 5) {
    return eTagSetCommError;
  }
  if (packet.data[0] != tag.tagIndex) {
    return eTagSetCommError;
  }
  if (packet.data[4] >= eTagSetSuccess && packet.data[4] <= eTagSetIndexOutOfRange) {
    return (eTagSetStatus_t)packet.data[4];
  }
  return eTagSetCommError;
}

bool DFRobot_C4004::clearTag(uint16_t tagIndex)
{
  uint8_t    data   = 0;
  sPacket_t &packet = _rxPacket;

  if (tagIndex == 0xFF || tagIndex > 0xFE) {
    return false;
  }

  data = (uint8_t)tagIndex;
  if (!requestFrame(C4004_CTRL_DETECTION_RANGE, C4004_CMD_DETECTION_RANGE_CLEAR_TAG, &data, 1, &packet)) {
    return false;
  }
  if (packet.len < 1) {
    return false;
  }
  if (packet.data[0] == 0xFE) {
    return false;
  }
  if (packet.data[0] == data) {
    return true;
  }
  return false;
}

bool DFRobot_C4004::clearAllTags(void)
{
  uint8_t    clearAll = 0xFF;
  sPacket_t &packet   = _rxPacket;

  if (!requestFrame(C4004_CTRL_DETECTION_RANGE, C4004_CMD_DETECTION_RANGE_CLEAR_TAG, &clearAll, 1, &packet)) {
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

bool DFRobot_C4004::setTagsFromConfig(const sTagConfig_t *pTags, uint8_t tagCount)
{
  const uint8_t tagLen      = 12;
  uint8_t      *data        = _rxPacket.data;
  uint16_t      offset      = 0;
  uint16_t      expectedLen = 0;
  sPacket_t    &packet      = _rxPacket;
  uint32_t      startTime   = 0;

  if (pTags == NULL && tagCount > 0) {
    return false;
  }
  if (tagCount > 32) {
    return false;
  }
  expectedLen = 2 + (uint16_t)tagCount * tagLen;
  if (expectedLen > C4004_MAX_PAYLOAD) {
    return false;
  }

  for (uint8_t i = 0; i < tagCount; i++) {
    if (pTags[i].tagType > eTagNoise || pTags[i].scopeType > eTagRangeRectangle) {
      return false;
    }
    if (pTags[i].ioIndex == 1 || pTags[i].ioIndex > 6) {
      return false;
    }
  }

  writeUint16(&data[offset], tagCount);
  offset += 2;
  for (uint8_t i = 0; i < tagCount; i++) {
    data[offset++] = pTags[i].tagIndex;
    data[offset++] = (uint8_t)pTags[i].tagType;
    data[offset++] = (uint8_t)pTags[i].scopeType;
    data[offset++] = pTags[i].ioIndex;
    writeSignBitInt16(&data[offset], pTags[i].centerX);
    offset += 2;
    writeSignBitInt16(&data[offset], pTags[i].centerY);
    offset += 2;
    writeUint16(&data[offset], pTags[i].width);
    offset += 2;
    writeUint16(&data[offset], pTags[i].height);
    offset += 2;
  }

  if (!sendCommand(C4004_CTRL_DETECTION_RANGE, C4004_CMD_DETECTION_RANGE_SET_TAGS_FROM_CONFIG, data, offset)) {
    return false;
  }

  startTime = millis();
  while ((uint32_t)(millis() - startTime) < C4004_DEFAULT_TIMEOUT) {
    uint16_t elapsed = (uint16_t)(millis() - startTime);
    if (elapsed >= C4004_DEFAULT_TIMEOUT) {
      break;
    }
    uint16_t leftTime = C4004_DEFAULT_TIMEOUT - elapsed;
    if (!readFrame(&packet, leftTime)) {
      continue;
    }
    handlePacket(&packet);
    if (packet.control == C4004_CTRL_DETECTION_RANGE && packet.cmd == C4004_CMD_DETECTION_RANGE_SET_TAGS_FROM_CONFIG) {
      if (packet.len != expectedLen) {
        return false;
      }
      if (readUint16(packet.data) != tagCount) {
        return false;
      }
      return true;
    }
  }
  return false;
}

bool DFRobot_C4004::setFourSidedRangeMode(sFourSidedRange_t &range)
{
  uint8_t    data[9];
  sPacket_t &packet = _rxPacket;

  data[0] = eRangeFourSide;
  writeSignBitInt16(&data[1], range.xMax);
  writeSignBitInt16(&data[3], range.xMin);
  writeSignBitInt16(&data[5], range.yMax);
  writeSignBitInt16(&data[7], range.yMin);

  if (!requestFrame(C4004_CTRL_DETECTION_RANGE, C4004_CMD_DETECTION_RANGE_SET_RANGE, data, sizeof(data), &packet, C4004_SET_RANGE_TIMEOUT)) {
    return false;
  }
  _rangeInfo = range;
  _rangeMode = eRangeFourSide;
  return true;
}

bool DFRobot_C4004::getFourSidedRangeMode(sFourSidedRange_t *pRange)
{
  uint8_t    data   = C4004_QUERY_DATA;
  sPacket_t &packet = _rxPacket;

  if (pRange == NULL) {
    return false;
  }
  if (!requestFrame(C4004_CTRL_DETECTION_RANGE, C4004_CMD_DETECTION_RANGE_QUERY_RANGE, &data, 1, &packet)) {
    return false;
  }
  *pRange = _rangeInfo;
  return true;
}

void DFRobot_C4004::setTrajectoryRangeMode(bool learning)
{
  uint8_t    data[2];
  sPacket_t &packet = _rxPacket;

  data[0] = eRangeTrajectory;
  data[1] = learning ? 1 : 0;

  requestFrame(C4004_CTRL_DETECTION_RANGE, C4004_CMD_DETECTION_RANGE_SET_RANGE, data, sizeof(data), &packet);
  _rangeMode = eRangeTrajectory;
}

bool DFRobot_C4004::setConfigFileModePoints(const sPoint_t *pPoints, uint16_t pointCount)
{
  uint8_t   *data      = _rxPacket.data;
  uint16_t   offset    = 0;
  uint16_t   respCount = 0;
  sPacket_t &packet    = _rxPacket;
  uint32_t   startTime = 0;

  if (pPoints == NULL && pointCount > 0) {
    return false;
  }
  if (pointCount > C4004_MAX_POINTS) {
    pointCount = C4004_MAX_POINTS;
  }

  data[offset++] = eRangeConfigFile;
  writeUint16(&data[offset], pointCount);
  offset += 2;

  for (uint16_t i = 0; i < pointCount; i++) {
    writeSignBitInt16(&data[offset], pPoints[i].x);
    offset += 2;
    writeSignBitInt16(&data[offset], pPoints[i].y);
    offset += 2;
  }

  if (!sendCommand(C4004_CTRL_DETECTION_RANGE, C4004_CMD_DETECTION_RANGE_SET_RANGE, data, offset)) {
    return false;
  }

  startTime = millis();
  while ((uint32_t)(millis() - startTime) < C4004_DEFAULT_TIMEOUT) {
    uint16_t elapsed = (uint16_t)(millis() - startTime);
    if (elapsed >= C4004_DEFAULT_TIMEOUT) {
      break;
    }
    uint16_t leftTime = C4004_DEFAULT_TIMEOUT - elapsed;
    if (!readFrame(&packet, leftTime)) {
      continue;
    }
    handlePacket(&packet);
    if (packet.control == C4004_CTRL_DETECTION_RANGE && packet.cmd == C4004_CMD_DETECTION_RANGE_SET_RANGE) {
      if (packet.len < 3 || packet.data[0] != (uint8_t)eRangeConfigFile) {
        return false;
      }
      respCount = readUint16(&packet.data[1]);
      if (respCount != pointCount || packet.len < (uint16_t)(3 + respCount * 4)) {
        return false;
      }
      _rangeMode = eRangeConfigFile;
      return true;
    }
  }
  return false;
}

bool DFRobot_C4004::getTrajectoryRangeMode(sPoint_t *pPoints, uint16_t *pPointCount)
{
  uint8_t    data   = C4004_QUERY_DATA;
  sPacket_t &packet = _rxPacket;
  uint16_t   count  = 0;

  if (pPoints == NULL || pPointCount == NULL) {
    return false;
  }
  *pPointCount = 0;

  if (!requestFrame(C4004_CTRL_DETECTION_RANGE, C4004_CMD_DETECTION_RANGE_QUERY_RANGE, &data, 1, &packet)) {
    return false;
  }
  if (packet.len < 3) {
    return false;
  }
  if ((eDetectionRangeMode_t)packet.data[0] != eRangeTrajectory) {
    return false;
  }

  count = readUint16(&packet.data[1]);
  if (count > C4004_MAX_POINTS) {
    return false;
  }
  if (packet.len < (uint16_t)(3 + count * 4)) {
    return false;
  }

  for (uint16_t i = 0; i < count; i++) {
    uint16_t offset = (uint16_t)(3 + i * 4);
    pPoints[i].x     = readSignBitInt16(&packet.data[offset]);
    pPoints[i].y     = readSignBitInt16(&packet.data[offset + 2]);
  }
  *pPointCount = count;
  return true;
}

bool DFRobot_C4004::getConfigFileModePoints(sPoint_t *pPoints, uint16_t *pPointCount)
{
  uint8_t    data   = C4004_QUERY_DATA;
  sPacket_t &packet = _rxPacket;
  uint16_t   count  = 0;

  if (pPoints == NULL || pPointCount == NULL) {
    return false;
  }
  *pPointCount = 0;

  if (!requestFrame(C4004_CTRL_DETECTION_RANGE, C4004_CMD_DETECTION_RANGE_QUERY_RANGE, &data, 1, &packet)) {
    return false;
  }
  if (packet.len < 3) {
    return false;
  }
  if ((eDetectionRangeMode_t)packet.data[0] != eRangeConfigFile) {
    return false;
  }

  count = readUint16(&packet.data[1]);
  if (count > C4004_MAX_POINTS) {
    return false;
  }
  if (packet.len < (uint16_t)(3 + count * 4)) {
    return false;
  }

  for (uint16_t i = 0; i < count; i++) {
    uint16_t offset = (uint16_t)(3 + i * 4);
    pPoints[i].x     = readSignBitInt16(&packet.data[offset]);
    pPoints[i].y     = readSignBitInt16(&packet.data[offset + 2]);
  }
  *pPointCount = count;
  return true;
}

DFRobot_C4004::eDetectionRangeMode_t DFRobot_C4004::getDetectionRangeMode(void)
{
  uint8_t    data   = C4004_QUERY_DATA;
  sPacket_t &packet = _rxPacket;

  if (!requestFrame(C4004_CTRL_DETECTION_RANGE, C4004_CMD_DETECTION_RANGE_QUERY_RANGE, &data, 1, &packet)) {
    return _rangeMode;
  }
  return _rangeMode;
}

uint8_t DFRobot_C4004::getPeopleCount(eGetDataMode_t mode)
{
  uint8_t    data   = C4004_QUERY_DATA;
  sPacket_t &packet = _rxPacket;

  if (mode == eGetDataActive) {
    requestFrame(C4004_CTRL_PEOPLE_COUNT, C4004_CMD_PEOPLE_COUNT_QUERY_COUNT, &data, 1, &packet);
  }
  return _peopleCount;
}

bool DFRobot_C4004::setRealTimePeopleTime(uint32_t time)
{
  return setUint32(C4004_CTRL_PEOPLE_COUNT, C4004_CMD_PEOPLE_COUNT_SET_REPORT_INTERVAL, time);
}

bool DFRobot_C4004::getRealTimePeopleTime(uint32_t *pTime)
{
  return queryUint32(C4004_CTRL_PEOPLE_COUNT, C4004_CMD_PEOPLE_COUNT_QUERY_REPORT_INTERVAL, pTime);
}

bool DFRobot_C4004::clearPeopleCount(void)
{
  uint8_t    data   = C4004_QUERY_DATA;
  sPacket_t &packet = _rxPacket;
  return requestFrame(C4004_CTRL_PEOPLE_COUNT, C4004_CMD_PEOPLE_COUNT_CLEAR_COUNT, &data, 1, &packet);
}

bool DFRobot_C4004::setTrackMeters(uint32_t distanceCm)
{
  return setUint32(C4004_CTRL_PEOPLE_COUNT, C4004_CMD_PEOPLE_COUNT_SET_TRAJECTORY_DISTANCE, distanceCm);
}

bool DFRobot_C4004::getTrackMeters(uint32_t *pDistanceCm)
{
  return queryUint32(C4004_CTRL_PEOPLE_COUNT, C4004_CMD_PEOPLE_COUNT_QUERY_TRAJECTORY_DISTANCE, pDistanceCm);
}

bool DFRobot_C4004::setTrackExistsTime(uint32_t time)
{
  return setUint32(C4004_CTRL_PEOPLE_COUNT, C4004_CMD_PEOPLE_COUNT_SET_TRAJECTORY_HOLD_TIME, time);
}

bool DFRobot_C4004::getTrackExistsTime(uint32_t *pTime)
{
  return queryUint32(C4004_CTRL_PEOPLE_COUNT, C4004_CMD_PEOPLE_COUNT_QUERY_TRAJECTORY_HOLD_TIME, pTime);
}

bool DFRobot_C4004::setUnmannedTime(uint32_t delayTime)
{
  return setUint32(C4004_CTRL_PEOPLE_COUNT, C4004_CMD_PEOPLE_COUNT_SET_NO_PERSON_DELAY, delayTime);
}

bool DFRobot_C4004::getUnmannedTime(uint32_t *pDelayTime)
{
  return queryUint32(C4004_CTRL_PEOPLE_COUNT, C4004_CMD_PEOPLE_COUNT_QUERY_NO_PERSON_DELAY, pDelayTime);
}

bool DFRobot_C4004::sendCommand(uint8_t control, uint8_t cmd, const uint8_t *pData, uint16_t len)
{
  uint8_t checksum = 0;
  uint8_t value    = 0;

  if (_s == NULL || len > C4004_MAX_PAYLOAD) {
    return false;
  }

  value = C4004_FRAME_HEAD1;
  checksum += value;
  _s->write(value);
  value = C4004_FRAME_HEAD2;
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
    value = (pData == NULL) ? 0 : pData[i];
    checksum += value;
    _s->write(value);
  }

  _s->write(checksum);
  _s->write((uint8_t)C4004_FRAME_TAIL1);
  _s->write((uint8_t)C4004_FRAME_TAIL2);

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
      String         dataLog = "TX pData: ";
      for (uint16_t i = 0; i < dumpLen; i++) {
        appendHexByte(dataLog, (pData == NULL) ? 0 : pData[i]);
        if (i + 1 < dumpLen) {
          dataLog += " ";
        }
      }
      DBG(dataLog);
      if (len > dumpLen) {
        DBG(String("TX pData truncated, total len=") + String(len));
      }
    } else {
      DBG("TX pData: (none)");
    }
  }
#endif
  return true;
}

void DFRobot_C4004::resetRxParser(void)
{
  _asmState        = eRxAsmSyncH1;
  _asmIdx          = 0;
  _asmChecksum     = 0;
  _asmRecvChecksum = 0;
}

void DFRobot_C4004::discardRxRing(void)
{
  _rxHead = 0;
  _rxTail = 0;
}

void DFRobot_C4004::rxPushByte(uint8_t value)
{
  uint16_t nextHead = (uint16_t)((_rxHead + 1) % C4004_RX_RING_SIZE);

  if (nextHead == _rxTail) {
    _rxTail = (uint16_t)((_rxTail + 1) % C4004_RX_RING_SIZE);
  }
  _rxRing[_rxHead] = value;
  _rxHead          = nextHead;
}

bool DFRobot_C4004::rxPopByte(uint8_t *pValue)
{
  if (pValue == NULL || _rxTail == _rxHead) {
    return false;
  }
  *pValue  = _rxRing[_rxTail];
  _rxTail = (uint16_t)((_rxTail + 1) % C4004_RX_RING_SIZE);
  return true;
}

#ifdef ENABLE_DBG
void DFRobot_C4004::logRxPacket(const sPacket_t *pPacket, uint8_t recvChecksum)
{
  if (pPacket == NULL) {
    return;
  }

  String meta = "RX ctrl=0x";
  appendHexByte(meta, pPacket->control);
  meta += " cmd=0x";
  appendHexByte(meta, pPacket->cmd);
  meta += " len=";
  meta += String(pPacket->len);
  meta += " checksum=0x";
  appendHexByte(meta, recvChecksum);
  DBG(meta);

  if (pPacket->len > 0) {
    const uint16_t dumpLen = (pPacket->len > 24) ? 24 : pPacket->len;
    String         dataLog = "RX data: ";
    for (uint16_t i = 0; i < dumpLen; i++) {
      appendHexByte(dataLog, pPacket->data[i]);
      if (i + 1 < dumpLen) {
        dataLog += " ";
      }
    }
    DBG(dataLog);
    if (pPacket->len > dumpLen) {
      DBG(String("RX data truncated, total len=") + String(pPacket->len));
    }
  }
}
#else
void DFRobot_C4004::logRxPacket(const sPacket_t *pPacket, uint8_t recvChecksum)
{
  (void)pPacket;
  (void)recvChecksum;
}
#endif

void DFRobot_C4004::feedAsmByte(uint8_t value)
{
  switch (_asmState) {
    case eRxAsmSyncH1:
      if (value == C4004_FRAME_HEAD1) {
        _asmChecksum = value;
        _asmState    = eRxAsmSyncH2;
      }
      break;

    case eRxAsmSyncH2:
      if (value == C4004_FRAME_HEAD2) {
        _asmChecksum += value;
        _asmState = eRxAsmCtrl;
      } else {
        _asmState = eRxAsmSyncH1;
        if (value == C4004_FRAME_HEAD1) {
          _asmChecksum = value;
          _asmState    = eRxAsmSyncH2;
        }
      }
      break;

    case eRxAsmCtrl:
      _pendingPacket.control = value;
      _asmChecksum += value;
      _asmState = eRxAsmCmd;
      break;

    case eRxAsmCmd:
      _pendingPacket.cmd = value;
      _asmChecksum += value;
      _asmState = eRxAsmLenHi;
      break;

    case eRxAsmLenHi:
      _pendingPacket.len = (uint16_t)((uint16_t)value << 8);
      _asmChecksum += value;
      _asmState = eRxAsmLenLo;
      break;

    case eRxAsmLenLo:
      _pendingPacket.len |= value;
      _asmChecksum += value;
      if (_pendingPacket.len > C4004_MAX_PAYLOAD) {
#ifdef ENABLE_DBG
        DBG(String("payload too long, len=") + String(_pendingPacket.len) + String(" max=") + String(C4004_MAX_PAYLOAD));
#endif
        resetRxParser();
        discardRxRing();
        break;
      }
      _asmIdx = 0;
      if (_pendingPacket.len == 0) {
        _asmState = eRxAsmChecksum;
      } else {
        _asmState = eRxAsmPayload;
      }
      break;

    case eRxAsmPayload:
      _pendingPacket.data[_asmIdx++] = value;
      _asmChecksum += value;
      if (_asmIdx >= _pendingPacket.len) {
        _asmState = eRxAsmChecksum;
      }
      break;

    case eRxAsmChecksum:
      _asmRecvChecksum = value;
      _asmState        = eRxAsmTail1;
      break;

    case eRxAsmTail1:
      if (value != C4004_FRAME_TAIL1) {
        resetRxParser();
        break;
      }
      _asmState = eRxAsmTail2;
      break;

    case eRxAsmTail2:
      if (value != C4004_FRAME_TAIL2) {
        resetRxParser();
        break;
      }
      if (_asmChecksum != _asmRecvChecksum) {
        DBG("checksum error");
        resetRxParser();
        break;
      }
      _pendingValid = true;
      logRxPacket(&_pendingPacket, _asmRecvChecksum);
      resetRxParser();
      break;

    default:
      resetRxParser();
      break;
  }
}

void DFRobot_C4004::pumpRx(void)
{
  uint8_t value = 0;

  if (_s == NULL) {
    return;
  }

  while (_s->available() > 0) {
    rxPushByte((uint8_t)_s->read());
  }

  while (!_pendingValid && rxPopByte(&value)) {
    feedAsmByte(value);
  }
}

bool DFRobot_C4004::takePendingFrame(sPacket_t *pPacket)
{
  if (pPacket == NULL || !_pendingValid) {
    return false;
  }
  memcpy(pPacket, &_pendingPacket, sizeof(sPacket_t));
  _pendingValid = false;
  return true;
}

bool DFRobot_C4004::requestFrame(uint8_t control, uint8_t cmd, const uint8_t *pData, uint16_t len, sPacket_t *pResponse, uint16_t timeoutMs)
{
  uint32_t startTime = 0;

  if (pResponse == NULL) {
    return false;
  }
  memset(pResponse, 0, sizeof(sPacket_t));

  if (!sendCommand(control, cmd, pData, len)) {
    return false;
  }
  startTime = millis();

  while ((uint32_t)(millis() - startTime) < timeoutMs) {
    uint16_t elapsed = (uint16_t)(millis() - startTime);
    if (elapsed >= timeoutMs) {
      break;
    }
    uint16_t leftTime = timeoutMs - elapsed;
    if (!readFrame(pResponse, leftTime)) {
      continue;
    }
    handlePacket(pResponse);
    if (pResponse->control == control && pResponse->cmd == cmd) {
      return true;
    }
  }
  return false;
}

bool DFRobot_C4004::readFrame(sPacket_t *pPacket, uint16_t timeoutMs)
{
  uint32_t startTime = millis();

  if (pPacket == NULL || _s == NULL) {
    return false;
  }
  memset(pPacket, 0, sizeof(sPacket_t));

  while ((uint32_t)(millis() - startTime) < timeoutMs) {
    pumpRx();
    if (takePendingFrame(pPacket)) {
      return true;
    }
    delay(1);
  }
  return false;
}

bool DFRobot_C4004::readByte(uint8_t *pValue, uint16_t timeoutMs)
{
  uint32_t startTime = millis();

  if (pValue == NULL || _s == NULL) {
    return false;
  }

  do {
    if (_s->available() > 0) {
      *pValue = (uint8_t)_s->read();
      return true;
    }
  } while ((uint32_t)(millis() - startTime) < timeoutMs);

  return false;
}

void DFRobot_C4004::flushInput(void)
{
  discardRxRing();
  resetRxParser();
  _pendingValid = false;
  if (_s == NULL) {
    return;
  }
  while (_s->available() > 0) {
    _s->read();
  }
}

DFRobot_C4004::eReportedEvent_t DFRobot_C4004::handlePacket(const sPacket_t *pPacket)
{
  if (pPacket == NULL) {
    return eEventError;
  }

  if ((pPacket->control == C4004_CTRL_SYSTEM && pPacket->cmd == C4004_CMD_SYSTEM_HEARTBEAT_REPORT) || (pPacket->control == C4004_CTRL_SYSTEM && pPacket->cmd == C4004_CMD_SYSTEM_HEARTBEAT_QUERY)) {
    _heartbeat = true;
  } else if ((pPacket->control == C4004_CTRL_WORK_STATUS && pPacket->cmd == C4004_CMD_WORK_STATUS_INIT_FINISHED_REPORT) || (pPacket->control == C4004_CTRL_WORK_STATUS && pPacket->cmd == C4004_CMD_WORK_STATUS_INIT_FINISHED_QUERY)) {
    if (pPacket->len > 0) {
      _initFinished = (pPacket->data[0] == 0x01 || pPacket->cmd == C4004_CMD_WORK_STATUS_INIT_FINISHED_REPORT);
    }
  } else if (pPacket->control == C4004_CTRL_PRESENCE && (pPacket->cmd == C4004_CMD_PRESENCE_REPORT || pPacket->cmd == C4004_CMD_PRESENCE_QUERY_STATE) && pPacket->len > 0) {
    _presence = (ePresenceState_t)pPacket->data[0];
  } else if (pPacket->control == C4004_CTRL_PRESENCE && (pPacket->cmd == C4004_CMD_PRESENCE_MOTION_REPORT || pPacket->cmd == C4004_CMD_PRESENCE_QUERY_MOTION) && pPacket->len > 0) {
    _motionState = (eMotionState_t)pPacket->data[0];
  } else if (pPacket->control == C4004_CTRL_TRAJECTORY && (pPacket->cmd == C4004_CMD_TRAJECTORY_TARGET_REPORT || pPacket->cmd == C4004_CMD_TRAJECTORY_QUERY_TARGET)) {
    parseTargets(pPacket->data, pPacket->len);
  } else if (pPacket->control == C4004_CTRL_TRAJECTORY && pPacket->cmd == C4004_CMD_TRAJECTORY_QUERY_TRAJECTORY_LED && pPacket->len > 0) {
    _trajectoryLed = pPacket->data[0];
  } else if (pPacket->control == C4004_CTRL_TRAJECTORY && pPacket->cmd == C4004_CMD_TRAJECTORY_QUERY_MOTION_LED && pPacket->len > 0) {
    _motionLed = pPacket->data[0];
  } else if (pPacket->control == C4004_CTRL_DETECTION_RANGE && pPacket->cmd == C4004_CMD_DETECTION_RANGE_TAG_REPORT) {
    parseTagEvent(pPacket->data, pPacket->len);
  } else if (pPacket->control == C4004_CTRL_DETECTION_RANGE && pPacket->cmd == C4004_CMD_DETECTION_RANGE_QUERY_RANGE) {
    parseBoundaryRange(pPacket->data, pPacket->len);
  } else if (pPacket->control == C4004_CTRL_PEOPLE_COUNT && (pPacket->cmd == C4004_CMD_PEOPLE_COUNT_REPORT || pPacket->cmd == C4004_CMD_PEOPLE_COUNT_QUERY_COUNT)) {
    parsePeopleCount(pPacket->data, pPacket->len);
  }

  return classifyPacket(pPacket);
}

DFRobot_C4004::eReportedEvent_t DFRobot_C4004::classifyPacket(const sPacket_t *pPacket)
{
  if (pPacket == NULL) {
    return eEventError;
  }
  if ((pPacket->control == C4004_CTRL_SYSTEM && pPacket->cmd == C4004_CMD_SYSTEM_HEARTBEAT_REPORT) || (pPacket->control == C4004_CTRL_SYSTEM && pPacket->cmd == C4004_CMD_SYSTEM_HEARTBEAT_QUERY)) {
    return eEventHeartbeat;
  }
  if (pPacket->control == C4004_CTRL_WORK_STATUS && pPacket->cmd == C4004_CMD_WORK_STATUS_INIT_FINISHED_REPORT) {
    return eEventInitFinished;
  }
  if (pPacket->control == C4004_CTRL_PRESENCE && (pPacket->cmd == C4004_CMD_PRESENCE_REPORT || pPacket->cmd == C4004_CMD_PRESENCE_QUERY_STATE)) {
    return eEventPresence;
  }
  if (pPacket->control == C4004_CTRL_PRESENCE && (pPacket->cmd == C4004_CMD_PRESENCE_MOTION_REPORT || pPacket->cmd == C4004_CMD_PRESENCE_QUERY_MOTION)) {
    return eEventMotion;
  }
  if (pPacket->control == C4004_CTRL_TRAJECTORY && (pPacket->cmd == C4004_CMD_TRAJECTORY_TARGET_REPORT || pPacket->cmd == C4004_CMD_TRAJECTORY_QUERY_TARGET)) {
    return eEventTrajectory;
  }
  if (pPacket->control == C4004_CTRL_DETECTION_RANGE && pPacket->cmd == C4004_CMD_DETECTION_RANGE_TAG_REPORT) {
    return eEventTag;
  }
  if (pPacket->control == C4004_CTRL_PEOPLE_COUNT && (pPacket->cmd == C4004_CMD_PEOPLE_COUNT_REPORT || pPacket->cmd == C4004_CMD_PEOPLE_COUNT_QUERY_COUNT)) {
    return eEventPeopleCount;
  }
  return eEventUnknown;
}

bool DFRobot_C4004::queryByte(uint8_t control, uint8_t cmd, uint8_t *pValue)
{
  uint8_t    data   = C4004_QUERY_DATA;
  sPacket_t &packet = _rxPacket;

  if (pValue == NULL) {
    return false;
  }
  if (!requestFrame(control, cmd, &data, 1, &packet) || packet.len < 1) {
    return false;
  }
  *pValue = packet.data[0];
  return true;
}

bool DFRobot_C4004::setByte(uint8_t control, uint8_t cmd, uint8_t value)
{
  sPacket_t &packet = _rxPacket;
  return requestFrame(control, cmd, &value, 1, &packet);
}

bool DFRobot_C4004::queryUint32(uint8_t control, uint8_t cmd, uint32_t *pValue)
{
  uint8_t    data   = C4004_QUERY_DATA;
  sPacket_t &packet = _rxPacket;

  if (pValue == NULL) {
    return false;
  }
  if (!requestFrame(control, cmd, &data, 1, &packet) || packet.len < 4) {
    return false;
  }
  *pValue = readUint32(packet.data);
  return true;
}

bool DFRobot_C4004::setUint32(uint8_t control, uint8_t cmd, uint32_t value)
{
  uint8_t    data[4];
  sPacket_t &packet = _rxPacket;

  writeUint32(data, value);
  return requestFrame(control, cmd, data, sizeof(data), &packet);
}

String DFRobot_C4004::queryString(uint8_t control, uint8_t cmd)
{
  uint8_t    data   = C4004_QUERY_DATA;
  sPacket_t &packet = _rxPacket;
  String     ret    = "";

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

void DFRobot_C4004::parseTargets(const uint8_t *pData, uint16_t len)
{
  const uint8_t targetLen = 11;
  uint8_t       count     = 0;

  if (pData == NULL) {
    _targetCount = 0;
    return;
  }
  count = len / targetLen;
  if (count > C4004_MAX_TARGETS) {
    count = C4004_MAX_TARGETS;
  }
  _targetCount = count;

  for (uint8_t i = 0; i < count; i++) {
    uint16_t offset           = i * targetLen;
    _targets[i].index         = pData[offset];
    _targets[i].kinesia       = pData[offset + 1];
    _targets[i].targetFeature = (eTargetFeature_t)pData[offset + 2];
    _targets[i].x             = readSignBitInt16(&pData[offset + 3]);
    _targets[i].y             = readSignBitInt16(&pData[offset + 5]);
    _targets[i].height        = readSignBitInt16(&pData[offset + 7]);
    _targets[i].speed         = readSignBitInt16(&pData[offset + 9]);
  }
}

uint8_t DFRobot_C4004::parseTagList(const uint8_t *pData, uint16_t len, sTagConfig_t *pTags, uint8_t maxTags)
{
  const uint8_t tagLen       = 12;
  uint16_t      availableLen = 0;
  uint16_t      total        = 0;
  uint8_t       actualCount  = 0;
  uint8_t       copyCount    = 0;

  if (pData == NULL || len < 2) {
    return 0;
  }

  total        = readUint16(pData);
  actualCount  = (total > 0xFF) ? 0xFF : (uint8_t)total;
  availableLen = len - 2;
  if (availableLen < (uint16_t)(actualCount * tagLen)) {
    actualCount = availableLen / tagLen;
  }
  if (pTags != NULL) {
    copyCount = actualCount;
    if (copyCount > maxTags) {
      copyCount = maxTags;
    }

    for (uint8_t i = 0; i < copyCount; i++) {
      uint16_t offset   = 2 + i * tagLen;
      pTags[i].tagIndex  = pData[offset];
      pTags[i].tagType   = (eTagType_t)pData[offset + 1];
      pTags[i].scopeType = (eTagRangeType_t)pData[offset + 2];
      pTags[i].ioIndex   = pData[offset + 3];
      pTags[i].centerX   = readSignBitInt16(&pData[offset + 4]);
      pTags[i].centerY   = readSignBitInt16(&pData[offset + 6]);
      pTags[i].width     = readUint16(&pData[offset + 8]);
      pTags[i].height    = readUint16(&pData[offset + 10]);
    }
  }
  return actualCount;
}

void DFRobot_C4004::parseTagEvent(const uint8_t *pData, uint16_t len)
{
  if (pData == NULL || len < 8) {
    _tagInfoValid = false;
    return;
  }

  memset(&_tagInfo, 0, sizeof(_tagInfo));
  _tagInfo.tagIndex = pData[0];
  _tagInfo.tagType  = (eTagType_t)pData[1];
  _tagInfo.ioIndex  = pData[2];
  _tagInfo.centerX  = readSignBitInt16(&pData[3]);
  _tagInfo.centerY  = readSignBitInt16(&pData[5]);
  if (_tagInfo.tagType == eTagBoundary) {
    _tagInfo.enterExit = static_cast<eBoundaryDirection_t>(pData[7]);
  } else if (_tagInfo.tagType == eTagApproachAway) {
    _tagInfo.motionDir = static_cast<eApproachAwayDirection_t>(pData[7]);
  } else if (_tagInfo.tagType == eTagPeopleCounting) {
    _tagInfo.motionNum = (pData[7] >> 4) & 0x0F;
    _tagInfo.staticNum = pData[7] & 0x0F;
  }
  _tagInfoValid = true;
}

void DFRobot_C4004::parseBoundaryRange(const uint8_t *pData, uint16_t len)
{
  uint8_t offset = 1;

  if (pData == NULL || len < 1) {
    return;
  }
  _rangeMode = (eDetectionRangeMode_t)pData[0];

  if (_rangeMode == eRangeFourSide) {
    if (len >= 10 && pData[1] == 0x00) {
      offset = 2;
    }
    if (len >= (uint16_t)(offset + 8)) {
      _rangeInfo.xMax = readSignBitInt16(&pData[offset]);
      _rangeInfo.xMin = readSignBitInt16(&pData[offset + 2]);
      _rangeInfo.yMax = readSignBitInt16(&pData[offset + 4]);
      _rangeInfo.yMin = readSignBitInt16(&pData[offset + 6]);
    }
  }
}

void DFRobot_C4004::parsePeopleCount(const uint8_t *pData, uint16_t len)
{
  if (pData == NULL || len == 0) {
    _peopleCount = 0;
    return;
  }
  if (len >= 2) {
    _peopleCount = pData[1];
  } else {
    _peopleCount = pData[0];
  }
}

uint16_t DFRobot_C4004::readUint16(const uint8_t *pData) const
{
  return ((uint16_t)pData[0] << 8) | pData[1];
}

int16_t DFRobot_C4004::readInt16(const uint8_t *pData) const
{
  return (int16_t)readUint16(pData);
}

int16_t DFRobot_C4004::readSignBitInt16(const uint8_t *pData) const
{
  uint16_t raw       = readUint16(pData);
  int16_t  magnitude = (int16_t)(raw & 0x7FFF);

  if ((raw & 0x8000) != 0) {
    return (int16_t)(-magnitude);
  }
  return magnitude;
}

uint32_t DFRobot_C4004::readUint32(const uint8_t *pData) const
{
  return ((uint32_t)pData[0] << 24) | ((uint32_t)pData[1] << 16) | ((uint32_t)pData[2] << 8) | pData[3];
}

void DFRobot_C4004::writeUint16(uint8_t *pData, uint16_t value) const
{
  pData[0] = (uint8_t)(value >> 8);
  pData[1] = (uint8_t)(value & 0xFF);
}

void DFRobot_C4004::writeInt16(uint8_t *pData, int16_t value) const
{
  writeUint16(pData, (uint16_t)value);
}

void DFRobot_C4004::writeSignBitInt16(uint8_t *pData, int16_t value) const
{
  int32_t  magnitude = value;
  uint16_t raw       = 0;

  if (magnitude < 0) {
    magnitude = -magnitude;
    raw       = 0x8000;
  }
  if (magnitude > 0x7FFF) {
    magnitude = 0x7FFF;
  }
  raw |= (uint16_t)magnitude;
  writeUint16(pData, raw);
}

void DFRobot_C4004::writeUint32(uint8_t *pData, uint32_t value) const
{
  pData[0] = (uint8_t)(value >> 24);
  pData[1] = (uint8_t)((value >> 16) & 0xFF);
  pData[2] = (uint8_t)((value >> 8) & 0xFF);
  pData[3] = (uint8_t)(value & 0xFF);
}
