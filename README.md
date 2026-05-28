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
* Supports people count query/report, report interval, trajectory distance, hold time and no-person delay settings.

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
   * @brief Initialize the DFRobot C4004 sensor.
   * @return true: Initialization succeeded, false: Initialization failed.
  */
  bool begin(void);

  /**
   * @fn isConnected
   * @brief Check if the DFRobot C4004 sensor is connected.
   * @return true: Connected, false: Not connected.
  */
  bool isConnected(void);

  /**
   * @fn reset
   * @brief Reset the DFRobot C4004 sensor.
   * @return true: Reset succeeded, false: Reset failed.
  */
  bool reset(void);

  /**
   * @fn factoryReset
   * @brief Factory reset the DFRobot C4004 sensor.
   * @return true: Reset succeeded, false: Reset failed.
  */
  bool factoryReset(void);

  /**
   * @fn getHeartbeat
   * @brief Get the heartbeat status of the DFRobot C4004 sensor.
   * @param mode: Data acquisition mode.
   * @n          eGetDataActive: Actively obtain the latest heartbeat status.
   * @n          eGetDataReport: Obtain the latest heartbeat status from the last report.
   * @return true: Heartbeat detected, false: No heartbeat detected.
  */
  bool getHeartbeat(eGetDataMode_t mode = eGetDataActive);

  /**
   * @fn getReportedInfo
   * @brief Get the latest reported event from the DFRobot C4004 sensor.
   * @param timeoutMs: Maximum waiting time for the report，in milliseconds，default is 50.
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
  eReportedEvent_t getReportedInfo(uint16_t timeoutMs = 50);

  /**
   * @fn getProductModel
   * @brief Get the product model of the DFRobot C4004 sensor.
   * @return String: Product model.
  */
  String getProductModel(void);

  /**
   * @fn getProductID
   * @brief Get the product ID of the DFRobot C4004 sensor.
   * @return uint16_t: Product ID.
  */
  uint16_t getProductID(void);

  /**
   * @fn getHardwareVersion
   * @brief Get the hardware version of the DFRobot C4004 sensor.
   * @return String: Hardware version.
  */
  String getHardwareVersion(void);

  /**
   * @fn getFirmwareVersion
   * @brief Get the firmware version of the DFRobot C4004 sensor.
   * @return String: Firmware version.
  */
  String getFirmwareVersion(void);

  /**
   * @fn setInstallInfo
   * @brief Set the installation information of the DFRobot C4004 sensor.
   * @param info: Installation information.
   * @n          mode: DFRobot C4004 mounting mode.
   * @n          heightCm: DFRobot C4004 installation height, in cm.
   * @n          zAngle: DFRobot C4004 installation z-axis angle, in degrees.
   * @return true: Set succeeded, false: Set failed.
  */
  bool setInstallInfo(sInstallInfo_t &info);

  /**
   * @fn getInstallInfo
   * @brief Get the installation information of the DFRobot C4004 sensor.
   * @param info: Installation information.
   * @n          mode: DFRobot C4004 mounting mode.
   * @n          heightCm: DFRobot C4004 installation height, in cm.
   * @n          zAngle: DFRobot C4004 installation z-axis angle, in degrees.
   * @return true: Get succeeded, false: Get failed.
  */
  bool getInstallInfo(sInstallInfo_t *info);

  /**
   * @fn setInstallHigh
   * @brief Set the installation height of the DFRobot C4004 sensor.
   * @param hight: Installation height, in cm.
   * @return true: Set succeeded, false: Set failed.
  */
  bool setInstallHigh(int32_t hight);

  /**
   * @fn getInstallHigh
   * @brief Get the installation height of the DFRobot C4004 sensor.
   * @param hight: Pointer to receive installation height, in cm.
   * @return true: Get succeeded, false: Get failed.
  */
  bool getInstallHigh(int *hight);

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
   * @brief Get the current presence detection result.
   * @return ePresenceState_t: Presence detection result.
   * @n          eNoPresence: No presence detected.
   * @n          ePresence: Presence detected.
   * @n          ePresenceUnknown: Unknown presence state.
  */
  ePresenceState_t getPresenceState(void);

  /**
   * @fn setMotionEnable
   * @brief Enable or disable the human motion detection function of the sensor.
   * @param enable: Enable or disable the human motion detection function.
   * @n          true: Enable, false: Disable.
   * @return true: Set succeeded, false: Set failed.
  */
  eMotionState_t getMotionState(void);

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
   * @fn getTargetInfo
   * @brief Get the target information of the DFRobot C4004 sensor.
   * @param index: Target index.
   * @param targetInfo: Pointer to receive the target information.
   * @param mode: Data acquisition mode.
   * @n          eGetDataActive: Query latest target information before reading.
   * @n          eGetDataReport: Read target information from cached report data.
   * @return true: Get succeeded, false: Get failed.
  */
  bool getTargetInfo(uint8_t index, sTargetInfo_t *targetInfo, eGetDataMode_t mode = eGetDataActive);

  /**
   * @fn getTargetList
   * @brief Get the list of target information of the DFRobot C4004 sensor.
   * @param targetBuf: Pointer to receive the target information list.
   * @param maxCount: Maximum number of targets to be read.
   * @param mode: Data acquisition mode.
   * @n          eGetDataActive: Query latest target information before reading.
   * @n          eGetDataReport: Read target information from cached report data.
   * @return uint8_t: Number of targets read.
  */
  uint8_t getTargetList(sTargetInfo_t *targetBuf, uint8_t maxCount, eGetDataMode_t mode = eGetDataActive);

  /**
   * @fn getTargetCount
   * @brief Get the number of targets detected by the DFRobot C4004 sensor.
   * @return uint8_t: Number of targets.
  */
  uint8_t getTargetCount(void);

  /**
   * @fn setTrajectoryLed
   * @brief Enable or disable the LED of the DFRobot C4004 sensor during trajectory tracking.
   * @param enable: Enable or disable the tag detection function.
  */
  bool setTrajectoryLed(bool enable);

  /**
   * @fn setMotionLed
   * @brief Enable or disable the LED of the DFRobot C4004 sensor during human motion detection.
   * @param enable: Enable or disable the tag detection function.
   * @return true: Set succeeded, false: Set failed.
  */
  bool setMotionLed(bool enable);

  /**
   * @fn getTrajectoryLed
   * @brief Get the LED status of the DFRobot C4004 sensor during trajectory tracking.
   * @return true: LED is enabled, false: LED is disabled.
  */
  bool getTrajectoryLed(void);

  /**
   * @fn getMotionLed
   * @brief Get the LED status of the DFRobot C4004 sensor during human motion detection.
   * @return true: LED is enabled, false: LED is disabled.
  */
  bool getMotionLed(void);

  /**
   * getTags
   * @brief Obtain all tag configuration information.
   * @param tags: Pointer to receive the tag configuration.
   * @param maxTags: Maximum number of tags to be read.
   * @param mode: Data acquisition mode.
   * @n          eGetDataActive: Query latest tag configuration before reading.
   * @n          eGetDataReport: Read tag configuration from cached data.
   * @return uint8_t: Number of tags read.
  */
  uint8_t getTags(sTagConfig_t *tags, uint8_t maxTags, eGetDataMode_t mode = eGetDataActive);

  /**
   * @fn setTag
   * @brief Set one tag using size mode.
   * @param tag: Tag configuration.
   * @n          tagIndex: Tag index.
   * @n          tagType: Tag type.
   * @n          scopeType: Tag range type.
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
   * @note This API reads report cache only. Call getReportedInfo() first to receive new report data.
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
  bool setFourSidedRangeMode(sFourSidedRange &range);

  /**
   * @fn getFourSidedRangeMode
   * @brief Query and get four-side boundary detection range.
   * @param range: Pointer to receive boundary range settings.
   * @return true: Get succeeded, false: Get failed.
  */
  bool getFourSidedRangeMode(sFourSidedRange *range);

  /**
   * @fn setTrajectoryRangeMode
   * @brief Start trajectory-range learning or use the learned trajectory range.
   * @param learning: Trajectory-range learning switch.
   * @n          true: Start learning trajectory range, false: Use trajectory range mode without learning.
   * @return true: Set succeeded, false: Set failed.
  */
  bool setTrajectoryRangeMode(bool learning);

  /**
   * @fn getTrajectoryRangeMode
   * @brief Query and get range points in trajectory mode (mode 0x05).
   * @param points: Pointer to receive trajectory-mode points.
   * @param pointCount: Pointer to receive point count.
   * @return true: Query succeeded, false: Query failed.
   * @note The points buffer must be able to hold at least MAX_POINTS points.
  */
  bool getTrajectoryRangeMode(sPoint_t *points, uint16_t *pointCount);

  /**
   * @fn setConfigFileModePoints
   * @brief Set detection range points using config-file mode (mode 0x06).
   * @param points: Pointer to the config-file-mode points.
   * @param pointCount: Number of points.
   * @return true: Set succeeded, false: Set failed.
   * @note Point values use sign-bit int16 encoding (bit15: 0=positive, 1=negative).
   * @note pointCount is limited to MAX_POINTS.
  */
  bool setConfigFileModePoints(const sPoint_t *points, uint16_t pointCount);

  /**
   * @fn getConfigFileModePoints
   * @brief Query and get range points in config-file mode (mode 0x06).
   * @param points: Pointer to receive config-file-mode points.
   * @param pointCount: Pointer to receive point count.
   * @return true: Query succeeded, false: Query failed.
   * @note The points buffer must be able to hold at least MAX_POINTS points.
  */
  bool getConfigFileModePoints(sPoint_t *points, uint16_t *pointCount);

  /**
   * @fn getDetectionRangeMode
   * @brief Query current detection range mode.
   * @return eDetectionRangeMode_t: Current detection range mode.
  */
  eDetectionRangeMode_t getDetectionRangeMode(void);

  /**
   * @fn getPeopleCountInfo
   * @brief Get people count.
   * @param mode: Data acquisition mode.
   * @n          eGetDataActive: Query latest data and update cache.
   * @n          eGetDataReport: Return cached data directly.
   * @return uint8_t: People count (maximum count reported by module).
  */
  uint8_t getPeopleCountInfo(eGetDataMode_t mode = eGetDataActive);

  /**
   * @fn setPeopleReportInterval
   * @brief Set people count report interval.
   * @param interval: Report interval, in seconds.
   * @return true: Set succeeded, false: Set failed.
  */
  bool setPeopleReportInterval(uint32_t interval);

  /**
   * @fn getPeopleReportInterval
   * @brief Get people count report interval.
   * @param interval: Pointer to receive report interval, in seconds.
   * @return true: Get succeeded, false: Get failed.
  */
  bool getPeopleReportInterval(uint32_t *interval);

  /**
   * @fn clearPeopleCount
   * @brief Clear people count statistics.
   * @return true: Clear succeeded, false: Clear failed.
  */
  bool clearPeopleCount(void);

  /**
   * @fn setTrajectoryGenerateDistance
   * @brief Set trajectory generation distance threshold.
   * @param distanceCm: Distance threshold, in cm.
   * @return true: Set succeeded, false: Set failed.
  */
  bool setTrajectoryGenerateDistance(uint32_t distanceCm);

  /**
   * @fn getTrajectoryGenerateDistance
   * @brief Get trajectory generation distance threshold.
   * @param distanceCm: Pointer to receive distance threshold, in cm.
   * @return true: Get succeeded, false: Get failed.
  */
  bool getTrajectoryGenerateDistance(uint32_t *distanceCm);

  /**
   * @fn setTrajectoryHoldTime
   * @brief Set trajectory hold time.
   * @param holdTime: Hold time, in seconds.
   * @return true: Set succeeded, false: Set failed.
  */
  bool setTrajectoryHoldTime(uint32_t holdTime);

  /**
   * @fn getTrajectoryHoldTime
   * @brief Get trajectory hold time.
   * @param holdTime: Pointer to receive hold time, in seconds.
   * @return true: Get succeeded, false: Get failed.
  */
  bool getTrajectoryHoldTime(uint32_t *holdTime);

  /**
   * @fn setNoPersonDelay
   * @brief Set no-person delay time.
   * @param delayTime: Delay time, in seconds.
   * @return true: Set succeeded, false: Set failed.
  */
  bool setNoPersonDelay(uint32_t delayTime);

  /**
   * @fn getNoPersonDelay
   * @brief Get no-person delay time.
   * @param delayTime: Pointer to receive delay time, in seconds.
   * @return true: Get succeeded, false: Get failed.
  */
  bool getNoPersonDelay(uint32_t *delayTime);
```

## Compatibility

MCU                | Work Well    | Work Wrong   | Untested    | Remarks
------------------ | :----------: | :----------: | :---------: | :----:
Arduino Uno        |      √       |              |             |
Arduino MEGA2560   |      √       |              |             |
Arduino Leonardo   |      √       |              |             |
FireBeetle-ESP32   |      √       |              |             |
Micro:bit          |              |              |      √      |

## History

- 2026/05/22 - V1.0.0 version

## Credits

Written by JiaLi(zhixin.liu@dfrobot.com), 2026. (Welcome to our [website](https://www.dfrobot.com/))
