# DFRobot_C4004
- [中文版](./README_CN.md)

DFRobot C4004 is a 60GHz 4T4R multi-zone presence mmWave radar for smart space management. It can report not only occupancy, but also real-time counts of static people and moving people in configured areas, with built-in zone logic and 6-way IO linkage for fast automation deployment.

## Product Link(www.dfrobot.com)

    SKU:SEN0753

## Table of Contents

* [Summary](#Summary)
* [Installation](#Installation)
* [Methods](#Methods)
* [Compatibility](#Compatibility)
* [History](#History)
* [Credits](#Credits)

## Summary

* Supports UART communication on Arduino UNO, FireBeetle ESP32 and Raspberry Pi.
* Supports system heartbeat, initialization state query, reset and factory reset.
* Supports product model, product ID, hardware version and firmware version query.
* Supports installation mode, installation height and Z angle configuration.
* Supports human presence detection and motion state query/report.
* Supports target trajectory tracking and target information report.
* Supports tag zone configuration, tag clear, batch tag configuration and tag event report.
* Supports four-side detection boundary and trajectory detection range mode settings.
* Supports live count query/report, real-time report interval, trajectory generation distance, trajectory lifetime and unoccupied time settings.

## Installation

There are two methods for using this library:

1. Open Arduino IDE, search for `DFRobot_C4004` in Tools -> Manage Libraries and install the library.
2. Download this library, copy it into the Arduino `libraries` folder, then open the examples folder and run the demo.

For Raspberry Pi, use the driver in `python/raspberrypi/` and install `pyserial` if it is not already installed.

```bash
pip3 install pyserial
```

## Methods

```C++

  /**
   * @fn begin
   * @brief Initialize the sensor module.
   * @return true: Initialization succeeded, false: Initialization failed.
  */
  bool begin(void);

  /**
   * @fn isConnected
   * @brief Check if the sensor is connected.
   * @return true: Connected, false: Not connected.
  */
  bool isConnected(void);

  /**
   * @fn reset
   * @brief Reset the sensor.
   * @return true: Reset succeeded, false: Reset failed.
  */
  bool reset(void);

  /**
   * @fn factoryReset
   * @brief Factory reset the sensor.
   * @return true: Reset succeeded, false: Reset failed.
  */
  bool factoryReset(void);

  /**
   * @fn getHeartbeat
   * @brief Get the heartbeat status of the sensor.
   * @param mode: Data acquisition mode.
   * @n          eGetDataActive: Actively obtain the latest heartbeat status.
   * @n          eGetDataReport: Obtain the latest heartbeat status from the last report.
   * @return true: Heartbeat detected, false: No heartbeat detected.
  */
  bool getHeartbeat(eGetDataMode_t mode = eGetDataActive);

  /**
   * @fn getReportedEvent
   * @brief Get the latest reported event from the sensor.
   * @param timeoutMs: Maximum waiting time for the report?in milliseconds?default is 50.
   * @return eReportedEvent_t: Reported event type.
   * @n          eEventNone: No event detected.
   * @n          eEventTrajectory: Trajectory tracking event detected.
   * @n          eEventPresence: Presence detection event detected.
   * @n          eEventMotion: Human motion event detected.
   * @n          eEventTag: Tag event detected.
   * @n          eEventHeartbeat: Heartbeat event detected.
   * @n          eEventInitFinished: Initialization finished event detected.
   * @n          eEventPeopleCount: People count event detected.
   * @n          eEventUnknown: Unknown event detected.
   * @n          eEventError: Error occurred.
  */
  eReportedEvent_t getReportedEvent(uint16_t timeoutMs = 50);

  /**
   * @fn getHardwareVersion
   * @brief Get the hardware version of the sensor.
   * @return String: Hardware version.
  */
  String getHardwareVersion(void);

  /**
   * @fn getFirmwareVersion
   * @brief Get the firmware version of the sensor.
   * @return String: Firmware version.
  */
  String getFirmwareVersion(void);

  /**
   * @fn setInstallInfo
   * @brief Set the installation information of the sensor.
   * @param info: Installation information.
   * @n          mode: Mounting mode.
   * @n          heightCm: Installation height, in cm.
   * @n          zAngle: Installation z-axis angle, in degrees.
   * @return true: Set succeeded, false: Set failed.
  */
  bool setInstallInfo(sInstallInfo_t &info);

  /**
   * @fn getInstallInfo
   * @brief Get the installation information of the sensor.
   * @param info: Installation information.
   * @n          mode: Mounting mode.
   * @n          heightCm: Installation height, in cm.
   * @n          zAngle: Installation z-axis angle, in degrees.
   * @return true: Get succeeded, false: Get failed.
  */
  bool getInstallInfo(sInstallInfo_t *info);

  /**
   * @fn setInstallHeight
   * @brief Set the installation height of the sensor.
   * @param height: Installation height, in cm.
   * @return true: Set succeeded, false: Set failed.
  */
  bool setInstallHeight(int32_t height);

  /**
   * @fn getInstallHeight
   * @brief Get the installation height of the sensor.
   * @param height: Pointer to receive installation height, in cm.
   * @return true: Get succeeded, false: Get failed.
  */
  bool getInstallHeight(int *pHeight);

  /**
   * @fn setPresenceEnable
   * @brief Enable or disable the presence detection function of the sensor.
   * @param enable: Enable or disable the presence detection function.
   * @n          true: Enable, false: Disable.
   * @return true: Set succeeded, false: Set failed.
  */
  bool setPresenceEnable(bool enable);

  /**
   * @fn getPresenceEnable
   * @brief Get whether the presence detection function is enabled.
   * @param enable: Pointer to receive the enable state.
   * @n          true: Enabled, false: Disabled.
   * @return true: Get succeeded, false: Get failed.
  */
  bool getPresenceEnable(bool *enable);

  /**
   * @fn getPresenceState
   * @brief Get whether a person is currently present within the detection range.
   * @param mode: Data acquisition mode.
   * @n          eGetDataActive: Query latest data and update cache.
   * @n          eGetDataReport: Return cached data from the last report.
   * @return ePresenceState_t: Presence detection result.
   * @n          eNoPresence: No presence detected.
   * @n          ePresence: Presence detected.
  */
  ePresenceState_t getPresenceState(eGetDataMode_t mode = eGetDataActive);

  /**
   * @fn getMotionState
   * @brief Get the current human motion state.
   * @param mode: Data acquisition mode.
   * @n          eGetDataActive: Query latest data and update cache.
   * @n          eGetDataReport: Return cached data from the last report.
   * @return eMotionState_t: Motion state.
   * @n          eMotionNone: No motion state.
   * @n          eMotionStatic: Stationary.
   * @n          eMotionActive: Active motion.
   * @n          eMotionUnknown: Unknown motion state.
   */
  eMotionState_t getMotionState(eGetDataMode_t mode = eGetDataActive);

  /**
   * @fn setTrajectoryTrackEnable
   * @brief Enable or disable the trajectory tracking function of the sensor.
   * @param enable: Enable or disable the trajectory tracking function.
   * @n          true: Enable, false: Disable.
   * @return true: Set succeeded, false: Set failed.
  */
  bool setTrajectoryTrackEnable(bool enable);

  /**
   * @fn getTrajectoryTrackEnable
   * @brief Query whether the trajectory tracking function is enabled.
   * @param enable: Pointer to receive the enable state.
   * @n          true: Enabled, false: Disabled.
   * @return true: Query succeeded, false: Query failed.
  */
  bool getTrajectoryTrackEnable(bool *enable);

  /**
   * @fn setFrameGenerateCount
   * @brief Set the frame count used to confirm transition from check state to active state.
   * @n A larger value suppresses noise more strongly, and also affects the trigger distance.
   * @param frames: Frame count, valid range: 1-7, default: 7.
   * @return true: Set succeeded, false: Set failed.
  */
  bool setFrameGenerateCount(uint8_t frames);

  /**
   * @fn getFrameGenerateCount
   * @brief Query the frame count used to confirm transition from check state to active state.
   * @n A larger value suppresses noise more strongly, and also affects the trigger distance.
   * @param frames: Pointer to receive frame count.
   * @return true: Query succeeded, false: Query failed.
  */
  bool getFrameGenerateCount(uint8_t *frames);

  /**
   * @fn getTargetList
   * @brief Get the list of target information of the sensor.
   * @param targetBuf: Pointer to receive the target information list.
   * @param maxCount: Maximum number of targets to be read.
   * @param mode: Data acquisition mode.
   * @n          eGetDataActive: Query latest target information before reading.
   * @n          eGetDataReport: Read target information from cached report data.
   * @return uint8_t: Number of targets read.
  */
  uint8_t getTargetList(sTargetInfo_t *targetBuf, uint8_t maxCount, eGetDataMode_t mode = eGetDataActive);

  /**
   * @fn setTrkLED
   * @brief Enable or disable the trajectory tracking LED function.
   * @n If enabled, the LED turns on only while learning/generating a trajectory range; it stays off at all other times.
   * @param enable: true to enable, false to disable.
   * @return true: Set succeeded, false: Set failed.
  */
  bool setTrkLED(bool enable);

  /**
   * @fn setOccLED
   * @brief Enable or disable the occupancy LED function.
   * @n If enabled, the LED turns on when the detection range is occupied (someone is present).
   * @param enable: true to enable, false to disable.
   * @return true: Set succeeded, false: Set failed.
  */
  bool setOccLED(bool enable);

  /**
   * @fn getTrkLED
   * @brief Get whether the trajectory tracking LED function is enabled.
   * @n When enabled, the LED turns on only while learning/generating a trajectory range; it stays off at all other times.
   * @return true: LED function is enabled, false: LED function is disabled.
  */
  bool getTrkLED(void);

  /**
   * @fn getOccLED
   * @brief Get whether the occupancy LED function is enabled.
   * @n When enabled, the LED turns on if the detection range is occupied (someone is present).
   * @return true: LED function is enabled, false: LED function is disabled.
  */
  bool getOccLED(void);

  /**
   * getTags
   * @brief Obtain all tag configuration information.
   * @param tags: Pointer to receive the tag configuration.
   * @param maxTags: Maximum number of tags to be written into tags.
   * @param mode: Data acquisition mode kept for compatibility.
   * @n          eGetDataActive: Query latest tag configuration from the device.
   * @n          eGetDataReport: Currently behaves the same as eGetDataActive.
   * @n          ioIndex in each tag: 0 means unused; 2-6 maps to IO2-IO6.
   * @return uint8_t: Actual number of tags returned by the device.
  */
  uint8_t getTags(sTagConfig_t *tags, uint8_t maxTags, eGetDataMode_t mode = eGetDataActive);

  /**
   * @fn setTag
   * @brief Set one tag using size mode.
   * @param tag: Tag configuration.
   * @n          tagIndex: Tag index.
   * @n          tagType: Tag type.
   * @n          scopeType: Tag range type.
   * @n          ioIndex: IO linkage index. 0 means unused; 2-6 maps to IO2-IO6.
   * @n          width: Tag width or circle radius, in cm.
   * @n          height: Tag height, in cm.
   * @return eTagSetStatus_t: Tag set status.
   * @n          eTagSetCommError: Communication failed or response mismatch.
   * @n          eTagSetSuccess: Tag set succeeded.
   * @n          eTagSetTrackCountError: Track count is not equal to 1.
   * @n          eTagSetAlreadyUsed: Tag has been occupied.
   * @n          eTagSetIndexOutOfRange: Tag index out of range.
   * @note centerX/centerY in sTagConfig_t are ignored by this API.
   * @note When setting labels using this API, it is necessary to ensure that the number of tracks is 1.
   * @note Set up to 32 tags at most.
  */
  eTagSetStatus_t setTag(const sTagConfig_t &tag);

  /**
   * @fn clearTag
   * @brief Clear the tag configuration.
   * @param tagIndex: Tag index (2-byte index in protocol payload).
   * @return true: Clear succeeded, false: Clear failed.
  */
  bool clearTag(uint16_t tagIndex);

  /**
   * @fn clearAllTags
   * @brief Clear all tag configurations.
   * @return true: Clear succeeded, false: Clear failed.
  */
  bool clearAllTags(void);

  /**
   * @fn setTagsFromConfig
   * @brief Set tag configurations from a list in coordinate mode.
   * @param tags: Pointer to the tag configuration list.
   * @param tagCount: Number of tags in the list.
   * @n          ioIndex in each tag: 0 means unused; 2-6 maps to IO2-IO6.
   * @return true: Set succeeded, false: Set failed.
   * @note The labels can be set in the form of coordinates, and there is no need to meet the requirement that the number of tracks is 1
   * @note Set up to 32 tags at most.
  */
  bool setTagsFromConfig(const sTagConfig_t *tags, uint8_t tagCount);

  /**
   * @fn getTagInfo
   * @brief Get the last tag event decoded from active report packet (CTRL 0x07, CMD 0x1B).
   * @param tagInfo: Pointer to receive reported tag event information.
   * @return true: Get succeeded, false: No valid reported tag event or invalid parameter.
   * @note This API reads report cache only. Call getReportedEvent() first to receive new report data.
   * @note Tag event reports include ioIndex.
  */
  bool getTagInfo(sTagInfo_t *tagInfo);

  /**
   * @fn setFourSidedRangeMode
   * @brief Set four-side boundary detection range.
   * @param range: Boundary range settings.
   * @n          xPositiveCm: Positive x boundary, in cm.
   * @n          xNegativeCm: Negative x boundary, in cm.
   * @n          yPositiveCm: Positive y boundary, in cm.
   * @n          yNegativeCm: Negative y boundary, in cm.
   * @return true: Set succeeded, false: Set failed.
  */
  bool setFourSidedRangeMode(sFourSidedRange_t &range);

  /**
   * @fn getFourSidedRangeMode
   * @brief Query and get four-side boundary detection range.
   * @param range: Pointer to receive boundary range settings.
   * @return true: Get succeeded, false: Get failed.
  */
  bool getFourSidedRangeMode(sFourSidedRange_t *range);

  /**
   * @fn setTrajectoryRangeMode
   * @brief Start generating a trajectory detection range, or use a previously generated one.
   * @n If generation/learning is enabled (true): after the sensor confirms there is only one trajectory,
   * @n it starts generating/learning the detection range.
   * @n If you disable learning (false) while generation is in progress: the sensor stops learning,
   * @n saves the auto-generated detection range, and enables it.
   * @n If trajectory-range mode is not currently enabled, call setTrajectoryRangeMode(false) to
   * @n enable and use the previously generated/saved detection range.
   * @param learning: Trajectory-range generation/learning switch.
   * @n          true: Start generating/learning the detection range.
   * @n          false: Stop learning and save/enable the generated range, or use a previously saved range.
  */
  void setTrajectoryRangeMode(bool learning);

  /**
   * @fn getTrajectoryRangeMode
   * @brief Query and get range points in trajectory mode.
   * @param points: Pointer to receive trajectory-mode points.
   * @param pointCount: Pointer to receive point count.
   * @return true: Query succeeded, false: Query failed.
   * @note The points buffer must be able to hold at least C4004_MAX_POINTS points.
  */
  bool getTrajectoryRangeMode(sPoint_t *points, uint16_t *pointCount);

  /**
   * @fn setConfigFileModePoints
   * @brief Set detection range points using config-file mode.
   * @param points: Pointer to the config-file-mode points.
   * @param pointCount: Number of points.
   * @return true: Set succeeded, false: Set failed.
   * @note Point values use sign-bit int16 encoding (bit15: 0=positive, 1=negative).
   * @note pointCount is limited to C4004_MAX_POINTS.
  */
  bool setConfigFileModePoints(const sPoint_t *points, uint16_t pointCount);

  /**
   * @fn getConfigFileModePoints
   * @brief Query and get range points in config-file mode.
   * @param points: Pointer to receive config-file-mode points.
   * @param pointCount: Pointer to receive point count.
   * @return true: Query succeeded, false: Query failed.
   * @note The points buffer must be able to hold at least C4004_MAX_POINTS points.
  */
  bool getConfigFileModePoints(sPoint_t *points, uint16_t *pointCount);

  /**
   * @fn getDetectionRangeMode
   * @brief Query current detection range mode.
   * @return eDetectionRangeMode_t: Current detection range mode.
  */
  eDetectionRangeMode_t getDetectionRangeMode(void);

  /**
   * @fn getLiveCount
   * @brief Get the live count. Only confirmed real person targets are counted.
   * @param mode: Data acquisition mode.
   * @n          eGetDataActive: Query latest data and update cache.
   * @n          eGetDataReport: Return cached data directly.
   * @return uint8_t: Live count after filtering.
  */
  uint8_t getLiveCount(eGetDataMode_t mode = eGetDataActive);

  /**
   * @fn setRealTimeReportInterval
   * @brief Set the real-time report interval.
   * @param time: Real-time report interval in seconds. Default: 1 s. Valid range: 1-3600 seconds.
   * @return true: Set succeeded, false: Set failed.
  */
  bool setRealTimeReportInterval(uint32_t time);

  /**
   * @fn getRealTimeReportInterval
   * @brief Get the real-time report interval.
   * @param time: Pointer to receive the real-time report interval in seconds.
   * @return true: Get succeeded, false: Get failed.
  */
  bool getRealTimeReportInterval(uint32_t *time);

  /**
   * @fn clearLiveCount
   * @brief Clear the live count detected by the sensor and restart detection/tracking from 0.
   * @n Use this when an interference object remains in the detection range and the sensor
   * @n cannot confirm or clear it by itself; call this API to refresh the live-count state.
   * @n Example: when the actual number of people does not match the live count, call this API
   * @n to clear and refresh; the sensor will re-identify people.
   * @return true: Clear succeeded, false: Clear failed.
  */
  bool clearLiveCount(void);

  /**
   * @fn setTrajectoryGenerationDistance
   * @brief Set the trajectory generation distance.
   * @n After a trajectory is generated, the distance the track must move before it is confirmed as a person.
   * @n Adjusts the judgment conditions of the live-count interface.
   * @param distanceCm: Trajectory generation distance, in cm. Default is 0 cm, valid range: 0-1000 cm.
   * @return true: Set succeeded, false: Set failed.
  */
  bool setTrajectoryGenerationDistance(uint32_t distanceCm);

  /**
   * @fn getTrajectoryGenerationDistance
   * @brief Get the trajectory generation distance.
   * @n After a trajectory is generated, the distance the track must move before it is confirmed as a person.
   * @n Adjusts the judgment conditions of the live-count interface.
   * @param distanceCm: Pointer to receive trajectory generation distance, in cm.
   * @return true: Get succeeded, false: Get failed.
  */
  bool getTrajectoryGenerationDistance(uint32_t *distanceCm);

  /**
   * @fn setTrajectoryLifetime
   * @brief Set the trajectory lifetime.
   * @n Adjusts the judgment conditions of the live-count interface.
   * @param time: Trajectory lifetime, in seconds. Default is 0 seconds, valid range: 0-600 seconds.
   * @return true: Set succeeded, false: Set failed.
  */
  bool setTrajectoryLifetime(uint32_t time);

  /**
   * @fn getTrajectoryLifetime
   * @brief Get the trajectory lifetime.
   * @n Adjusts the judgment conditions of the live-count interface.
   * @param time: Pointer to receive trajectory lifetime, in seconds.
   * @return true: Get succeeded, false: Get failed.
  */
  bool getTrajectoryLifetime(uint32_t *time);

  /**
   * @fn setUnoccupiedTime
   * @brief Set the unoccupied time.
   * @n Period used to judge whether a point is a real person target.
   * @n If it is not a real person target, the target is automatically cleared after this period.
   * @param delayTime: Unoccupied time, in seconds. Default is 30 seconds, valid range: 5-3600 seconds.
   * @return true: Set succeeded, false: Set failed.
  */
  bool setUnoccupiedTime(uint32_t delayTime);

  /**
   * @fn getUnoccupiedTime
   * @brief Get the unoccupied time.
   * @n Period used to judge whether a point is a real person target.
   * @n If it is not a real person target, the target is automatically cleared after this period.
   * @param delayTime: Pointer to receive period time, in seconds.
   * @return true: Get succeeded, false: Get failed.
  */
  bool getUnoccupiedTime(uint32_t *delayTime);
```

## Compatibility

| 主板        | 通过 | 未通过 | 未测试 | 备注 |
| ----------- | :--: | :----: | :----: | ---- |
| Arduino uno |  √   |        |        |      |
| Mega2560    |  √   |        |        |      |
| Leonardo    |  √   |        |        |      |
| ESP32       |  √   |        |        |      |
| micro:bit   |      |        |   √    |      |

## History

- 2026/05/22 - V1.0.0 version

## Credits

Written by JiaLi(jia.li@dfrobot.com), 2026. (Welcome to our [website](https://www.dfrobot.com/))
