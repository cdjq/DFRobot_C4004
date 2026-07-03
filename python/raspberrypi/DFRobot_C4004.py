# -*- coding: utf-8 -*
'''!
  @file DFRobot_C4004.py
  @brief Raspberry Pi library for the DFRobot C4004 (SKU:SEN0753).
  @details The DFRobot C4004 uses the UART frame protocol 0x53 0x59 ... checksum 0x54 0x43.
  @copyright Copyright (c) 2026 DFRobot Co.Ltd (http://www.dfrobot.com)
  @license The MIT License (MIT)
  @author JiaLi(zhixin.liu@dfrobot.com)
  @version V1.0.0
  @date 2026-05-22
  @url https://github.com/DFRobot/DFRobot_C4004
'''

import time
import serial
from collections import deque


class InstallInfo(object):
  '''! @brief Complete installation information. '''
  def __init__(self, mode=0, height_cm=0, x_angle=0, y_angle=0, z_angle=0):
    self.mode = mode
    self.height_cm = height_cm
    self.x_angle = x_angle
    self.y_angle = y_angle
    self.z_angle = z_angle


class TargetInfo(object):
  '''! @brief One tracked target information block. '''
  def __init__(self):
    self.index = 0
    self.kinesia = 0
    self.target_feature = 0
    self.x = 0
    self.y = 0
    self.height = 0
    self.speed = 0


class TagConfig(object):
  '''! @brief Tag configuration. '''
  def __init__(self):
    self.tag_index = 0
    self.tag_type = 0
    self.scope_type = 1
    self.io_index = 0
    self.center_x = 0
    self.center_y = 0
    self.width = 0
    self.height = 0


class TagInfo(object):
  '''! @brief Last tag event decoded from a report. '''
  def __init__(self):
    self.tag_index = 0
    self.tag_type = 0
    self.io_index = 0
    self.center_x = 0
    self.center_y = 0
    self.enter_exit = DFRobot_C4004.DIR_NONE
    self.motion_dir = DFRobot_C4004.DIR_NONE
    self.motion_num = 0
    self.static_num = 0


class FourSidedRange_t(object):
  '''! @brief Four-side detection boundary settings. '''
  def __init__(self):
    self.mode = 0xFF
    self.x_positive_cm = 0
    self.x_negative_cm = 0
    self.y_positive_cm = 0
    self.y_negative_cm = 0


class Point(object):
  '''! @brief One point for trajectory/config range modes. '''
  def __init__(self, x=0, y=0):
    self.x = x
    self.y = y


class Packet(object):
  '''! @brief Decoded UART packet. '''
  def __init__(self, control=0, cmd=0, data=None):
    self.control = control
    self.cmd = cmd
    self.data = bytearray(data or [])


class DFRobot_C4004(object):
  '''!
    @brief DFRobot C4004 (SKU:SEN0753) driver.
  '''

  FRAME_HEAD1 = 0x53
  FRAME_HEAD2 = 0x59
  FRAME_TAIL1 = 0x54
  FRAME_TAIL2 = 0x43
  QUERY_DATA = 0x0F
  MAX_TARGETS = 8
  MAX_POINTS = 150
  MAX_PAYLOAD = 3 + MAX_POINTS * 4
  MAX_FRAME_SIZE = 9 + MAX_PAYLOAD
  RX_RING_SIZE = MAX_FRAME_SIZE + 128
  DEFAULT_TIMEOUT = 0.2
  RESET_TIMEOUT = 0.3
  FACTORY_RESET_TIMEOUT = 0.35
  _TAG_CONFIG_LIMIT = 32

  _RX_ASM_SYNC_H1 = 0
  _RX_ASM_SYNC_H2 = 1
  _RX_ASM_CTRL = 2
  _RX_ASM_CMD = 3
  _RX_ASM_LEN_HI = 4
  _RX_ASM_LEN_LO = 5
  _RX_ASM_PAYLOAD = 6
  _RX_ASM_CHECKSUM = 7
  _RX_ASM_TAIL1 = 8
  _RX_ASM_TAIL2 = 9

  CTRL_SYSTEM = 0x01
  CTRL_PRODUCT_INFO = 0x02
  CTRL_OTA = 0x03
  CTRL_WORK_STATUS = 0x05
  CTRL_INSTALL_INFO = 0x06
  CTRL_DETECTION_RANGE = 0x07
  CTRL_PRESENCE = 0x80
  CTRL_TRAJECTORY = 0x82
  CTRL_FALL_DETECTION = 0x83
  CTRL_PEOPLE_COUNT = 0x86

  CMD_SYSTEM_HEARTBEAT_REPORT = 0x01
  CMD_SYSTEM_RESET = 0x02
  CMD_SYSTEM_FACTORY_RESET = 0x03
  CMD_SYSTEM_HEARTBEAT_QUERY = 0x80

  CMD_PRODUCT_MODEL_QUERY = 0xA1
  CMD_PRODUCT_ID_QUERY = 0xA2
  CMD_PRODUCT_HARDWARE_VERSION_QUERY = 0xA3
  CMD_PRODUCT_FIRMWARE_VERSION_QUERY = 0xA4

  CMD_WORK_STATUS_INIT_FINISHED_REPORT = 0x01
  CMD_WORK_STATUS_INIT_FINISHED_QUERY = 0x81

  CMD_INSTALL_SET_ANGLE = 0x01
  CMD_INSTALL_SET_HEIGHT = 0x02
  CMD_INSTALL_SET_MODE = 0x06
  CMD_INSTALL_QUERY_ANGLE = 0x81
  CMD_INSTALL_QUERY_HEIGHT = 0x82
  CMD_INSTALL_QUERY_MODE = 0x86

  CMD_PRESENCE_SET_ENABLE = 0x00
  CMD_PRESENCE_REPORT = 0x01
  CMD_PRESENCE_MOTION_REPORT = 0x02
  CMD_PRESENCE_QUERY_ENABLE = 0x80
  CMD_PRESENCE_QUERY_STATE = 0x81
  CMD_PRESENCE_QUERY_MOTION = 0x82

  CMD_TRAJECTORY_SET_ENABLE = 0x00
  CMD_TRAJECTORY_TARGET_REPORT = 0x02
  CMD_TRAJECTORY_QUERY_ENABLE = 0x80
  CMD_TRAJECTORY_QUERY_TARGET = 0x82
  CMD_TRAJECTORY_SET_TRAJECTORY_LED = 0x0B
  CMD_TRAJECTORY_SET_MOTION_LED = 0x0C
  CMD_TRAJECTORY_SET_CHECK_TO_ACTIVE_FRAMES = 0x0D
  CMD_TRAJECTORY_QUERY_TRAJECTORY_LED = 0x8B
  CMD_TRAJECTORY_QUERY_MOTION_LED = 0x8C
  CMD_TRAJECTORY_QUERY_CHECK_TO_ACTIVE_FRAMES = 0x8D

  CMD_DETECTION_RANGE_QUERY_TAGS = 0x91
  CMD_DETECTION_RANGE_SET_TAG = 0x11
  CMD_DETECTION_RANGE_CLEAR_TAG = 0x13
  CMD_DETECTION_RANGE_SET_TAGS_FROM_CONFIG = 0x19
  CMD_DETECTION_RANGE_SET_RANGE = 0x1A
  CMD_DETECTION_RANGE_QUERY_RANGE = 0x9A
  CMD_DETECTION_RANGE_TAG_REPORT = 0x1B

  CMD_PEOPLE_COUNT_REPORT = 0x0A
  CMD_PEOPLE_COUNT_QUERY_COUNT = 0x8A
  CMD_PEOPLE_COUNT_SET_REPORT_INTERVAL = 0x0B
  CMD_PEOPLE_COUNT_QUERY_REPORT_INTERVAL = 0x8B
  CMD_PEOPLE_COUNT_CLEAR_COUNT = 0x11
  CMD_PEOPLE_COUNT_SET_TRAJECTORY_DISTANCE = 0x0E
  CMD_PEOPLE_COUNT_QUERY_TRAJECTORY_DISTANCE = 0x8E
  CMD_PEOPLE_COUNT_SET_TRAJECTORY_HOLD_TIME = 0x15
  CMD_PEOPLE_COUNT_QUERY_TRAJECTORY_HOLD_TIME = 0x95
  CMD_PEOPLE_COUNT_SET_NO_PERSON_DELAY = 0x17
  CMD_PEOPLE_COUNT_QUERY_NO_PERSON_DELAY = 0x97

  EVENT_NONE = 0x00
  EVENT_TRAJECTORY = 0x01
  EVENT_PRESENCE = 0x02
  EVENT_MOTION = 0x03
  EVENT_TAG = 0x04
  EVENT_HEARTBEAT = 0x05
  EVENT_INIT_FINISHED = 0x06
  EVENT_PEOPLE_COUNT = 0x07
  EVENT_UNKNOWN = 0xFE
  EVENT_ERROR = 0xFF

  GET_DATA_ACTIVE = 0x00
  GET_DATA_REPORT = 0x01

  INSTALL_MODE_UNKNOWN = 0x00
  INSTALL_MODE_SIDE = 0x01
  INSTALL_MODE_TOP = 0x02

  NO_PRESENCE = 0x00
  PRESENCE = 0x01
  PRESENCE_UNKNOWN = 0xFF

  MOTION_NONE = 0x00
  MOTION_STATIC = 0x01
  MOTION_ACTIVE = 0x02
  MOTION_UNKNOWN = 0xFF

  STATIC = 0x00
  MOTION = 0x01
  UNCERTAIN = 0x02
  UNKNOWN = 0xFF

  TAG_NONE = 0x00
  TAG_BOUNDARY = 0x01
  TAG_APPROACH_AWAY = 0x02
  TAG_PEOPLE_COUNTING = 0x03
  TAG_NOISE = 0x04

  ENTER = 0x00
  EXIT = 0x01
  DIR_NONE = 0x02
  APPROACH = 0x00
  AWAY = 0x01

  TAG_RANGE_CIRCLE = 0x00
  TAG_RANGE_RECTANGLE = 0x01

  TAG_SET_COMM_ERROR = 0x00
  TAG_SET_SUCCESS = 0x01
  TAG_SET_TRACK_COUNT_ERROR = 0x02
  TAG_SET_ALREADY_USED = 0x03
  TAG_SET_INDEX_OUT_OF_RANGE = 0x04

  RANGE_SIDE_DEFAULT = 0x00
  RANGE_SIDE_LEFT_EDGE = 0x01
  RANGE_SIDE_RIGHT_EDGE = 0x02
  RANGE_HOTEL_CORRIDOR = 0x03
  RANGE_FOUR_SIDE = 0x04
  RANGE_TRAJECTORY = 0x05
  RANGE_CONFIG_FILE = 0x06
  RANGE_NO_BOUNDARY = 0x07
  RANGE_TOP_DEFAULT = 0x08
  RANGE_TOP_LEFT_EDGE = 0x09
  RANGE_TOP_RIGHT_EDGE = 0x0A
  RANGE_UNKNOWN = 0xFF

  TRAJ_RANGE_ERR_COMM = 0x00
  TRAJ_RANGE_OK = 0x01
  TRAJ_RANGE_ERR_PARAM = 0x02
  TRAJ_RANGE_ERR_MODE = 0x03
  TRAJ_RANGE_ERR_RES = 0x04
  TRAJ_RANGE_ERR_DATA = 0x05

  def __init__(self, port='/dev/ttyAMA0', baudrate=115200, timeout=0.2):
    '''!
      @brief Constructor.
      @param port Serial port path or an opened serial-like object.
      @param baudrate UART baudrate.
      @param timeout Serial timeout in seconds.
    '''
    self._port = port
    self._baudrate = baudrate
    self._timeout = timeout
    self.ser = port if hasattr(port, 'read') and hasattr(port, 'write') else None
    self._heartbeat = False
    self._init_finished = False
    self._presence_enable = 0xFF
    self._presence = self.PRESENCE_UNKNOWN
    self._motion_state = self.MOTION_UNKNOWN
    self._trajectory_led = 0xFF
    self._motion_led = 0xFF
    self._targets = []
    self._target_count = 0
    self._tag_info = TagInfo()
    self._tag_info_valid = False
    self._range_info = FourSidedRange_t()
    self._people_count = 0
    self._init_rx_state()

  def _init_rx_state(self):
    self._rx_ring = deque(maxlen=self.RX_RING_SIZE)
    self._pending_packet = None
    self._pending_valid = False
    self._reset_rx_parser()

  def begin(self):
    '''!
      @brief Open serial port and query initialization state.
      @return True if the DFRobot C4004 replied.
    '''
    if self.ser is None:
      self.ser = serial.Serial(self._port, self._baudrate, timeout=self._timeout)
    elif hasattr(self.ser, 'is_open') and self.ser.is_open is False:
      self.ser.open()

    time.sleep(0.05)
    start = time.time()
    while time.time() - start < 1.2:
      if self.is_init_finished():
        return True
      time.sleep(0.02)
    return self.is_connected()

  def close(self):
    '''! @brief Close serial port. '''
    if self.ser is not None and hasattr(self.ser, 'close'):
      self.ser.close()

  def is_init_finished(self):
    '''!
      @brief Query whether module initialization is finished.
      @return true if initialization is finished, otherwise false.
    '''
    packet = self._request_frame(self.CTRL_WORK_STATUS, self.CMD_WORK_STATUS_INIT_FINISHED_QUERY, [self.QUERY_DATA])
    if packet is not None and len(packet.data) > 0:
      self._init_finished = packet.data[0] == 0x01
    return self._init_finished

  def is_connected(self):
    '''!
      @brief Check whether the module is connected.
      @return true if connected, otherwise false.
    '''
    return self.get_heartbeat(self.GET_DATA_ACTIVE)

  def reset(self):
    '''!
      @brief Reboot the module.
      @return true if succeeded, otherwise false.
    '''
    ret = self._request_frame(self.CTRL_SYSTEM, self.CMD_SYSTEM_RESET, [self.QUERY_DATA], self.RESET_TIMEOUT) is not None
    time.sleep(0.1)
    return ret

  def factory_reset(self):
    '''!
      @brief Restore module to factory settings.
      @return true if succeeded, otherwise false.
    '''
    ret = self._request_frame(self.CTRL_SYSTEM, self.CMD_SYSTEM_FACTORY_RESET, [self.QUERY_DATA], self.FACTORY_RESET_TIMEOUT) is not None
    time.sleep(0.1)
    return ret

  def get_heartbeat(self, mode=GET_DATA_ACTIVE):
    '''!
      @brief Get heartbeat status.
      @param mode Data acquisition mode.
      @n GET_DATA_ACTIVE: Active query heartbeat.
      @n GET_DATA_REPORT: Read heartbeat from cached report status.
      @return true if heartbeat is available, otherwise false.
    '''
    if mode == self.GET_DATA_REPORT:
      return self._heartbeat
    packet = self._request_frame(self.CTRL_SYSTEM, self.CMD_SYSTEM_HEARTBEAT_QUERY, [self.QUERY_DATA])
    if packet is None:
      self._heartbeat = False
      return False
    if len(packet.data) > 0 and packet.data[0] != self.QUERY_DATA:
      self._heartbeat = False
      return False
    self._heartbeat = True
    return True

  def get_reported_info(self, timeout=0.05):
    '''!
      @brief Read and decode one reported event.
      @param timeout Maximum wait time in seconds.
      @return Event type, such as EVENT_PRESENCE, EVENT_MOTION, EVENT_TRAJECTORY, etc.
    '''
    packet = self._read_frame(timeout)
    if packet is None:
      return self.EVENT_NONE
    return self._handle_packet(packet)

  def get_hardware_version(self):
    '''!
      @brief Query hardware version string.
      @return Hardware version string.
    '''
    return self._query_string(self.CTRL_PRODUCT_INFO, self.CMD_PRODUCT_HARDWARE_VERSION_QUERY)

  def get_firmware_version(self):
    '''!
      @brief Query firmware version string.
      @return Firmware version string.
    '''
    return self._query_string(self.CTRL_PRODUCT_INFO, self.CMD_PRODUCT_FIRMWARE_VERSION_QUERY)

  def set_install_info(self, info):
    '''!
      @brief Set installation information.
      @param info InstallInfo object.
      @n info.mode: INSTALL_MODE_SIDE or INSTALL_MODE_TOP.
      @n info.height_cm: Installation height in cm, valid range 0-65535.
      @n info.x_angle/y_angle/z_angle: Installation angles in degree, valid range -180 to 180.
      @return true if succeeded, otherwise false.
      @note Invalid mode or height returns false. Out-of-range angles are clamped.
    '''
    if info.mode not in (self.INSTALL_MODE_SIDE, self.INSTALL_MODE_TOP):
      return False
    height_cm = int(info.height_cm)
    if height_cm < 0 or height_cm > 0xFFFF:
      return False
    x_angle = int(info.x_angle) * 100
    y_angle = int(info.y_angle) * 100
    z_angle = int(info.z_angle) * 100
    x_angle = max(-18000, min(18000, x_angle))
    y_angle = max(-18000, min(18000, y_angle))
    z_angle = max(-18000, min(18000, z_angle))

    angle = self._i16_bytes(x_angle) + self._i16_bytes(y_angle) + self._i16_bytes(z_angle)
    height = self._u16_bytes(height_cm)
    return (self._request_frame(self.CTRL_INSTALL_INFO, self.CMD_INSTALL_SET_MODE, [info.mode]) is not None and
            self._request_frame(self.CTRL_INSTALL_INFO, self.CMD_INSTALL_SET_ANGLE, angle) is not None and
            self._request_frame(self.CTRL_INSTALL_INFO, self.CMD_INSTALL_SET_HEIGHT, height) is not None)

  def get_install_info(self, info):
    '''!
      @brief Query installation information.
      @param info InstallInfo object to receive result.
      @return true if succeeded, otherwise false.
      @note Returned angle values are degree.
    '''
    if info is None:
      return False
    info.mode = self.INSTALL_MODE_UNKNOWN
    info.height_cm = 0
    info.x_angle = 0
    info.y_angle = 0
    info.z_angle = 0

    packet = self._request_frame(self.CTRL_INSTALL_INFO, self.CMD_INSTALL_QUERY_ANGLE, [self.QUERY_DATA])
    if packet is None or len(packet.data) < 6:
      return False
    info.x_angle = int(self._i16(packet.data, 0) / 100)
    info.y_angle = int(self._i16(packet.data, 2) / 100)
    info.z_angle = int(self._i16(packet.data, 4) / 100)

    packet = self._request_frame(self.CTRL_INSTALL_INFO, self.CMD_INSTALL_QUERY_HEIGHT, [self.QUERY_DATA])
    if packet is None or len(packet.data) < 2:
      return False
    info.height_cm = self._u16(packet.data, 0)

    packet = self._request_frame(self.CTRL_INSTALL_INFO, self.CMD_INSTALL_QUERY_MODE, [self.QUERY_DATA])
    if packet is None or len(packet.data) < 1:
      return False
    info.mode = packet.data[0]
    return True

  def set_install_high(self, hight):
    '''!
      @brief Set installation height.
      @param hight Installation height in cm.
      @return true if succeeded, otherwise false.
    '''
    hight = int(hight)
    if hight < 0 or hight > 0xFFFF:
      return False
    return self._request_frame(self.CTRL_INSTALL_INFO, self.CMD_INSTALL_SET_HEIGHT, self._u16_bytes(hight)) is not None

  def get_install_high(self):
    '''!
      @brief Query installation height.
      @return Installation height in cm. Returns 0 on failure.
    '''
    packet = self._request_frame(self.CTRL_INSTALL_INFO, self.CMD_INSTALL_QUERY_HEIGHT, [self.QUERY_DATA])
    if packet is None or len(packet.data) < 2:
      return 0
    return self._u16(packet.data, 0)

  def set_presence_enable(self, enable):
    '''!
      @brief Enable or disable presence detection.
      @param enable true to enable, false to disable.
      @return true if succeeded, otherwise false.
    '''
    ret = self._set_byte(self.CTRL_PRESENCE, self.CMD_PRESENCE_SET_ENABLE, 1 if enable else 0)
    if ret:
      self._presence_enable = 1 if enable else 0
    return ret

  def get_presence_enable(self, enable):
    '''!
      @brief Query whether presence detection is enabled.
      @param enable Output container for bool state.
      @n Supported output containers: list/dict/object(with value field).
      @return true if succeeded, otherwise false.
    '''
    if enable is None:
      return False
    value = self._query_byte(self.CTRL_PRESENCE, self.CMD_PRESENCE_QUERY_ENABLE)
    if value is None:
      return False
    self._presence_enable = value
    state = value != 0
    if isinstance(enable, list):
      if len(enable) == 0:
        enable.append(state)
      else:
        enable[0] = state
      return True
    if isinstance(enable, dict):
      enable['value'] = state
      return True
    if hasattr(enable, 'value'):
      enable.value = state
      return True
    return False

  def get_presence_state(self, mode=GET_DATA_ACTIVE):
    '''!
      @brief Get presence state.
      @param mode Data acquisition mode.
      @n GET_DATA_ACTIVE: Query latest presence state before reading cache.
      @n GET_DATA_REPORT: Read from report cache only.
      @return NO_PRESENCE / PRESENCE / PRESENCE_UNKNOWN.
    '''
    if isinstance(mode, bool):
      mode = self.GET_DATA_ACTIVE if mode else self.GET_DATA_REPORT
    if mode == self.GET_DATA_ACTIVE:
      value = self._query_byte(self.CTRL_PRESENCE, self.CMD_PRESENCE_QUERY_STATE)
      if value is not None:
        self._presence = value
    return self._presence

  def get_motion_state(self, mode=GET_DATA_ACTIVE):
    '''!
      @brief Get motion state.
      @param mode Data acquisition mode.
      @n GET_DATA_ACTIVE: Query latest motion state before reading cache.
      @n GET_DATA_REPORT: Read from report cache only.
      @return MOTION_NONE / MOTION_STATIC / MOTION_ACTIVE / MOTION_UNKNOWN.
    '''
    if isinstance(mode, bool):
      mode = self.GET_DATA_ACTIVE if mode else self.GET_DATA_REPORT
    if mode == self.GET_DATA_ACTIVE:
      value = self._query_byte(self.CTRL_PRESENCE, self.CMD_PRESENCE_QUERY_MOTION)
      if value is not None:
        self._motion_state = value
    return self._motion_state

  def set_trajectory_track_enable(self, enable):
    '''!
      @brief Enable or disable trajectory tracking function.
      @param enable true to enable, false to disable.
      @return true if succeeded, otherwise false.
    '''
    return self._set_byte(self.CTRL_TRAJECTORY, self.CMD_TRAJECTORY_SET_ENABLE, 1 if enable else 0)

  def get_trajectory_track_enable(self, enable):
    '''!
      @brief Query trajectory tracking enable state.
      @param enable Output container for bool state.
      @n Supported output containers: list/dict/object(with value field).
      @return true if succeeded, otherwise false.
    '''
    if enable is None:
      return False
    value = self._query_byte(self.CTRL_TRAJECTORY, self.CMD_TRAJECTORY_QUERY_ENABLE)
    if value is None:
      return False
    state = value != 0
    if isinstance(enable, list):
      if len(enable) == 0:
        enable.append(state)
      else:
        enable[0] = state
      return True
    if isinstance(enable, dict):
      enable['value'] = state
      return True
    if hasattr(enable, 'value'):
      enable.value = state
      return True
    return False

  def set_check_to_active_frames(self, frames):
    '''!
      @brief Set the frame count used to confirm transition from check state to active state.
      @param frames Frame count, valid range: 1-7.
      @return true if succeeded, otherwise false.
    '''
    frames = int(frames)
    if frames < 1 or frames > 7:
      return False
    return self._set_byte(self.CTRL_TRAJECTORY, self.CMD_TRAJECTORY_SET_CHECK_TO_ACTIVE_FRAMES, frames)

  def get_check_to_active_frames(self, frames):
    '''!
      @brief Query the frame count used to confirm transition from check state to active state.
      @param frames Output container for frame count.
      @n Supported output containers: list/dict/object(with value field).
      @return true if succeeded, otherwise false.
    '''
    if frames is None:
      return False
    value = self._query_byte(self.CTRL_TRAJECTORY, self.CMD_TRAJECTORY_QUERY_CHECK_TO_ACTIVE_FRAMES)
    if value is None:
      return False
    return self._set_output_value(frames, value)

  def get_target_list(self, mode=GET_DATA_ACTIVE):
    '''!
      @brief Get target list.
      @param mode Data acquisition mode.
      @n GET_DATA_ACTIVE: Query latest target list before reading cache.
      @n GET_DATA_REPORT: Read from report cache only.
      @return List of TargetInfo objects.
    '''
    if isinstance(mode, bool):
      mode = self.GET_DATA_ACTIVE if mode else self.GET_DATA_REPORT
    if mode == self.GET_DATA_ACTIVE:
      self._request_frame(self.CTRL_TRAJECTORY, self.CMD_TRAJECTORY_QUERY_TARGET, [self.QUERY_DATA])
    return list(self._targets)

  def set_trajectory_led(self, enable):
    '''!
      @brief Enable or disable trajectory LED.
      @param enable true to enable, false to disable.
      @return true if succeeded, otherwise false.
    '''
    ret = self._set_byte(self.CTRL_TRAJECTORY, self.CMD_TRAJECTORY_SET_TRAJECTORY_LED, 1 if enable else 0)
    if ret:
      self._trajectory_led = 1 if enable else 0
    return ret

  def set_motion_led(self, enable):
    '''!
      @brief Enable or disable motion LED.
      @param enable true to enable, false to disable.
      @return true if succeeded, otherwise false.
    '''
    ret = self._set_byte(self.CTRL_TRAJECTORY, self.CMD_TRAJECTORY_SET_MOTION_LED, 1 if enable else 0)
    if ret:
      self._motion_led = 1 if enable else 0
    return ret

  def get_trajectory_led(self):
    '''!
      @brief Query trajectory LED status.
      @return true if enabled, false if disabled.
    '''
    value = self._query_byte(self.CTRL_TRAJECTORY, self.CMD_TRAJECTORY_QUERY_TRAJECTORY_LED)
    if value is not None:
      self._trajectory_led = value
    return self._trajectory_led != 0

  def get_motion_led(self):
    '''!
      @brief Query motion LED status.
      @return true if enabled, false if disabled.
    '''
    value = self._query_byte(self.CTRL_TRAJECTORY, self.CMD_TRAJECTORY_QUERY_MOTION_LED)
    if value is not None:
      self._motion_led = value
    return self._motion_led != 0

  def get_tags(self, mode=GET_DATA_ACTIVE, max_tags=None):
    '''!
      @brief Get all tag configurations from the device.
      @param mode Data acquisition mode kept for compatibility.
      @n GET_DATA_ACTIVE: Active query.
      @n GET_DATA_REPORT: Currently behaves the same as GET_DATA_ACTIVE.
      @param max_tags Maximum number of tags to return. None returns all parsed tags.
      @return List of TagConfig objects.
    '''
    if isinstance(mode, bool):
      mode = self.GET_DATA_ACTIVE if mode else self.GET_DATA_REPORT
    packet = self._request_frame(self.CTRL_DETECTION_RANGE, self.CMD_DETECTION_RANGE_QUERY_TAGS, [self.QUERY_DATA])
    if packet is None:
      return []
    tags, _ = self._parse_tag_list(packet.data, max_tags)
    return tags

  def _is_valid_tag_config(self, tag):
    '''!
      @brief Validate tag type, range shape and IO linkage index.
      @param tag TagConfig object.
      @return True if the tag config is valid, otherwise False.
      @note io_index is valid only when it is 0 (unused) or within 2-6 (IO2-IO6).
    '''
    if not (self.TAG_NONE <= tag.tag_type <= self.TAG_NOISE):
      return False
    if not (self.TAG_RANGE_CIRCLE <= tag.scope_type <= self.TAG_RANGE_RECTANGLE):
      return False
    if tag.io_index == 1 or tag.io_index < 0 or tag.io_index > 6:
      return False
    return True

  def set_tag(self, tag):
    '''!
      @brief Set one tag using size mode.
      @param tag TagConfig object.
      @return Tag set status code:
      @n TAG_SET_COMM_ERROR / TAG_SET_SUCCESS / TAG_SET_TRACK_COUNT_ERROR / TAG_SET_ALREADY_USED / TAG_SET_INDEX_OUT_OF_RANGE.
      @note center_x/center_y fields are ignored by this API.
      @note Invalid tag_type, scope_type or io_index returns TAG_SET_COMM_ERROR before sending command.
    '''
    if not self._is_valid_tag_config(tag):
      return self.TAG_SET_COMM_ERROR
    data = [tag.tag_index, tag.tag_type, tag.scope_type, tag.io_index]
    data += self._u16_bytes(tag.width)
    data += self._u16_bytes(tag.height)
    packet = self._request_frame(self.CTRL_DETECTION_RANGE, self.CMD_DETECTION_RANGE_SET_TAG, data)
    if packet is None or len(packet.data) < 4:
      return self.TAG_SET_COMM_ERROR
    if packet.data[0] != (tag.tag_index & 0xFF):
      return self.TAG_SET_COMM_ERROR
    status = packet.data[3]
    if self.TAG_SET_SUCCESS <= status <= self.TAG_SET_INDEX_OUT_OF_RANGE:
      return status
    return self.TAG_SET_COMM_ERROR

  def clear_tag(self, tag_index):
    '''!
      @brief Clear one tag by tag index.
      @param tag_index Tag index (2-byte index in protocol payload).
      @return true if succeeded, otherwise false.
    '''
    packet = self._request_frame(self.CTRL_DETECTION_RANGE, self.CMD_DETECTION_RANGE_CLEAR_TAG, self._u16_bytes(tag_index))
    if packet is None:
      return False
    if len(packet.data) > 0 and packet.data[0] == 0xFE:
      return False
    if len(packet.data) < 2:
      return False
    if self._u16(packet.data, 0) != (tag_index & 0xFFFF):
      return False
    return True

  def clear_all_tags(self):
    '''!
      @brief Clear all tags.
      @return true if succeeded, otherwise false.
    '''
    packet = self._request_frame(self.CTRL_DETECTION_RANGE, self.CMD_DETECTION_RANGE_CLEAR_TAG, [0xFF])
    if packet is None:
      return False
    if len(packet.data) > 0:
      if packet.data[0] == 0xFE:
        return False
      if packet.data[0] != 0xFF:
        return False
    return True

  def set_tags_from_config(self, tags):
    '''!
      @brief Set tag configurations in coordinate mode.
      @param tags Iterable of TagConfig objects.
      @return true if succeeded, otherwise false.
      @note The number of input tags is limited to 32 by protocol.
    '''
    if tags is None:
      return False
    tags = list(tags)
    if len(tags) > self._TAG_CONFIG_LIMIT:
      return False
    expected_len = 2 + len(tags) * 12
    if expected_len > self.MAX_PAYLOAD:
      return False
    for tag in tags:
      if not self._is_valid_tag_config(tag):
        return False
    data = self._u16_bytes(len(tags))
    for tag in tags:
      data += [tag.tag_index, tag.tag_type, tag.scope_type, tag.io_index]
      data += self._sb16_bytes(tag.center_x)
      data += self._sb16_bytes(tag.center_y)
      data += self._u16_bytes(tag.width)
      data += self._u16_bytes(tag.height)
    packet = self._request_frame(self.CTRL_DETECTION_RANGE, self.CMD_DETECTION_RANGE_SET_TAGS_FROM_CONFIG, data)
    if packet is None:
      return False
    if len(packet.data) != expected_len:
      return False
    if self._u16(packet.data, 0) != len(tags):
      return False
    return True

  def get_tag_info(self):
    '''!
      @brief Get the last tag event decoded from active report packet (CTRL 0x07, CMD 0x1B).
      @return TagInfo object if valid, otherwise None.
      @note This API reads report cache only. Call get_reported_info() first to receive new report data.
    '''
    if not self._tag_info_valid:
      return None
    return self._tag_info

  def set_four_sided_range_mode(self, range_info):
    '''!
      @brief Set four-side boundary detection range (mode 0x04).
      @param range_info FourSidedRange_t object.
      @return true if succeeded, otherwise false.
      @note Position values use sign-bit int16 encoding (bit15: 0=positive, 1=negative).
    '''
    data = [self.RANGE_FOUR_SIDE]
    data += self._sb16_bytes(range_info.x_positive_cm)
    data += self._sb16_bytes(range_info.x_negative_cm)
    data += self._sb16_bytes(range_info.y_positive_cm)
    data += self._sb16_bytes(range_info.y_negative_cm)
    ret = self._request_frame(self.CTRL_DETECTION_RANGE, self.CMD_DETECTION_RANGE_SET_RANGE, data) is not None
    if ret:
      # Keep local cache as value-copy to match C++ behavior.
      self._range_info.mode = self.RANGE_FOUR_SIDE
      self._range_info.x_positive_cm = range_info.x_positive_cm
      self._range_info.x_negative_cm = range_info.x_negative_cm
      self._range_info.y_positive_cm = range_info.y_positive_cm
      self._range_info.y_negative_cm = range_info.y_negative_cm
    return ret

  def get_four_sided_range_mode(self, range_info):
    '''!
      @brief Query and get four-side boundary detection range.
      @param range_info FourSidedRange_t object to receive result.
      @return true if succeeded, otherwise false.
    '''
    if range_info is None:
      return False
    packet = self._request_frame(self.CTRL_DETECTION_RANGE, self.CMD_DETECTION_RANGE_QUERY_RANGE, [self.QUERY_DATA])
    if packet is None:
      return False
    range_info.mode = self._range_info.mode
    range_info.x_positive_cm = self._range_info.x_positive_cm
    range_info.x_negative_cm = self._range_info.x_negative_cm
    range_info.y_positive_cm = self._range_info.y_positive_cm
    range_info.y_negative_cm = self._range_info.y_negative_cm
    return True

  def set_trajectory_range_mode(self, learning):
    '''!
      @brief Start trajectory-range learning or use the learned trajectory range (mode 0x05).
      @param learning True starts trajectory-range learning; False uses trajectory range mode without learning.
    '''
    data = [self.RANGE_TRAJECTORY, 1 if learning else 0]
    self._request_frame(self.CTRL_DETECTION_RANGE, self.CMD_DETECTION_RANGE_SET_RANGE, data)
    self._range_info.mode = self.RANGE_TRAJECTORY

  def set_config_file_mode_points(self, points):
    '''!
      @brief Set detection range points in config-file mode (mode 0x06).
      @param points Iterable of Point objects.
      @return true if succeeded, otherwise false.
      @note Payload format: 0x06 + 2B point_count + n*(2B X + 2B Y).
      @note Position values use sign-bit int16 encoding (bit15: 0=positive, 1=negative).
    '''
    if points is None:
      return False

    points = list(points)[:self.MAX_POINTS]
    data = [self.RANGE_CONFIG_FILE]
    data += self._u16_bytes(len(points))
    for point in points:
      data += self._sb16_bytes(getattr(point, 'x', 0))
      data += self._sb16_bytes(getattr(point, 'y', 0))

    packet = self._request_frame(self.CTRL_DETECTION_RANGE, self.CMD_DETECTION_RANGE_SET_RANGE, data)
    if packet is None or len(packet.data) < 3:
      return False
    if packet.data[0] != self.RANGE_CONFIG_FILE:
      return False

    resp_count = self._u16(packet.data, 1)
    if resp_count != len(points):
      return False
    if len(packet.data) < 3 + resp_count * 4:
      return False

    self._range_info.mode = self.RANGE_CONFIG_FILE
    return True

  def get_detection_range_mode(self):
    '''!
      @brief Query current detection range mode.
      @return Current mode value.
    '''
    self._request_frame(self.CTRL_DETECTION_RANGE, self.CMD_DETECTION_RANGE_QUERY_RANGE, [self.QUERY_DATA])
    return self._range_info.mode

  def get_trajectory_range_mode(self, points, point_count):
    '''!
      @brief Query and get trajectory detection range points (mode 0x05).
      @param points List used to receive Point objects.
      @param point_count Output container for point count.
      @n Supported output containers: list/dict/object(with value field).
      @return true if succeeded, otherwise false.
    '''
    if points is None or point_count is None:
      return False
    if not isinstance(points, list):
      return False

    packet = self._request_frame(self.CTRL_DETECTION_RANGE, self.CMD_DETECTION_RANGE_QUERY_RANGE, [self.QUERY_DATA])
    if packet is None:
      return False
    if len(packet.data) < 3:
      return False
    if packet.data[0] != self.RANGE_TRAJECTORY:
      return False

    count = self._u16(packet.data, 1)
    if count > self.MAX_POINTS:
      return False
    if len(packet.data) < 3 + count * 4:
      return False

    points[:] = []
    for i in range(count):
      offset = 3 + i * 4
      points.append(Point(self._sb16(packet.data, offset), self._sb16(packet.data, offset + 2)))

    if isinstance(point_count, list):
      if len(point_count) == 0:
        point_count.append(count)
      else:
        point_count[0] = count
      return True
    if isinstance(point_count, dict):
      point_count['value'] = count
      return True
    if hasattr(point_count, 'value'):
      point_count.value = count
      return True
    return False

  def get_config_file_mode_points(self, points, point_count):
    '''!
      @brief Query and get config-file mode points (mode 0x06).
      @param points List used to receive Point objects.
      @param point_count Output container for point count.
      @n Supported output containers: list/dict/object(with value field).
      @return true if succeeded, otherwise false.
    '''
    if points is None or point_count is None:
      return False
    if not isinstance(points, list):
      return False

    packet = self._request_frame(self.CTRL_DETECTION_RANGE, self.CMD_DETECTION_RANGE_QUERY_RANGE, [self.QUERY_DATA])
    if packet is None:
      return False
    if len(packet.data) < 3:
      return False
    if packet.data[0] != self.RANGE_CONFIG_FILE:
      return False

    count = self._u16(packet.data, 1)
    if count > self.MAX_POINTS:
      return False
    if len(packet.data) < 3 + count * 4:
      return False

    points[:] = []
    for i in range(count):
      offset = 3 + i * 4
      points.append(Point(self._sb16(packet.data, offset), self._sb16(packet.data, offset + 2)))

    if isinstance(point_count, list):
      if len(point_count) == 0:
        point_count.append(count)
      else:
        point_count[0] = count
      return True
    if isinstance(point_count, dict):
      point_count['value'] = count
      return True
    if hasattr(point_count, 'value'):
      point_count.value = count
      return True
    return False

  def get_people_time(self, mode=GET_DATA_ACTIVE):
    '''!
      @brief Get people count information.
      @param mode Data acquisition mode.
      @n GET_DATA_ACTIVE: Query latest people count before reading cache.
      @n GET_DATA_REPORT: Read from report cache only.
      @return Current people count value.
    '''
    if isinstance(mode, bool):
      mode = self.GET_DATA_ACTIVE if mode else self.GET_DATA_REPORT
    if mode == self.GET_DATA_ACTIVE:
      self._request_frame(self.CTRL_PEOPLE_COUNT, self.CMD_PEOPLE_COUNT_QUERY_COUNT, [self.QUERY_DATA])
    return self._people_count

  def set_real_time_people_time(self, interval):
    '''!
      @brief Set people count report interval.
      @param interval Interval in seconds.
      @return true if succeeded, otherwise false.
    '''
    return self._set_u32(self.CTRL_PEOPLE_COUNT, self.CMD_PEOPLE_COUNT_SET_REPORT_INTERVAL, interval)

  def get_real_time_people_time(self):
    '''!
      @brief Query people count report interval.
      @return Interval in seconds. Returns 0 on failure.
    '''
    return self._query_u32(self.CTRL_PEOPLE_COUNT, self.CMD_PEOPLE_COUNT_QUERY_REPORT_INTERVAL)

  def clear_people_count(self):
    '''!
      @brief Clear people count statistics.
      @return true if succeeded, otherwise false.
    '''
    return self._request_frame(self.CTRL_PEOPLE_COUNT, self.CMD_PEOPLE_COUNT_CLEAR_COUNT, [self.QUERY_DATA]) is not None

  def set_track_meters(self, distance_cm):
    '''!
      @brief Set trajectory generation distance threshold.
      @param distance_cm Distance threshold in cm.
      @return true if succeeded, otherwise false.
    '''
    return self._set_u32(self.CTRL_PEOPLE_COUNT, self.CMD_PEOPLE_COUNT_SET_TRAJECTORY_DISTANCE, distance_cm)

  def get_track_meters(self):
    '''!
      @brief Query trajectory generation distance threshold.
      @return Distance threshold in cm. Returns 0 on failure.
    '''
    return self._query_u32(self.CTRL_PEOPLE_COUNT, self.CMD_PEOPLE_COUNT_QUERY_TRAJECTORY_DISTANCE)

  def set_track_exists_time(self, time):
    '''!
      @brief Set trajectory hold time.
      @param time Hold time in seconds.
      @return true if succeeded, otherwise false.
    '''
    return self._set_u32(self.CTRL_PEOPLE_COUNT, self.CMD_PEOPLE_COUNT_SET_TRAJECTORY_HOLD_TIME, time)

  def get_track_exists_time(self):
    '''!
      @brief Query trajectory hold time.
      @return Hold time in seconds. Returns 0 on failure.
    '''
    return self._query_u32(self.CTRL_PEOPLE_COUNT, self.CMD_PEOPLE_COUNT_QUERY_TRAJECTORY_HOLD_TIME)

  def set_unmanned_time(self, delay_time):
    '''!
      @brief Set no-person delay time.
      @param delay_time Delay time in seconds.
      @return true if succeeded, otherwise false.
    '''
    return self._set_u32(self.CTRL_PEOPLE_COUNT, self.CMD_PEOPLE_COUNT_SET_NO_PERSON_DELAY, delay_time)

  def get_unmanned_time(self):
    '''!
      @brief Query no-person delay time.
      @return Delay time in seconds. Returns 0 on failure.
    '''
    return self._query_u32(self.CTRL_PEOPLE_COUNT, self.CMD_PEOPLE_COUNT_QUERY_NO_PERSON_DELAY)

  def _query_byte(self, control, cmd):
    packet = self._request_frame(control, cmd, [self.QUERY_DATA])
    if packet is None or len(packet.data) < 1:
      return None
    return packet.data[0]

  def _set_byte(self, control, cmd, value):
    return self._request_frame(control, cmd, [value & 0xFF]) is not None

  def _query_u32(self, control, cmd):
    packet = self._request_frame(control, cmd, [self.QUERY_DATA])
    if packet is None or len(packet.data) < 4:
      return 0
    return self._u32(packet.data, 0)

  def _set_u32(self, control, cmd, value):
    return self._request_frame(control, cmd, self._u32_bytes(value)) is not None

  def _query_string(self, control, cmd):
    packet = self._request_frame(control, cmd, [self.QUERY_DATA])
    if packet is None:
      return ''
    return ''.join(chr(b) for b in packet.data if b != 0)

  def _get_product_model(self):
    return self._query_string(self.CTRL_PRODUCT_INFO, self.CMD_PRODUCT_MODEL_QUERY)

  def _get_product_id(self):
    packet = self._request_frame(self.CTRL_PRODUCT_INFO, self.CMD_PRODUCT_ID_QUERY, [self.QUERY_DATA])
    if packet is None or len(packet.data) == 0:
      return 0
    if len(packet.data) >= 2:
      return self._u16(packet.data, 0)
    if len(packet.data) == 1:
      return packet.data[0]
    return 0

  def _set_output_value(self, output, value):
    if isinstance(output, list):
      if len(output) == 0:
        output.append(value)
      else:
        output[0] = value
      return True
    if isinstance(output, dict):
      output['value'] = value
      return True
    if hasattr(output, 'value'):
      output.value = value
      return True
    return False

  def _reset_rx_parser(self):
    self._asm_state = self._RX_ASM_SYNC_H1
    self._asm_control = 0
    self._asm_cmd = 0
    self._asm_len = 0
    self._asm_data = bytearray()
    self._asm_checksum = 0
    self._asm_idx = 0
    self._asm_recv_checksum = 0

  def _discard_rx_ring(self):
    self._rx_ring.clear()

  def _feed_asm_byte(self, value):
    if self._asm_state == self._RX_ASM_SYNC_H1:
      if value == self.FRAME_HEAD1:
        self._asm_checksum = value
        self._asm_state = self._RX_ASM_SYNC_H2
    elif self._asm_state == self._RX_ASM_SYNC_H2:
      if value == self.FRAME_HEAD2:
        self._asm_checksum = (self._asm_checksum + value) & 0xFF
        self._asm_state = self._RX_ASM_CTRL
      else:
        self._asm_state = self._RX_ASM_SYNC_H1
        if value == self.FRAME_HEAD1:
          self._asm_checksum = value
          self._asm_state = self._RX_ASM_SYNC_H2
    elif self._asm_state == self._RX_ASM_CTRL:
      self._asm_control = value
      self._asm_checksum = (self._asm_checksum + value) & 0xFF
      self._asm_state = self._RX_ASM_CMD
    elif self._asm_state == self._RX_ASM_CMD:
      self._asm_cmd = value
      self._asm_checksum = (self._asm_checksum + value) & 0xFF
      self._asm_state = self._RX_ASM_LEN_HI
    elif self._asm_state == self._RX_ASM_LEN_HI:
      self._asm_len = (value << 8)
      self._asm_checksum = (self._asm_checksum + value) & 0xFF
      self._asm_state = self._RX_ASM_LEN_LO
    elif self._asm_state == self._RX_ASM_LEN_LO:
      self._asm_len |= value
      self._asm_checksum = (self._asm_checksum + value) & 0xFF
      if self._asm_len > self.MAX_PAYLOAD:
        self._reset_rx_parser()
        self._discard_rx_ring()
        return
      self._asm_data = bytearray()
      self._asm_idx = 0
      if self._asm_len == 0:
        self._asm_state = self._RX_ASM_CHECKSUM
      else:
        self._asm_state = self._RX_ASM_PAYLOAD
    elif self._asm_state == self._RX_ASM_PAYLOAD:
      self._asm_data.append(value)
      self._asm_checksum = (self._asm_checksum + value) & 0xFF
      self._asm_idx += 1
      if self._asm_idx >= self._asm_len:
        self._asm_state = self._RX_ASM_CHECKSUM
    elif self._asm_state == self._RX_ASM_CHECKSUM:
      self._asm_recv_checksum = value
      self._asm_state = self._RX_ASM_TAIL1
    elif self._asm_state == self._RX_ASM_TAIL1:
      if value != self.FRAME_TAIL1:
        self._reset_rx_parser()
        return
      self._asm_state = self._RX_ASM_TAIL2
    elif self._asm_state == self._RX_ASM_TAIL2:
      if value != self.FRAME_TAIL2:
        self._reset_rx_parser()
        return
      if self._asm_checksum != self._asm_recv_checksum:
        self._reset_rx_parser()
        return
      self._pending_packet = Packet(self._asm_control, self._asm_cmd, self._asm_data)
      self._pending_valid = True
      self._reset_rx_parser()

  def _pump_rx(self):
    if self.ser is None:
      return
    if hasattr(self.ser, 'in_waiting') and self.ser.in_waiting:
      self._rx_ring.extend(self.ser.read(self.ser.in_waiting))
    while (not self._pending_valid) and self._rx_ring:
      value = self._rx_ring.popleft()
      self._feed_asm_byte(value)

  def _take_pending_frame(self):
    if not self._pending_valid:
      return None
    packet = self._pending_packet
    self._pending_valid = False
    self._pending_packet = None
    return packet

  def _flush_input(self):
    self._discard_rx_ring()
    self._reset_rx_parser()
    self._pending_valid = False
    self._pending_packet = None
    if self.ser is None:
      return
    if hasattr(self.ser, 'reset_input_buffer'):
      self.ser.reset_input_buffer()
      return
    if hasattr(self.ser, 'in_waiting'):
      while self.ser.in_waiting:
        self.ser.read(self.ser.in_waiting)

  def _send_command(self, control, cmd, data):
    if self.ser is None:
      return False
    data = bytearray(data or [])
    length = len(data)
    frame = bytearray([self.FRAME_HEAD1, self.FRAME_HEAD2, control & 0xFF, cmd & 0xFF,
                       (length >> 8) & 0xFF, length & 0xFF])
    frame += data
    checksum = sum(frame) & 0xFF
    frame += bytearray([checksum, self.FRAME_TAIL1, self.FRAME_TAIL2])
    self.ser.write(frame)
    return True

  def _request_frame(self, control, cmd, data, timeout=None):
    if timeout is None:
      timeout = self.DEFAULT_TIMEOUT
    if not self._send_command(control, cmd, data):
      return None
    start = time.time()
    while time.time() - start < timeout:
      packet = self._read_frame(max(0.01, timeout - (time.time() - start)))
      if packet is None:
        continue
      self._handle_packet(packet)
      if packet.control == control and packet.cmd == cmd:
        return packet
    return None

  def _read_frame(self, timeout=0.05):
    start = time.time()
    while time.time() - start < timeout:
      self._pump_rx()
      packet = self._take_pending_frame()
      if packet is not None:
        return packet
      time.sleep(0.001)
    return None

  def _handle_packet(self, packet):
    if packet.control == self.CTRL_SYSTEM and packet.cmd in (self.CMD_SYSTEM_HEARTBEAT_REPORT, self.CMD_SYSTEM_HEARTBEAT_QUERY):
      self._heartbeat = True
    elif packet.control == self.CTRL_WORK_STATUS and packet.cmd in (self.CMD_WORK_STATUS_INIT_FINISHED_REPORT, self.CMD_WORK_STATUS_INIT_FINISHED_QUERY):
      if len(packet.data) > 0:
        self._init_finished = (packet.data[0] == 0x01) or (packet.cmd == self.CMD_WORK_STATUS_INIT_FINISHED_REPORT)
    elif packet.control == self.CTRL_PRESENCE and packet.cmd == self.CMD_PRESENCE_QUERY_ENABLE and len(packet.data) > 0:
      self._presence_enable = packet.data[0]
    elif packet.control == self.CTRL_PRESENCE and packet.cmd in (self.CMD_PRESENCE_REPORT, self.CMD_PRESENCE_QUERY_STATE) and len(packet.data) > 0:
      self._presence = packet.data[0]
    elif packet.control == self.CTRL_PRESENCE and packet.cmd in (self.CMD_PRESENCE_MOTION_REPORT, self.CMD_PRESENCE_QUERY_MOTION) and len(packet.data) > 0:
      self._motion_state = packet.data[0]
    elif packet.control == self.CTRL_TRAJECTORY and packet.cmd in (self.CMD_TRAJECTORY_TARGET_REPORT, self.CMD_TRAJECTORY_QUERY_TARGET):
      self._parse_targets(packet.data)
    elif packet.control == self.CTRL_TRAJECTORY and packet.cmd == self.CMD_TRAJECTORY_QUERY_TRAJECTORY_LED and len(packet.data) > 0:
      self._trajectory_led = packet.data[0]
    elif packet.control == self.CTRL_TRAJECTORY and packet.cmd == self.CMD_TRAJECTORY_QUERY_MOTION_LED and len(packet.data) > 0:
      self._motion_led = packet.data[0]
    elif packet.control == self.CTRL_DETECTION_RANGE and packet.cmd == self.CMD_DETECTION_RANGE_TAG_REPORT:
      self._parse_tag_event(packet.data)
    elif packet.control == self.CTRL_DETECTION_RANGE and packet.cmd == self.CMD_DETECTION_RANGE_QUERY_RANGE:
      self._parse_boundary_range(packet.data)
    elif packet.control == self.CTRL_PEOPLE_COUNT and packet.cmd in (self.CMD_PEOPLE_COUNT_REPORT, self.CMD_PEOPLE_COUNT_QUERY_COUNT):
      self._parse_people_count(packet.data)
    return self._classify_packet(packet)

  def _classify_packet(self, packet):
    if packet.control == self.CTRL_SYSTEM and packet.cmd in (self.CMD_SYSTEM_HEARTBEAT_REPORT, self.CMD_SYSTEM_HEARTBEAT_QUERY):
      return self.EVENT_HEARTBEAT
    if packet.control == self.CTRL_WORK_STATUS and packet.cmd == self.CMD_WORK_STATUS_INIT_FINISHED_REPORT:
      return self.EVENT_INIT_FINISHED
    if packet.control == self.CTRL_PRESENCE and packet.cmd in (self.CMD_PRESENCE_REPORT, self.CMD_PRESENCE_QUERY_STATE):
      return self.EVENT_PRESENCE
    if packet.control == self.CTRL_PRESENCE and packet.cmd in (self.CMD_PRESENCE_MOTION_REPORT, self.CMD_PRESENCE_QUERY_MOTION):
      return self.EVENT_MOTION
    if packet.control == self.CTRL_TRAJECTORY and packet.cmd in (self.CMD_TRAJECTORY_TARGET_REPORT, self.CMD_TRAJECTORY_QUERY_TARGET):
      return self.EVENT_TRAJECTORY
    if packet.control == self.CTRL_DETECTION_RANGE and packet.cmd == self.CMD_DETECTION_RANGE_TAG_REPORT:
      return self.EVENT_TAG
    if packet.control == self.CTRL_PEOPLE_COUNT and packet.cmd in (self.CMD_PEOPLE_COUNT_REPORT, self.CMD_PEOPLE_COUNT_QUERY_COUNT):
      return self.EVENT_PEOPLE_COUNT
    return self.EVENT_UNKNOWN

  def _parse_targets(self, data):
    if data is None:
      self._target_count = 0
      self._targets = []
      return
    target_len = 11
    count = len(data) // target_len
    if count > self.MAX_TARGETS:
      count = self.MAX_TARGETS
    self._target_count = count
    self._targets = []
    for i in range(count):
      offset = i * target_len
      target = TargetInfo()
      target.index = data[offset]
      target.kinesia = data[offset + 1]
      target.target_feature = data[offset + 2]
      target.x = self._sb16(data, offset + 3)
      target.y = self._sb16(data, offset + 5)
      target.height = self._sb16(data, offset + 7)
      target.speed = self._sb16(data, offset + 9)
      self._targets.append(target)

  def _parse_tag_list(self, data, max_tags=None):
    tags = []
    if data is None or len(data) < 2:
      return tags, 0
    tag_len = 12
    total = self._u16(data, 0)
    actual_count = 0xFF if total > 0xFF else total
    available_len = len(data) - 2
    if available_len < actual_count * tag_len:
      actual_count = available_len // tag_len
    copy_count = actual_count
    if max_tags is not None and copy_count > max_tags:
      copy_count = max_tags
    for i in range(copy_count):
      offset = 2 + i * tag_len
      tag = TagConfig()
      tag.tag_index = data[offset]
      tag.tag_type = data[offset + 1]
      tag.scope_type = data[offset + 2]
      tag.io_index = data[offset + 3]
      tag.center_x = self._sb16(data, offset + 4)
      tag.center_y = self._sb16(data, offset + 6)
      tag.width = self._u16(data, offset + 8)
      tag.height = self._u16(data, offset + 10)
      tags.append(tag)
    return tags, actual_count

  def _parse_tag_event(self, data):
    if data is None or len(data) < 8:
      self._tag_info_valid = False
      return
    info = TagInfo()
    info.tag_index = data[0]
    info.tag_type = data[1]
    info.io_index = data[2]
    info.center_x = self._sb16(data, 3)
    info.center_y = self._sb16(data, 5)
    if info.tag_type == self.TAG_BOUNDARY:
      info.enter_exit = data[7]
    elif info.tag_type == self.TAG_APPROACH_AWAY:
      info.motion_dir = data[7]
    elif info.tag_type == self.TAG_PEOPLE_COUNTING:
      info.motion_num = (data[7] >> 4) & 0x0F
      info.static_num = data[7] & 0x0F
    self._tag_info = info
    self._tag_info_valid = True

  def _parse_boundary_range(self, data):
    if data is None or len(data) < 1:
      return
    self._range_info.mode = data[0]
    if self._range_info.mode == self.RANGE_FOUR_SIDE:
      offset = 1
      if len(data) >= 10 and data[1] == 0x00:
        offset = 2
      if len(data) >= offset + 8:
        self._range_info.x_positive_cm = self._sb16(data, offset)
        self._range_info.x_negative_cm = self._sb16(data, offset + 2)
        self._range_info.y_positive_cm = self._sb16(data, offset + 4)
        self._range_info.y_negative_cm = self._sb16(data, offset + 6)

  def _parse_people_count(self, data):
    if data is None or len(data) == 0:
      self._people_count = 0
    elif len(data) >= 2:
      self._people_count = data[1]
    else:
      self._people_count = data[0]

  @staticmethod
  def _u16(data, offset):
    return (data[offset] << 8) | data[offset + 1]

  @classmethod
  def _i16(cls, data, offset):
    value = cls._u16(data, offset)
    return value - 0x10000 if value & 0x8000 else value

  @classmethod
  def _sb16(cls, data, offset):
    value = cls._u16(data, offset)
    magnitude = value & 0x7FFF
    return -magnitude if value & 0x8000 else magnitude

  @staticmethod
  def _u32(data, offset):
    return (data[offset] << 24) | (data[offset + 1] << 16) | (data[offset + 2] << 8) | data[offset + 3]

  @staticmethod
  def _u16_bytes(value):
    value &= 0xFFFF
    return [(value >> 8) & 0xFF, value & 0xFF]

  @classmethod
  def _i16_bytes(cls, value):
    return cls._u16_bytes(value & 0xFFFF)

  @classmethod
  def _sb16_bytes(cls, value):
    magnitude = int(value)
    raw = 0
    if magnitude < 0:
      magnitude = -magnitude
      raw = 0x8000
    if magnitude > 0x7FFF:
      magnitude = 0x7FFF
    raw |= magnitude
    return cls._u16_bytes(raw)

  @staticmethod
  def _u32_bytes(value):
    value &= 0xFFFFFFFF
    return [(value >> 24) & 0xFF, (value >> 16) & 0xFF, (value >> 8) & 0xFF, value & 0xFF]
