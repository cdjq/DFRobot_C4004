# DFRobot_C4004 Raspberry Pi Library
- [中文版](./README_CN.md)

This folder contains the Raspberry Pi Python driver for DFRobot C4004, a 60GHz 4T4R multi-zone presence mmWave radar for smart space management with real-time static/moving people counts and 6-way IO linkage.

## Installation

```bash
pip3 install pyserial
```

If you use `read_zone_state_by_gpio.py`, enable Raspberry Pi GPIO and install `RPi.GPIO` when needed.

## Wiring

DFRobot C4004 pin | Raspberry Pi
---------- | ------------
VCC        | 5V
GND        | GND
TX         | RXD
RX         | TXD

The default serial port in examples is `/dev/ttyAMA0`, baudrate `115200`.

## Methods

```python
  def begin(self):
    '''!
      @brief Initialize module and verify communication.
      @return True or False
    '''

  def close(self):
    '''!
      @brief Close serial port.
    '''

  def is_init_finished(self):
    '''!
      @brief Query whether module initialization is finished.
      @return True or False
    '''

  def is_connected(self):
    '''!
      @brief Check whether module is connected.
      @return True or False
    '''

  def reset(self):
    '''!
      @brief Reboot module.
      @return True or False
    '''

  def factory_reset(self):
    '''!
      @brief Restore module to factory settings.
      @return True or False
    '''

  def get_heartbeat(self, mode=GET_DATA_ACTIVE):
    '''!
      @brief Get heartbeat status.
      @param mode: data mode
      @n   GET_DATA_ACTIVE: active query
      @n   GET_DATA_REPORT: read report cache
      @return True or False
    '''

  def get_reported_info(self, timeout=0.05):
    '''!
      @brief Read and decode one reported event.
      @param timeout: timeout in seconds
      @return event type
    '''

  def get_product_model(self):
    '''!
      @brief Get product model.
      @return String
    '''

  def get_product_id(self):
    '''!
      @brief Get product ID.
      @return Integer product ID
    '''

  def get_hardware_version(self):
    '''!
      @brief Get hardware version.
      @return String
    '''

  def get_firmware_version(self):
    '''!
      @brief Get firmware version.
      @return String
    '''

  def set_install_info(self, info):
    '''!
      @brief Set installation information.
      @param info: InstallInfo object
      @n   mode: INSTALL_MODE_SIDE / INSTALL_MODE_TOP
      @n   height_cm: installation height in cm
      @n   x_angle/y_angle/z_angle: installation angles in degree
      @return True or False
    '''

  def get_install_info(self, info):
    '''!
      @brief Get installation information.
      @param info: InstallInfo object for output
      @return True or False
    '''

  def set_install_high(self, hight):
    '''!
      @brief Set installation height.
      @param hight: installation height in cm
      @return True or False
    '''

  def get_install_high(self):
    '''!
      @brief Get installation height.
      @return installation height in cm. Returns 0 on failure.
    '''

  def set_install_height(self, height_cm):
    '''! @brief Alias of set_install_high with corrected spelling. '''

  def get_install_height(self):
    '''! @brief Alias of get_install_high with corrected spelling. '''

  def set_presence_enable(self, enable):
    '''!
      @brief Enable or disable presence detection.
      @param enable: True/False
      @return True or False
    '''

  def get_presence_enable(self, enable):
    '''!
      @brief Get presence detection enable state.
      @param enable: output container (list/dict/object.value)
      @return True or False
    '''

  def get_presence_state(self):
    '''!
      @brief Get presence state.
      @return NO_PRESENCE / PRESENCE / PRESENCE_UNKNOWN
    '''

  def get_motion_state(self):
    '''!
      @brief Get motion state.
      @return MOTION_NONE / MOTION_STATIC / MOTION_ACTIVE / MOTION_UNKNOWN
    '''

  def set_trajectory_track_enable(self, enable):
    '''!
      @brief Enable or disable trajectory tracking.
      @param enable: True/False
      @return True or False
    '''

  def get_trajectory_track_enable(self, enable):
    '''!
      @brief Get trajectory tracking enable state.
      @param enable: output container (list/dict/object.value)
      @return True or False
    '''

  def set_check_to_active_frames(self, frames):
    '''!
      @brief Set the frame count used to confirm transition from check state to active state.
      @param frames: frame count, valid range 1-7
      @return True or False
    '''

  def get_check_to_active_frames(self, frames):
    '''!
      @brief Get the frame count used to confirm transition from check state to active state.
      @param frames: output container (list/dict/object.value)
      @return True or False
    '''

  def get_target_list(self, mode=GET_DATA_ACTIVE):
    '''!
      @brief Get target list.
      @param mode: data mode
      @n   GET_DATA_ACTIVE: active query
      @n   GET_DATA_REPORT: read report cache
      @return List[TargetInfo]
    '''

  def get_target_info(self, index=0, mode=GET_DATA_ACTIVE):
    '''!
      @brief Get one target info by index.
      @param index: target index
      @param mode: data mode
      @n   GET_DATA_ACTIVE: active query
      @n   GET_DATA_REPORT: read report cache
      @return TargetInfo or None
      @note Falls back to list position access when no matching target index exists.
    '''

  def get_target_count(self):
    '''!
      @brief Get cached target count.
      @return Integer
    '''

  def set_trajectory_led(self, enable):
    '''!
      @brief Set trajectory LED.
      @param enable: True/False
      @return True or False
    '''

  def set_motion_led(self, enable):
    '''!
      @brief Set motion LED.
      @param enable: True/False
      @return True or False
    '''

  def get_trajectory_led(self):
    '''!
      @brief Get trajectory LED state.
      @return True or False
    '''

  def get_motion_led(self):
    '''!
      @brief Get motion LED state.
      @return True or False
    '''

  def get_tags(self, mode=GET_DATA_ACTIVE, max_tags=None):
    '''!
      @brief Get all tag configs from the device.
      @param mode: data mode kept for compatibility
      @n   GET_DATA_ACTIVE: active query from device
      @n   GET_DATA_REPORT: currently behaves the same as GET_DATA_ACTIVE
      @param max_tags: maximum number of tags to return. None returns all parsed tags.
      @return List[TagConfig]
    '''

  def set_tag(self, tag):
    '''!
      @brief Set one tag (size mode).
      @param tag: TagConfig object
      @n   tag.io_index: IO linkage index. 0 means unused; 2-6 maps to IO2-IO6.
      @n   tag.width: tag width or circle radius in cm
      @n   tag.height: tag height in cm
      @return tag set status code
      @n   TAG_SET_COMM_ERROR
      @n   TAG_SET_SUCCESS
      @n   TAG_SET_TRACK_COUNT_ERROR
      @n   TAG_SET_ALREADY_USED
      @n   TAG_SET_INDEX_OUT_OF_RANGE
      @note center_x/center_y are ignored by this API.
      @note Track count must be 1 when using this API.
      @note Set up to 32 tags at most.
    '''

  def clear_tag(self, tag_index):
    '''!
      @brief Clear one tag.
      @param tag_index: tag index (2-byte index in protocol payload)
      @return True or False
    '''

  def clear_all_tags(self):
    '''!
      @brief Clear all tags.
      @return True or False
    '''

  def set_tags_from_config(self, tags):
    '''!
      @brief Set tags in coordinate mode.
      @param tags: iterable of TagConfig
      @n   tag.io_index in each tag: IO linkage index. 0 means unused; 2-6 maps to IO2-IO6.
      @return True or False
      @note Coordinate mode does not require track count to be 1.
      @note Set up to 32 tags at most.
    '''

  def get_tag_info(self):
    '''!
      @brief Get last tag event decoded from report cache (CTRL 0x07, CMD 0x1B).
      @return TagInfo object, or None if no valid event.
      @note This API reads report cache only. Call get_reported_info() first to receive new report data.
      @note Tag event reports include info.io_index.
    '''

  def set_four_sided_range_mode(self, range_info):
    '''!
      @brief Set four-side boundary range (mode 0x04).
      @param range_info: FourSidedRange_t object
      @return True or False
      @note Position values use sign-bit int16 encoding (bit15: 0=positive, 1=negative).
    '''

  def get_four_sided_range_mode(self, range_info):
    '''!
      @brief Get four-side boundary range.
      @param range_info: FourSidedRange_t object for output
      @return True or False
    '''

  def set_trajectory_range_mode(self, learning):
    '''!
      @brief Start trajectory-range learning or use the learned trajectory range (mode 0x05).
      @param learning: True starts learning; False uses trajectory range mode without learning.
      @return True or False
    '''

  def set_config_file_mode_points(self, points):
    '''!
      @brief Set config-file mode points (mode 0x06).
      @param points: iterable of Point
      @return True or False
      @n   payload: 0x06 + 2B count + n*(2B X + 2B Y)
      @note Point values use sign-bit int16 encoding (bit15: 0=positive, 1=negative).
      @note Point count is limited to MAX_POINTS (150).
    '''

  def get_trajectory_range_mode(self, points, point_count):
    '''!
      @brief Get trajectory mode points (mode 0x05).
      @param points: output list for Point objects
      @param point_count: output container (list/dict/object.value)
      @return True or False
      @note points list must be able to hold at least MAX_POINTS points.
    '''

  def get_trajectory_mode_points(self, points, point_count):
    '''!
      @brief Backward-compatible alias of get_trajectory_range_mode.
      @param points: output list for Point objects
      @param point_count: output container (list/dict/object.value)
      @return True or False
    '''

  def get_config_file_mode_points(self, points, point_count):
    '''!
      @brief Get config-file mode points (mode 0x06).
      @param points: output list for Point objects
      @param point_count: output container (list/dict/object.value)
      @return True or False
      @note points list must be able to hold at least MAX_POINTS points.
    '''

  def get_detection_range_mode(self):
    '''!
      @brief Query current detection range mode.
      @return mode value
    '''

  def get_people_time(self, mode=GET_DATA_ACTIVE):
    '''!
      @brief Get people count.
      @param mode: data mode
      @n   GET_DATA_ACTIVE: query latest data and update cache
      @n   GET_DATA_REPORT: return cached data directly
      @return people count (maximum count reported by module)
    '''

  def set_real_time_people_time(self, interval):
    '''!
      @brief Set people report interval.
      @param interval: seconds
      @return True or False
    '''

  def get_real_time_people_time(self):
    '''!
      @brief Get people report interval.
      @return seconds
    '''

  def clear_people_count(self):
    '''!
      @brief Clear people count statistics.
      @return True or False
    '''

  def set_track_meters(self, distance_cm):
    '''!
      @brief Set trajectory generation distance threshold.
      @param distance_cm: threshold in cm
      @return True or False
    '''

  def get_track_meters(self):
    '''!
      @brief Get trajectory generation distance threshold.
      @return threshold in cm
    '''

  def set_track_exists_time(self, time):
    '''!
      @brief Set trajectory hold time.
      @param time: seconds
      @return True or False
    '''

  def get_track_exists_time(self):
    '''!
      @brief Get trajectory hold time.
      @return seconds
    '''

  def set_unmanned_time(self, delay_time):
    '''!
      @brief Set no-person delay time.
      @param delay_time: seconds
      @return True or False
    '''

  def get_unmanned_time(self):
    '''!
      @brief Get no-person delay time.
      @return seconds
    '''
```

## History

- 2026/05/22 - V1.0.0 version

## Credits

Written by JiaLi(zhixin.liu@dfrobot.com), 2026. (Welcome to our [website](https://www.dfrobot.com/))
