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
      @return TargetInfo or None
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

  def get_tags(self, mode=GET_DATA_ACTIVE):
    '''!
      @brief Get all tag configs from cache.
      @param mode: data mode
      @n   GET_DATA_ACTIVE: query latest tag list before reading cache
      @n   GET_DATA_REPORT: read from cache only
      @return List[TagConfig]
    '''

  def set_tag(self, tag):
    '''!
      @brief Set one tag (size mode).
      @param tag: TagConfig object
      @return tag set status code
      @n   TAG_SET_COMM_ERROR
      @n   TAG_SET_SUCCESS
      @n   TAG_SET_TRACK_COUNT_ERROR
      @n   TAG_SET_ALREADY_USED
      @n   TAG_SET_INDEX_OUT_OF_RANGE
    '''

  def clear_tag(self, tag_index):
    '''!
      @brief Clear one tag.
      @param tag_index: tag index
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
      @return True or False
    '''

  def get_tag_info(self):
    '''!
      @brief Get last tag event decoded from report cache.
      @return TagInfo object, or None if no valid event.
    '''

  def set_boundary_detection_range(self, range_info):
    '''!
      @brief Set four-side boundary range (mode 0x04).
      @param range_info: BoundaryDetectionRange object
      @return True or False
    '''

  def get_boundary_detection_range(self, range_info):
    '''!
      @brief Get four-side boundary range.
      @param range_info: BoundaryDetectionRange object for output
      @return True or False
    '''

  def set_trajectory_detection_range(self, enable):
    '''!
      @brief Enable or disable trajectory range mode (mode 0x05).
      @param enable: True/False
      @return True or False
    '''

  def set_config_file_mode_points(self, points):
    '''!
      @brief Set config-file mode points (mode 0x06).
      @param points: iterable of Point
      @return True or False
      @n   payload: 0x06 + 2B count + n*(2B X + 2B Y)
    '''

  def get_trajectory_detection_range(self, points, point_count):
    '''!
      @brief Get trajectory mode points (mode 0x05).
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
    '''

  def get_detection_range_mode(self):
    '''!
      @brief Get current detection range mode.
      @return mode value
    '''

  def get_people_count_info(self, mode=GET_DATA_ACTIVE):
    '''!
      @brief Get people count.
      @param mode: data mode
      @return people count
    '''

  def set_people_report_interval(self, interval):
    '''!
      @brief Set people report interval.
      @param interval: seconds
      @return True or False
    '''

  def get_people_report_interval(self):
    '''!
      @brief Get people report interval.
      @return seconds
    '''

  def clear_people_count(self):
    '''!
      @brief Clear people count statistics.
      @return True or False
    '''

  def set_trajectory_generate_distance(self, distance_cm):
    '''!
      @brief Set trajectory generation distance threshold.
      @param distance_cm: threshold in cm
      @return True or False
    '''

  def get_trajectory_generate_distance(self):
    '''!
      @brief Get trajectory generation distance threshold.
      @return threshold in cm
    '''

  def set_trajectory_hold_time(self, hold_time):
    '''!
      @brief Set trajectory hold time.
      @param hold_time: seconds
      @return True or False
    '''

  def get_trajectory_hold_time(self):
    '''!
      @brief Get trajectory hold time.
      @return seconds
    '''

  def set_no_person_delay(self, delay_time):
    '''!
      @brief Set no-person delay time.
      @param delay_time: seconds
      @return True or False
    '''

  def get_no_person_delay(self):
    '''!
      @brief Get no-person delay time.
      @return seconds
    '''
```

## History

- 2026/05/22 - V1.0.0 version

## Credits

Written by JiaLi(zhixin.liu@dfrobot.com), 2026. (Welcome to our [website](https://www.dfrobot.com/))
