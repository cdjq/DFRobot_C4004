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
      @brief Initialize the DFRobot C4004 sensor.
      @n Opens the serial port (if needed) and waits for initialization finished.
      @return True: Initialization succeeded, False: Initialization failed.
    '''

  def close(self):
    '''!
      @brief Close serial port.
    '''

  def is_init_finished(self):
    '''!
      @brief Query whether module initialization is finished.
      @return True if initialization is finished, otherwise False.
    '''

  def is_connected(self):
    '''!
      @brief Check if the DFRobot C4004 sensor is connected.
      @return True: Connected, False: Not connected.
    '''

  def reset(self):
    '''!
      @brief Reset the DFRobot C4004 sensor.
      @return True: Reset succeeded, False: Reset failed.
    '''

  def factory_reset(self):
    '''!
      @brief Factory reset the DFRobot C4004 sensor.
      @return True: Reset succeeded, False: Reset failed.
    '''

  def get_heartbeat(self, mode=GET_DATA_ACTIVE):
    '''!
      @brief Get the heartbeat status of the DFRobot C4004 sensor.
      @param mode: Data acquisition mode.
      @n          GET_DATA_ACTIVE: Actively obtain the latest heartbeat status.
      @n          GET_DATA_REPORT: Obtain the latest heartbeat status from the last report.
      @return True: Heartbeat detected, False: No heartbeat detected.
    '''

  def get_reported_event(self, timeout=0.05):
    '''!
      @brief Wait for and decode one report frame pushed by the DFRobot C4004 sensor.
      @param timeout: Max time to wait for one complete UART report frame, in seconds (default: 0.05).
      @n          The call blocks at most timeout. If no complete frame arrives in time, returns EVENT_NONE.
      @n          If a frame arrives earlier, it returns as soon as the frame is decoded (may be shorter than timeout).
      @return Reported event type.
      @n          EVENT_NONE: No complete frame in this round (including wait timeout).
      @n          EVENT_TRAJECTORY: Trajectory tracking event detected.
      @n          EVENT_PRESENCE: Presence detection event detected.
      @n          EVENT_MOTION: Human motion event detected.
      @n          EVENT_TAG: Tag event detected.
      @n          EVENT_HEARTBEAT: Heartbeat event detected.
      @n          EVENT_INIT_FINISHED: Initialization finished event detected.
      @n          EVENT_PEOPLE_COUNT: People count event detected.
      @n          EVENT_UNKNOWN: Complete frame received, but event type is unrecognized.
      @n          EVENT_ERROR: Internal error (e.g. null pointer); rare at application layer.
    '''

  def get_hardware_version(self):
    '''!
      @brief Get the hardware version of the DFRobot C4004 sensor.
      @return Hardware version string.
    '''

  def get_firmware_version(self):
    '''!
      @brief Get the firmware version of the DFRobot C4004 sensor.
      @return Firmware version string.
    '''

  def set_install_info(self, info):
    '''!
      @brief Set the installation information of the DFRobot C4004 sensor.
      @param info: Installation information.
      @n          mode: Mounting mode, INSTALL_MODE_SIDE or INSTALL_MODE_TOP.
      @n          height_cm: Installation height in cm.
      @n            - Side (z_angle 0°): default 180 cm, recommended 180±20 cm (too low is easily blocked).
      @n            - Top (z_angle 90°): recommended 220-280 cm (2.2-2.8 m).
      @n          z_angle: Pitch tilt in degrees (default 0°). 0° = side (looking along +Y), 90° = top (looking down).
      @n            See DFRobot_InstallInfo for the sensor X/Y coordinate system relative to object positions.
      @return True: Set succeeded, False: Set failed.
      @note Invalid mode or height returns False. Out-of-range angles are clamped.
      @note If the installation height is too low, it is easy to be blocked
    '''

  def get_install_info(self, info):
    '''!
      @brief Get the installation information of the DFRobot C4004 sensor.
      @param info: Installation information.
      @n          mode: Mounting mode, INSTALL_MODE_SIDE or INSTALL_MODE_TOP.
      @n          height_cm: Installation height in cm.
      @n            - Side (z_angle 0°): default 180 cm, recommended 180±20 cm (too low is easily blocked).
      @n            - Top (z_angle 90°): recommended 220-280 cm (2.2-2.8 m).
      @n          z_angle: Installation tilt angle in degrees. Defines mounting: 0° = side, 90° = top.
      @return True: Get succeeded, False: Get failed.
    '''

  def set_install_height(self, height):
    '''!
      @brief Set the installation height of the DFRobot C4004 sensor.
      @param height: Installation height in cm.
      @n            - Side (z_angle 0°): default 180 cm, recommended 180±20 cm.
      @n            - Top (z_angle 90°): recommended 220-280 cm (2.2-2.8 m).
      @return True: Set succeeded, False: Set failed.
      @note If the installation height is too low, it is easy to be blocked
    '''

  def get_install_height(self):
    '''!
      @brief Get the installation height of the DFRobot C4004 sensor.
      @return Installation height in cm. Returns 0 on failure.
    '''

  def set_presence_enable(self, enable):
    '''!
      @brief Enable or disable the presence detection function of the sensor.
      @param enable: Enable or disable the presence detection function.
      @n          True: Enable, False: Disable.
      @return True: Set succeeded, False: Set failed.
    '''

  def get_presence_enable(self, enable):
    '''!
      @brief Get whether the presence detection function is enabled.
      @param enable: Output container for the enable state.
      @n          True: Enabled, False: Disabled.
      @n          Supported containers: list / dict / object(with value field).
      @return True: Get succeeded, False: Get failed.
    '''

  def get_presence_state(self, mode=GET_DATA_ACTIVE):
    '''!
      @brief Get whether a person is currently present within the detection range.
      @param mode: Data acquisition mode.
      @n          GET_DATA_ACTIVE: Query latest data and update cache.
      @n          GET_DATA_REPORT: Return cached data from the last report.
      @return Presence detection result.
      @n          NO_PRESENCE: No presence detected.
      @n          PRESENCE: Presence detected.
    '''

  def get_motion_state(self, mode=GET_DATA_ACTIVE):
    '''!
      @brief Get the current human motion state within the detection range.
      @param mode: Data acquisition mode.
      @n          GET_DATA_ACTIVE: Query latest data and update cache.
      @n          GET_DATA_REPORT: Return cached data from the last report.
      @return Motion state.
      @n          MOTION_NONE: No motion state.
      @n          MOTION_STATIC: Stationary.
      @n          MOTION_ACTIVE: Active motion.
    '''

  def set_trajectory_track_enable(self, enable):
    '''!
      @brief Enable or disable the trajectory tracking function of the sensor.
      @param enable: Enable or disable the trajectory tracking function.
      @n          True: Enable, False: Disable.
      @return True: Set succeeded, False: Set failed.
    '''

  def get_trajectory_track_enable(self, enable):
    '''!
      @brief Query whether the trajectory tracking function is enabled.
      @param enable: Output container for the enable state.
      @n          True: Enabled, False: Disabled.
      @n          Supported containers: list / dict / object(with value field).
      @return True: Query succeeded, False: Query failed.
    '''

  def set_frame_generate_count(self, frames):
    '''!
      @brief Set the frame count used to confirm transition from check state to active state.
      @n A larger value suppresses noise more strongly, and also affects the trigger distance.
      @param frames: Frame count, valid range: 1-7, default: 7.
      @return True: Set succeeded, False: Set failed.
    '''

  def get_frame_generate_count(self, frames):
    '''!
      @brief Query the frame count used to confirm transition from check state to active state.
      @n A larger value suppresses noise more strongly, and also affects the trigger distance.
      @param frames: Output container for frame count.
      @n          Supported containers: list / dict / object(with value field).
      @return True: Query succeeded, False: Query failed.
    '''

  def get_target_list(self, mode=GET_DATA_ACTIVE):
    '''!
      @brief Get the list of target information of the DFRobot C4004 sensor.
      @param mode: Data acquisition mode.
      @n          GET_DATA_ACTIVE: Query latest target information before reading.
      @n          GET_DATA_REPORT: Read target information from cached report data.
      @return List of DFRobot_TargetInfo objects.
    '''

  def set_trk_led(self, enable):
    '''!
      @brief Enable or disable the trajectory tracking LED function.
      @n If enabled, the LED turns on only while learning/generating a trajectory range; it stays off at all other times.
      @param enable: True to enable, False to disable.
      @return True: Set succeeded, False: Set failed.
    '''

  def set_occ_led(self, enable):
    '''!
      @brief Enable or disable the occupancy LED function.
      @n If enabled, the LED turns on when the detection range is occupied (someone is present).
      @param enable: True to enable, False to disable.
      @return True: Set succeeded, False: Set failed.
    '''

  def get_trk_led(self):
    '''!
      @brief Get whether the trajectory tracking LED function is enabled.
      @n When enabled, the LED turns on only while learning/generating a trajectory range; it stays off at all other times.
      @return True: LED function is enabled, False: LED function is disabled.
    '''

  def get_occ_led(self):
    '''!
      @brief Get whether the occupancy LED function is enabled.
      @n When enabled, the LED turns on if the detection range is occupied (someone is present).
      @return True: LED function is enabled, False: LED function is disabled.
    '''

  def get_tags(self, mode=GET_DATA_ACTIVE, max_tags=None):
    '''!
      @brief Obtain all tag configuration information.
      @param mode: Data acquisition mode kept for compatibility.
      @n          GET_DATA_ACTIVE: Query latest tag configuration from the device.
      @n          GET_DATA_REPORT: Currently behaves the same as GET_DATA_ACTIVE.
      @n          io_index in each tag: 0 means unused; 2-6 maps to IO2-IO6.
      @param max_tags: Maximum number of tags to return. None returns all parsed tags.
      @return List of DFRobot_TagConfig objects.
    '''

  def set_tag(self, tag):
    '''!
      @brief Set one tag using size mode.
      @param tag: Tag configuration.
      @n          tag_index: Tag index.
      @n          tag_type: Tag type.
      @n          scope_type: Tag range type.
      @n          io_index: IO linkage index. 0 means unused; 2-6 maps to IO2-IO6.
      @n          width: Tag width or circle radius, in cm.
      @n          height: Tag height, in cm.
      @return Tag set status.
      @n          TAG_SET_COMM_ERROR: Communication failed or response mismatch.
      @n          TAG_SET_SUCCESS: Tag set succeeded.
      @n          TAG_SET_TRACK_COUNT_ERROR: Track count is not equal to 1.
      @n          TAG_SET_ALREADY_USED: Tag has been occupied.
      @n          TAG_SET_INDEX_OUT_OF_RANGE: Tag index out of range.
      @note center_x/center_y in DFRobot_TagConfig are ignored by this API.
      @note Invalid tag_type, scope_type or io_index returns TAG_SET_COMM_ERROR before sending command.
      @note When setting labels using this API, it is necessary to ensure that the number of tracks is 1.
      @note Set up to 32 tags at most.
    '''

  def clear_tag(self, tag_index):
    '''!
      @brief Clear the tag configuration.
      @param tag_index: Tag index (1-byte index in protocol payload, 0-254).
      @return True: Clear succeeded, False: Clear failed.
      @note 0xFF is reserved for clear_all_tags(); do not pass it to clear_tag().
      @note Device returns 0xFE in response when tag index is out of range.
    '''

  def clear_all_tags(self):
    '''!
      @brief Clear all tag configurations.
      @return True: Clear succeeded, False: Clear failed.
    '''

  def set_tags_from_config(self, tags):
    '''!
      @brief Set tag configurations from a list in coordinate mode.
      @param tags: Iterable of DFRobot_TagConfig objects.
      @n          io_index in each tag: 0 means unused; 2-6 maps to IO2-IO6.
      @return True: Set succeeded, False: Set failed.
      @note The labels can be set in the form of coordinates, and there is no need to meet the requirement that the number of tracks is 1
      @note Set up to 32 tags at most.
      @note Invalid tag_type, scope_type or io_index in any tag returns False before sending command.
    '''

  def get_tag_info(self):
    '''!
      @brief Get the last tag event decoded from active report packet (CTRL 0x07, CMD 0x1B).
      @return DFRobot_TagInfo object if valid, otherwise None.
      @note This API reads report cache only. Call get_reported_event() to receive new report data first.
      @note Tag event reports include io_index.
    '''

  def set_four_sided_range_mode(self, range_info):
    '''!
      @brief Set four-side boundary detection range.
      @param range_info: Boundary range settings.
      @n          x_max: Maximum x boundary, in cm.
      @n          x_min: Minimum x boundary, in cm.
      @n          y_max: Maximum y boundary, in cm.
      @n          y_min: Minimum y boundary, in cm.
      @return True: Set succeeded, False: Set failed.
    '''

  def get_four_sided_range_mode(self, range_info):
    '''!
      @brief Query and get four-side boundary detection range.
      @param range_info: FourSidedRange object to receive boundary range settings.
      @return True: Get succeeded, False: Get failed.
    '''

  def set_trajectory_range_mode(self, learning):
    '''!
      @brief Start generating a trajectory detection range, or use a previously generated one.
      @n If generation/learning is enabled (True): after the sensor confirms there is only one trajectory,
      @n it starts generating/learning the detection range.
      @n If you disable learning (False) while generation is in progress: the sensor stops learning,
      @n saves the auto-generated detection range, and enables it.
      @n If trajectory-range mode is not currently enabled, call set_trajectory_range_mode(False) to
      @n enable and use the previously generated/saved detection range.
      @param learning: Trajectory-range generation/learning switch.
      @n          True: Start generating/learning the detection range.
      @n          False: Stop learning and save/enable the generated range, or use a previously saved range.
    '''

  def get_trajectory_range_mode(self, points, point_count):
    '''!
      @brief Query and get range points in trajectory mode (mode 0x05).
      @param points: Buffer/list to receive trajectory-mode points.
      @param point_count: Output container for point count (list/dict/object with value).
      @return True: Query succeeded, False: Query failed.
      @note The points buffer must be able to hold at least MAX_POINTS points.
    '''

  def set_config_file_mode_points(self, points):
    '''!
      @brief Set detection range points using config-file mode (mode 0x06).
      @param points: Iterable of DFRobot_Point objects.
      @return True: Set succeeded, False: Set failed.
      @note Point values use sign-bit int16 encoding (bit15: 0=positive, 1=negative).
      @note Point count is limited to MAX_POINTS.
    '''

  def get_config_file_mode_points(self, points, point_count):
    '''!
      @brief Query and get range points in config-file mode (mode 0x06).
      @param points: Buffer/list to receive config-file-mode points.
      @param point_count: Output container for point count (list/dict/object with value).
      @return True: Query succeeded, False: Query failed.
      @note The points buffer must be able to hold at least MAX_POINTS points.
    '''

  def get_detection_range_mode(self):
    '''!
      @brief Query current detection range mode.
      @return Current detection range mode.
    '''

  def get_people_count(self, mode=GET_DATA_ACTIVE):
    '''!
      @brief Get the real-time people count. Only confirmed real person targets are counted.
      @param mode: Data acquisition mode.
      @n          GET_DATA_ACTIVE: Query latest data and update cache.
      @n          GET_DATA_REPORT: Return cached data directly.
      @return Real-time people count after filtering.
    '''

  def set_real_time_people_time(self, interval):
    '''!
      @brief Set the people-count report interval.
      @param interval: Report interval in seconds. Default: 1 s. Valid range: 1-3600 seconds.
      @return True: Set succeeded, False: Set failed.
    '''

  def get_real_time_people_time(self):
    '''!
      @brief Get the people-count report interval.
      @return Report interval in seconds. Returns 0 on failure.
    '''

  def clear_people_count(self):
    '''!
      @brief Clear the people count detected by the sensor and restart detection/tracking from 0.
      @n Use this when an interference object remains in the detection range and the sensor
      @n cannot confirm or clear it by itself; call this API to refresh the people-count state.
      @return True: Clear succeeded, False: Clear failed.
    '''

  def set_track_meters(self, distance_cm):
    '''!
      @brief Set trajectory movement distance threshold.
      @n After a trajectory is generated, the distance the track must move before it is confirmed as a person.
      @n Adjusts the judgment conditions of the real-time people-count interface.
      @param distance_cm: Distance threshold, in cm. Default is 0 cm, valid range: 0-1000 cm.
      @return True: Set succeeded, False: Set failed.
    '''

  def get_track_meters(self):
    '''!
      @brief Get trajectory movement distance threshold.
      @n After a trajectory is generated, the distance the track must move before it is confirmed as a person.
      @n Adjusts the judgment conditions of the real-time people-count interface.
      @return Distance threshold in cm. Returns 0 on failure.
    '''

  def set_track_exists_time(self, time):
    '''!
      @brief Set trajectory hold time.
      @n Adjusts the judgment conditions of the real-time people-count interface.
      @param time: Hold time, in seconds.default is 0 seconds,Valid range: 0-600 seconds.
      @return True: Set succeeded, False: Set failed.
    '''

  def get_track_exists_time(self):
    '''!
      @brief Get trajectory hold time.
      @n Adjusts the judgment conditions of the real-time people-count interface.
      @return Hold time in seconds. Returns 0 on failure.
    '''

  def set_unmanned_time(self, delay_time):
    '''!
      @brief Set unmanned delay time.
      @n Period used to judge whether a point is a real person target.
      @n If it is not a real person target, the target is automatically cleared after this period.
      @param delay_time: Period time, in seconds.default is 30 seconds,Valid range: 5-3600 seconds.
      @return True: Set succeeded, False: Set failed.
    '''

  def get_unmanned_time(self):
    '''!
      @brief Get unmanned delay time.
      @n Period used to judge whether a point is a real person target.
      @n If it is not a real person target, the target is automatically cleared after this period.
      @return Period time in seconds. Returns 0 on failure.
    '''
```

## Examples

| Board        | Work Well | Work Wrong | Untested | Remarks |
| ------------ | :-------: | :--------: | :------: | ------- |
| RaspberryPi2 |           |            |    √     |         |
| RaspberryPi3 |     √     |            |          |         |
| RaspberryPi4 |           |            |    √     |         |

* Python Version

| Python  | Work Well | Work Wrong | Untested | Remarks |
| ------- | :-------: | :--------: | :------: | ------- |
| Python2 |     √     |            |          |         |
| Python3 |           |            |    √     |         |

## History

- 2026/05/22 - V1.0.0 version

## Credits

Written by JiaLi(jia.li@dfrobot.com), 2026. (Welcome to our [website](https://www.dfrobot.com/))
