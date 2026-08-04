/*!
 * @file DFRobot_C4004.h
 * @brief Arduino driver for the sensor module.
 * @details Provides common configuration, query, and report parsing APIs.
 * @copyright Copyright (c) 2026 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @license The MIT License (MIT)
 * @author JiaLi(jia.li@dfrobot.com)
 * @version V1.0.0
 * @date 2026-05-22
 * @url https://github.com/DFRobot/DFRobot_C4004
 */
#ifndef DFROBOT_C4004_H
#define DFROBOT_C4004_H

#include "Arduino.h"

#if defined(ESP8266) || defined(ARDUINO_AVR_UNO)
#include <SoftwareSerial.h>
#else
#include <HardwareSerial.h>
#endif

// #define ENABLE_DBG
#ifdef ENABLE_DBG
#define DBG(...)                 \
  {                              \
    Serial.print("[");           \
    Serial.print(__FUNCTION__);  \
    Serial.print("(): ");        \
    Serial.print(__LINE__);      \
    Serial.print(" ] ");         \
    Serial.println(__VA_ARGS__); \
  }
#else
#define DBG(...)
#endif

/**
 * @note
 * SoftSerial note: On  Arduino UNO and ESP8266 the library uses SoftwareSerial, which cannot
 * guarantee stable high-baud UART communication; prefer boards with HardwareSerial
 * when possible.
 * RAM note: these buffers cost ~2KB of SRAM. Requires an MCU with
 * at least 2KB SRAM. If RAM is tight or you see crashes/garbled RX, lower C4004_MAX_POINTS.
 */

#if defined(ARDUINO_AVR_UNO) || defined(ARDUINO_AVR_LEONARDO)
#define C4004_MAX_POINTS 64
#else
#define C4004_MAX_POINTS 150
#endif
#define C4004_MAX_PAYLOAD    (3 + C4004_MAX_POINTS * 4)
#define C4004_MAX_FRAME_SIZE (9 + C4004_MAX_PAYLOAD)
#define C4004_RX_RING_SIZE   (C4004_MAX_FRAME_SIZE + 128)
#define C4004_MAX_TARGETS    8

#define C4004_QUERY_DATA            0x0F
#define C4004_FRAME_HEAD1           0x53
#define C4004_FRAME_HEAD2           0x59
#define C4004_FRAME_TAIL1           0x54
#define C4004_FRAME_TAIL2           0x43
#define C4004_DEFAULT_TIMEOUT       200
#define C4004_RESET_TIMEOUT         300
#define C4004_FACTORY_RESET_TIMEOUT 350
#define C4004_TAG_SET_TIMEOUT       400
#define C4004_SET_RANGE_TIMEOUT     400
#define C4004_HEARTBEAT_TIMEOUT     90000UL

#define C4004_CTRL_SYSTEM          0x01
#define C4004_CTRL_PRODUCT_INFO    0x02
#define C4004_CTRL_OTA             0x03
#define C4004_CTRL_WORK_STATUS     0x05
#define C4004_CTRL_INSTALL_INFO    0x06
#define C4004_CTRL_DETECTION_RANGE 0x07
#define C4004_CTRL_PRESENCE        0x80
#define C4004_CTRL_TRAJECTORY      0x82
#define C4004_CTRL_FALL_DETECTION  0x83
#define C4004_CTRL_PEOPLE_COUNT    0x86

#define C4004_CMD_SYSTEM_HEARTBEAT_REPORT 0x01
#define C4004_CMD_SYSTEM_RESET            0x02
#define C4004_CMD_SYSTEM_FACTORY_RESET    0x03
#define C4004_CMD_SYSTEM_HEARTBEAT_QUERY  0x80

#define C4004_CMD_PRODUCT_MODEL_QUERY            0xA1
#define C4004_CMD_PRODUCT_ID_QUERY               0xA2
#define C4004_CMD_PRODUCT_HARDWARE_VERSION_QUERY 0xA3
#define C4004_CMD_PRODUCT_FIRMWARE_VERSION_QUERY 0xA4

#define C4004_CMD_WORK_STATUS_INIT_FINISHED_REPORT 0x01
#define C4004_CMD_WORK_STATUS_INIT_FINISHED_QUERY  0x81

#define C4004_CMD_INSTALL_SET_ANGLE    0x01
#define C4004_CMD_INSTALL_SET_HEIGHT   0x02
#define C4004_CMD_INSTALL_SET_MODE     0x06
#define C4004_CMD_INSTALL_QUERY_ANGLE  0x81
#define C4004_CMD_INSTALL_QUERY_HEIGHT 0x82
#define C4004_CMD_INSTALL_QUERY_MODE   0x86

#define C4004_CMD_PRESENCE_SET_ENABLE    0x00
#define C4004_CMD_PRESENCE_REPORT        0x01
#define C4004_CMD_PRESENCE_MOTION_REPORT 0x02
#define C4004_CMD_PRESENCE_QUERY_ENABLE  0x80
#define C4004_CMD_PRESENCE_QUERY_STATE   0x81
#define C4004_CMD_PRESENCE_QUERY_MOTION  0x82

#define C4004_CMD_TRAJECTORY_SET_ENABLE                   0x00
#define C4004_CMD_TRAJECTORY_TARGET_REPORT                0x02
#define C4004_CMD_TRAJECTORY_QUERY_ENABLE                 0x80
#define C4004_CMD_TRAJECTORY_QUERY_TARGET                 0x82
#define C4004_CMD_TRAJECTORY_SET_TRAJECTORY_LED           0x0B
#define C4004_CMD_TRAJECTORY_SET_MOTION_LED               0x0C
#define C4004_CMD_TRAJECTORY_SET_CHECK_TO_ACTIVE_FRAMES   0x0D
#define C4004_CMD_TRAJECTORY_QUERY_TRAJECTORY_LED         0x8B
#define C4004_CMD_TRAJECTORY_QUERY_MOTION_LED             0x8C
#define C4004_CMD_TRAJECTORY_QUERY_CHECK_TO_ACTIVE_FRAMES 0x8D

#define C4004_CMD_DETECTION_RANGE_QUERY_TAGS           0x91
#define C4004_CMD_DETECTION_RANGE_SET_TAG              0x11
#define C4004_CMD_DETECTION_RANGE_CLEAR_TAG            0x13
#define C4004_CMD_DETECTION_RANGE_SET_TAGS_FROM_CONFIG 0x19
#define C4004_CMD_DETECTION_RANGE_SET_RANGE            0x1A
#define C4004_CMD_DETECTION_RANGE_QUERY_RANGE          0x9A
#define C4004_CMD_DETECTION_RANGE_TAG_REPORT           0x1B

#define C4004_CMD_PEOPLE_COUNT_REPORT                     0x0A
#define C4004_CMD_PEOPLE_COUNT_QUERY_COUNT                0x8A
#define C4004_CMD_PEOPLE_COUNT_SET_REPORT_INTERVAL        0x0B
#define C4004_CMD_PEOPLE_COUNT_QUERY_REPORT_INTERVAL      0x8B
#define C4004_CMD_PEOPLE_COUNT_CLEAR_COUNT                0x11
#define C4004_CMD_PEOPLE_COUNT_SET_TRAJECTORY_DISTANCE    0x0E
#define C4004_CMD_PEOPLE_COUNT_QUERY_TRAJECTORY_DISTANCE  0x8E
#define C4004_CMD_PEOPLE_COUNT_SET_TRAJECTORY_HOLD_TIME   0x15
#define C4004_CMD_PEOPLE_COUNT_QUERY_TRAJECTORY_HOLD_TIME 0x95
#define C4004_CMD_PEOPLE_COUNT_SET_NO_PERSON_DELAY        0x17
#define C4004_CMD_PEOPLE_COUNT_QUERY_NO_PERSON_DELAY      0x97


class DFRobot_C4004 {
public:

  /**
   * @enum eReportedEvent_t
   * @brief Reported event type returned by getReportedEvent().
   */
  typedef enum {
    eEventNone         = 0x00,    ///> No complete frame in this round (including wait timeout)
    eEventTrajectory   = 0x01,    ///> Trajectory event
    eEventPresence     = 0x02,    ///> Presence event, used for presence detection
    eEventMotion       = 0x03,    ///> Motion event, used for motion detection
    eEventTag          = 0x04,    ///> Tag event, used for tag detection
    eEventHeartbeat    = 0x05,    ///> Heartbeat event, used for heartbeat detection
    eEventInitFinished = 0x06,    ///> Initialization finished event
    eEventPeopleCount  = 0x07,    ///> People count event, used for people count detection
    eEventUnknown      = 0xFE,    ///> Complete frame received, but event type is unrecognized
    eEventError        = 0xFF     ///> Internal error (e.g. null pointer); rare at application layer
  } eReportedEvent_t;

  /**
   * @enum eGetDataMode_t
   * @brief Data acquisition mode used by getter APIs.
   */
  typedef enum {
    eGetDataActive = 0x00, ///> Active mode: get data from the sensor immediately
    eGetDataReport = 0x01  ///> Report mode: get data from the sensor after a report is received
  } eGetDataMode_t;

  /**
   * @enum eInstallMode_t
   * @brief Sensor mounting mode.
   * @n Mounting is defined by zAngle: 0° = side, 90° = top (same underlying algorithm).
   */
  typedef enum {
    eUnknown = 0x00,
    eSide    = 0x01,    ///> Side mount (zAngle 0°). Default height 180 cm, recommended 180±20 cm
    eTop     = 0x02     ///> Top/ceiling mount (zAngle 90° only). Recommended height 220-280 cm
  } eInstallMode_t;

  /**
   * @enum ePresenceState_t
   * @brief Presence detection result.
   */
  typedef enum {
    eNoPresence = 0x00,    ///> No presence detected
    ePresence   = 0x01     ///> Presence detected
  } ePresenceState_t;

  /**
   * @enum eMotionState_t
   * @brief Human motion state.
   */
  typedef enum {
    eMotionNone   = 0x00,    ///> No motion state
    eMotionStatic = 0x01,    ///> Stationary
    eMotionActive = 0x02     ///> Active motion
  } eMotionState_t;

  /**
   * @enum eTargetFeature_t
   * @brief Target feature type in trajectory target data.
   */
  typedef enum {
    eStatic    = 0x00,    ///> Static target
    eMotion    = 0x01,    ///> Moving target
    eUncertain = 0x02     ///> Uncertain target feature
  } eTargetFeature_t;

  /**
   * @enum eTagType_t
   * @brief Tag property type.
   */
  typedef enum {
    eTagNone           = 0x00,    ///> Invalid/unused tag type
    eTagBoundary       = 0x01,    ///> Edge/boundary tag: reports Enter/Exit when a person passes through
    eTagApproachAway   = 0x02,    ///> Approach/away tag: reports Approach/Away relative to the tag zone
    eTagPeopleCounting = 0x03,    ///> People-counting tag: reports moving and stationary people counts in the zone
    eTagNoise          = 0x04     ///> Noise tag: marks the zone as an interference area
  } eTagType_t;

  /**
   * @enum eBoundaryDirection_t
   * @brief Boundary tag direction reported in tag events.
   */
  typedef enum {
    eEnter        = 0x00,    ///> Enter the detection range area
    eExit         = 0x01,    ///> Exit the detection range area
    eBoundaryNone = 0x02     ///> No boundary tag
  } eBoundaryDirection_t;

  /**
   * @enum eApproachAwayDirection_t
   * @brief Approach/away tag direction reported in tag events.
   */
  typedef enum {
    eApproach         = 0x00,    ///> Approach direction,approaching the tag area
    eAway             = 0x01,    ///> Away direction,leaving the tag area
    eApproachAwayNone = 0x02     ///> No approach/away tag
  } eApproachAwayDirection_t;

  /**
   * @enum eTagRangeType_t
   * @brief Tag range shape.
   */
  typedef enum {
    eCircle    = 0x00,    ///> Circle range
    eRectangle = 0x01,    ///> Rectangle range
  } eTagRangeType_t;

  /**
   * @enum eTagSetStatus_t
   * @brief Tag set status returned by setTag().
   */
  typedef enum {
    eTagSetCommError       = 0x00,    ///> Communication failed or response mismatch
    eTagSetSuccess         = 0x01,    ///> Tag set succeeded
    eTagSetTrackCountError = 0x02,    ///> Track count is not equal to 1
    eTagSetAlreadyUsed     = 0x03,    ///> Tag has been occupied
    eTagSetIndexOutOfRange = 0x04     ///> Tag index out of range
  } eTagSetStatus_t;

  /**
   * @enum eTrajRangeStatus_t
   * @brief Point-range query status returned by trajectory/config-file point query APIs.
   */
  typedef enum {
    eTrajRangeErrComm  = 0x00,    ///> Communication failed or response mismatch
    eTrajRangeOk       = 0x01,    ///> Query succeeded
    eTrajRangeErrParam = 0x02,    ///> Invalid parameter
    eTrajRangeErrMode  = 0x03,    ///> Current detection range mode mismatch
    eTrajRangeErrRes   = 0x04,    ///> Resource error
    eTrajRangeErrData  = 0x05     ///> Invalid or incomplete data
  } eTrajRangeStatus_t;

  /**
   * @enum eDetectionRangeMode_t
   * @brief Detection boundary mode defined by the protocol.
   */
  typedef enum {
    eRangeFourSide   = 0x04,    ///> Four-side detection boundary mode
    eRangeTrajectory = 0x05,    ///> Trajectory detection boundary mode
    eRangeConfigFile = 0x06,    ///> Config-file detection boundary mode
    eRangeUnknown    = 0xFF     ///> Unknown detection boundary mode
  } eDetectionRangeMode_t;

  /**
   * @struct sInstallInfo_t
   * @brief Complete installation information.
   * @n Sensor coordinate system (origin at the sensor, unit: cm):
   * @n - +Y: forward detection direction (away from the sensor face); object depth in front of the sensor.
   * @n - +X / -X: left / right lateral offset relative to the forward (+Y) direction.
   * @n - Target and tag positions (x, y) are reported in this same horizontal plane.
   * @n - zAngle: sensor pitch tilt in degrees (rotation that tips the +Y beam from horizontal toward the floor).
   * @n   0° = side mount (looking along +Y); 90° = top mount (looking down).
   */
  typedef struct {
    eInstallMode_t mode;      ///> Mounting mode, eSide or eTop (keep consistent with zAngle)
    uint16_t       heightCm;  ///> Side: default 180 cm, recommended 180±20 cm; Top: 220-280 cm (2.2-2.8 m)
    int16_t        xAngle;    ///> Unused and can be ignored
    int16_t        yAngle;    ///> Unused and can be ignored
    int16_t        zAngle;    ///> Pitch tilt in degrees. Default 0°. 0° = side (+Y horizontal), 90° = top
  } sInstallInfo_t;

  /**
   * @struct sPoint_t
   * @brief One point used by polygon/config boundary modes (X/Y in cm; see sInstallInfo_t).
   */
  typedef struct {
    int16_t x;    ///> X (cm), left/right relative to +Y
    int16_t y;    ///> Y (cm), depth in front of the sensor
  } sPoint_t;

  /**
   * @struct sTargetInfo_t
   * @brief One tracked target information block.
   * @note kinesia is the quantified human motion amplitude from the algorithm:
   * @n          0: No person
   * @n          1: Stationary (breathing only, no limb movement)
   * @n          2~30: Small limb movements
   * @n          31~60: Slow body movement
   * @n          61~100: Fast body movement
   */
  typedef struct {
    uint8_t          index;            ///> Target index
    uint8_t          kinesia;          ///> Quantified human motion amplitude, range 0~100
    eTargetFeature_t targetFeature;    ///> Target feature type
    int16_t          x;                ///> Target X coordinate (cm), left/right relative to +Y
    int16_t          y;                ///> Target Y coordinate (cm), depth in front of the sensor (+Y)
    int16_t          height;           ///> Unused and can be ignored
    int16_t          speed;            ///> Target speed, unit: cm/s. Positive: approaching the sensor; negative: leaving the sensor
  } sTargetInfo_t;

  /**
   * @struct sTagConfig_t
   * @brief Tag configuration used by tag query and batch config APIs.
   */
  typedef struct {
    uint8_t         tagIndex;       ///> Tag index
    eTagType_t      tagType;        ///> Tag type
    eTagRangeType_t scopeType;      ///> Tag range type
    uint8_t         ioIndex;        ///> IO index, 0: unused; 2-6: IO2-IO6 linkage. IO1 is fixed to the overall detection range and cannot be bound to a specific tag zone
    int16_t         centerX;        ///> Tag center X (cm), left/right relative to +Y
    int16_t         centerY;        ///> Tag center Y (cm), depth in front of the sensor (+Y)
    uint16_t        width;          ///> Rectangle: size along X-axis (cm); Circle: radius (cm)
    uint16_t        height;         ///> Rectangle: size along Y-axis (cm); Circle: ignored
  } sTagConfig_t;

  /**
   * @struct sTagInfo_t
   * @brief Last tag event decoded from an active report.
   * @note Tag event reports include ioIndex.
   * @note When tagType is eTagBoundary, motionNum and staticNum are invalid.
   * @note When tagType is eTagApproachAway, motionNum and staticNum are invalid.
   * @note When tagType is eTagPeopleCounting, enterExit and motionDir are invalid.
   * @note When tagType is eTagNoise, enterExit, motionDir, motionNum and staticNum are invalid.
   * @note When tagType is eTagNone, enterExit, motionDir, motionNum and staticNum are invalid.
   */
  typedef struct {
    uint8_t                  tagIndex;    ///> Tag index
    eTagType_t               tagType;     ///> Tag type
    uint8_t                  ioIndex;     ///> IO index
    int16_t                  centerX;     ///> Tag center X (cm), left/right relative to +Y
    int16_t                  centerY;     ///> Tag center Y (cm), depth in front of the sensor (+Y)
    eBoundaryDirection_t     enterExit;   ///> Enter/exit direction
    eApproachAwayDirection_t motionDir;   ///> Approach/away direction
    uint8_t                  motionNum;   ///> Moving number
    uint8_t                  staticNum;   ///> Static number
  } sTagInfo_t;

  /**
   * @struct sFourSidedRange_t
   * @brief Four-side detection boundary settings.
   * @n Boundaries are in the sensor X/Y plane (see sInstallInfo_t): X left/right, Y forward depth.
   */
  typedef struct {
    int16_t xMax;    ///> Maximum X boundary (cm), right side relative to +Y
    int16_t xMin;    ///> Minimum X boundary (cm), left side relative to +Y
    int16_t yMax;    ///> Maximum Y boundary (cm), far end of forward detection
    int16_t yMin;    ///> Minimum Y boundary (cm), near end (usually 0 at the sensor)
  } sFourSidedRange_t;

  /**
   * @struct sPacket_t
   * @brief UART frame packet used internally by the driver.
   */
  typedef struct {
    uint8_t  control;
    uint8_t  cmd;
    uint16_t len;
    uint8_t  data[C4004_MAX_PAYLOAD];
  } sPacket_t;

#if defined(ESP8266) || defined(ARDUINO_AVR_UNO)
  DFRobot_C4004(SoftwareSerial *pSerial, uint32_t baud);
#else
  DFRobot_C4004(HardwareSerial *pSerial, uint32_t baud, uint8_t rxpin = 0, uint8_t txpin = 0);
#endif

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
   * @brief Wait for and decode one report frame pushed by the sensor.
   * @param timeoutMs: Max time to wait for one complete UART report frame, in milliseconds (default: 50).
   * @n          The call blocks at most timeoutMs. If no complete frame arrives in time, returns eEventNone.
   * @n          If a frame arrives earlier, it returns as soon as the frame is decoded (may be shorter than timeoutMs).
   * @return eReportedEvent_t: Reported event type.
   * @n          eEventNone: No complete frame in this round (including wait timeout).
   * @n          eEventTrajectory: Trajectory tracking event detected.
   * @n          eEventPresence: Presence detection event detected.
   * @n          eEventMotion: Human motion event detected.
   * @n          eEventTag: Tag event detected.
   * @n          eEventHeartbeat: Heartbeat event detected.
   * @n          eEventInitFinished: Initialization finished event detected.
   * @n          eEventPeopleCount: People count event detected.
   * @n          eEventUnknown: Complete frame received, but event type is unrecognized.
   * @n          eEventError: Internal error (e.g. null pointer); rare at application layer.
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
   * @n          mode: Mounting mode, eSide or eTop. 
   * @n          heightCm: Installation height in cm.
   * @n            - Side (zAngle 0°): default 180 cm, recommended 180±20 cm (too low is easily blocked).
   * @n            - Top (zAngle 90°): recommended 220-280 cm (2.2-2.8 m).
   * @n          zAngle: Pitch tilt in degrees (default 0°). 0° = side (looking along +Y), 90° = top (looking down).
   * @n            See sInstallInfo_t for the sensor X/Y coordinate system relative to object positions.
   * @return true: Set succeeded, false: Set failed.
   * @note Invalid mode or height returns false. Out-of-range angles are clamped.
   * @note If the installation height is too low, it is easy to be blocked
  */
  bool setInstallInfo(sInstallInfo_t &info);

  /**
   * @fn getInstallInfo
   * @brief Get the installation information of the sensor.
   * @param pInfo: Installation information.
   * @n          mode: Mounting mode, eSide or eTop.
   * @n          heightCm: Installation height in cm.
   * @n            - Side (zAngle 0°): default 180 cm, recommended 180±20 cm (too low is easily blocked).
   * @n            - Top (zAngle 90°): recommended 220-280 cm (2.2-2.8 m).
   * @n          zAngle: Installation tilt angle in degrees. Defines mounting: 0° = side, 90° = top.
   * @return true: Get succeeded, false: Get failed.
  */
  bool getInstallInfo(sInstallInfo_t *pInfo);

  /**
   * @fn setInstallHeight
   * @brief Set the installation height of the sensor.
   * @param height: Installation height in cm.
   * @n            - Side (zAngle 0°): default 180 cm, recommended 180±20 cm.
   * @n            - Top (zAngle 90°): recommended 220-280 cm (2.2-2.8 m).
   * @return true: Set succeeded, false: Set failed.
   * @note If the installation height is too low, it is easy to be blocked
  */
  bool setInstallHeight(int32_t height);

  /**
   * @fn getInstallHeight
   * @brief Get the installation height of the sensor.
   * @param pHeight: Pointer to receive installation height in cm.
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
   * @param pEnable: Pointer to receive the enable state.
   * @n          true: Enabled, false: Disabled.
   * @return true: Get succeeded, false: Get failed.
  */
  bool getPresenceEnable(bool *pEnable);

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
   * @brief Get the current human motion state within the detection range.
   * @param mode: Data acquisition mode.
   * @n          eGetDataActive: Query latest data and update cache.
   * @n          eGetDataReport: Return cached data from the last report.
   * @return eMotionState_t: Motion state.
   * @n          eMotionNone: No motion state.
   * @n          eMotionStatic: Stationary.
   * @n          eMotionActive: Active motion.
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
   * @param pEnable: Pointer to receive the enable state.
   * @n          true: Enabled, false: Disabled.
   * @return true: Query succeeded, false: Query failed.
  */
  bool getTrajectoryTrackEnable(bool *pEnable);

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
   * @param pFrames: Pointer to receive frame count.
   * @return true: Query succeeded, false: Query failed.
  */
  bool getFrameGenerateCount(uint8_t *pFrames);

  /**
   * @fn getTargetList
   * @brief Get the list of target information of the sensor.
   * @param pTargetBuf: Pointer to receive the target information list.
   * @param maxCount: Maximum number of targets to be read.
   * @param mode: Data acquisition mode.
   * @n          eGetDataActive: Query latest target information before reading.
   * @n          eGetDataReport: Read target information from cached report data.
   * @return uint8_t: Number of targets read.
  */
  uint8_t getTargetList(sTargetInfo_t *pTargetBuf, uint8_t maxCount, eGetDataMode_t mode = eGetDataActive);

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
   * @param pTags: Pointer to receive the tag configuration.
   * @param maxTags: Maximum number of tags to be written into tags.
   * @param mode: Data acquisition mode kept for compatibility.
   * @n          eGetDataActive: Query latest tag configuration from the device.
   * @n          eGetDataReport: Currently behaves the same as eGetDataActive.
   * @n          ioIndex in each tag: 0 means unused; 2-6 maps to IO2-IO6.
   * @return uint8_t: Actual number of tags returned by the device.
  */
  uint8_t getTags(sTagConfig_t *pTags, uint8_t maxTags, eGetDataMode_t mode = eGetDataActive);

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
   * @note Invalid tagType, scopeType or ioIndex returns eTagSetCommError before sending command.
   * @note When setting labels using this API, it is necessary to ensure that the number of tracks is 1.
   * @note Set up to 32 tags at most.
  */
  eTagSetStatus_t setTag(const sTagConfig_t &tag);

  /**
   * @fn clearTag
   * @brief Clear the tag configuration.
   * @param tagIndex: Tag index (1-byte index in protocol payload, 0-254).
   * @return true: Clear succeeded, false: Clear failed.
   * @note 0xFF is reserved for clearAllTags(); do not pass it to clearTag().
   * @note Device returns 0xFE in response when tag index is out of range.
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
   * @param pTags: Pointer to the tag configuration list.
   * @param tagCount: Number of tags in the list.
   * @n          ioIndex in each tag: 0 means unused; 2-6 maps to IO2-IO6.
   * @return true: Set succeeded, false: Set failed.
   * @note The labels can be set in the form of coordinates, and there is no need to meet the requirement that the number of tracks is 1
   * @note Set up to 32 tags at most.
   * @note Invalid tagType, scopeType or ioIndex in any tag returns false before sending command.
  */
  bool setTagsFromConfig(const sTagConfig_t *pTags, uint8_t tagCount);

  /**
   * @fn getTagInfo
   * @brief Get the last tag event decoded from active report packet (CTRL 0x07, CMD 0x1B).
   * @param pTagInfo: Pointer to receive reported tag event information.
   * @return true: Get succeeded, false: No valid reported tag event or invalid parameter.
   * @note This API reads report cache only. Call getReportedEvent() to receive new report data first.
   * @note Tag event reports include ioIndex.
  */
  bool getTagInfo(sTagInfo_t *pTagInfo);

  /**
   * @fn setFourSidedRangeMode
   * @brief Set four-side boundary detection range.
   * @param range: Boundary range settings.
   * @n          xMax: Maximum x boundary, in cm.
   * @n          xMin: Minimum x boundary, in cm.
   * @n          yMax: Maximum y boundary, in cm.
   * @n          yMin: Minimum y boundary, in cm.
   * @return true: Set succeeded, false: Set failed.
  */
  bool setFourSidedRangeMode(sFourSidedRange_t &range);

  /**
   * @fn getFourSidedRangeMode
   * @brief Query and get four-side boundary detection range.
   * @param pRange: Pointer to receive boundary range settings.
   * @return true: Get succeeded, false: Get failed.
  */
  bool getFourSidedRangeMode(sFourSidedRange_t *pRange);

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
   * @param pPoints: Pointer to receive trajectory-mode points.
   * @param pPointCount: Pointer to receive point count.
   * @return true: Query succeeded, false: Query failed.
   * @note The points buffer must be able to hold at least C4004_MAX_POINTS points.
  */
  bool getTrajectoryRangeMode(sPoint_t *pPoints, uint16_t *pPointCount);

  /**
   * @fn setConfigFileModePoints
   * @brief Set detection range points using config-file mode.
   * @param pPoints: Pointer to the config-file-mode points.
   * @param pointCount: Number of points.
   * @return true: Set succeeded, false: Set failed.
   * @note Point values use sign-bit int16 encoding (bit15: 0=positive, 1=negative).
   * @note pointCount is limited to C4004_MAX_POINTS.
  */
  bool setConfigFileModePoints(const sPoint_t *pPoints, uint16_t pointCount);

  /**
   * @fn getConfigFileModePoints
   * @brief Query and get range points in config-file mode.
   * @param pPoints: Pointer to receive config-file-mode points.
   * @param pPointCount: Pointer to receive point count.
   * @return true: Query succeeded, false: Query failed.
   * @note The points buffer must be able to hold at least C4004_MAX_POINTS points.
  */
  bool getConfigFileModePoints(sPoint_t *pPoints, uint16_t *pPointCount);

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
   * @param pTime: Pointer to receive the real-time report interval in seconds.
   * @return true: Get succeeded, false: Get failed.
  */
  bool getRealTimeReportInterval(uint32_t *pTime);

  /**
   * @fn clearLiveCount
   * @brief Clear the live count detected by the sensor and restart detection/tracking from 0.
   * @n Use this when an interference object remains in the detection range and the sensor
   * @n cannot confirm or clear it by itself; call this API to refresh the live-count state.
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
   * @param pDistanceCm: Pointer to receive trajectory generation distance, in cm.
   * @return true: Get succeeded, false: Get failed.
  */
  bool getTrajectoryGenerationDistance(uint32_t *pDistanceCm);

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
   * @param pTime: Pointer to receive trajectory lifetime, in seconds.
   * @return true: Get succeeded, false: Get failed.
  */
  bool getTrajectoryLifetime(uint32_t *pTime);

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
   * @param pDelayTime: Pointer to receive unoccupied time, in seconds.
   * @return true: Get succeeded, false: Get failed.
  */
  bool getUnoccupiedTime(uint32_t *pDelayTime);

protected:
  bool             isInitFinished(void);
  String           getProductModel(void);
  uint16_t         getProductID(void);
  bool             sendCommand(uint8_t control, uint8_t cmd, const uint8_t *pData, uint16_t len);
  bool             requestFrame(uint8_t control, uint8_t cmd, const uint8_t *pData, uint16_t len, sPacket_t *pResponse, uint16_t timeoutMs = 200);
  bool             readFrame(sPacket_t *pPacket, uint16_t timeoutMs);
  bool             readByte(uint8_t *pValue, uint16_t timeoutMs);
  void             flushInput(void);
  eReportedEvent_t handlePacket(const sPacket_t *pPacket);
  eReportedEvent_t classifyPacket(const sPacket_t *pPacket);

  bool   queryByte(uint8_t control, uint8_t cmd, uint8_t *pValue);
  bool   setByte(uint8_t control, uint8_t cmd, uint8_t value);
  bool   queryUint32(uint8_t control, uint8_t cmd, uint32_t *pValue);
  bool   setUint32(uint8_t control, uint8_t cmd, uint32_t value);
  String queryString(uint8_t control, uint8_t cmd);

  void    parseTargets(const uint8_t *pData, uint16_t len);
  uint8_t parseTagList(const uint8_t *pData, uint16_t len, sTagConfig_t *pTags, uint8_t maxTags);
  void    parseTagEvent(const uint8_t *pData, uint16_t len);
  void    parseBoundaryRange(const uint8_t *pData, uint16_t len);
  void    parsePeopleCount(const uint8_t *pData, uint16_t len);

  uint16_t readUint16(const uint8_t *pData) const;
  int16_t  readInt16(const uint8_t *pData) const;
  int16_t  readSignBitInt16(const uint8_t *pData) const;
  uint32_t readUint32(const uint8_t *pData) const;
  void     writeUint16(uint8_t *pData, uint16_t value) const;
  void     writeInt16(uint8_t *pData, int16_t value) const;
  void     writeSignBitInt16(uint8_t *pData, int16_t value) const;
  void     writeUint32(uint8_t *pData, uint32_t value) const;
  void     initObject(void);

  void pumpRx(void);
  void resetRxParser(void);
  void discardRxRing(void);
  bool takePendingFrame(sPacket_t *pPacket);
  void rxPushByte(uint8_t value);
  bool rxPopByte(uint8_t *pValue);
  void feedAsmByte(uint8_t value);
  void logRxPacket(const sPacket_t *pPacket, uint8_t recvChecksum);

private:
  enum eRxAsmState_t {
    eRxAsmSyncH1 = 0,
    eRxAsmSyncH2,
    eRxAsmCtrl,
    eRxAsmCmd,
    eRxAsmLenHi,
    eRxAsmLenLo,
    eRxAsmPayload,
    eRxAsmChecksum,
    eRxAsmTail1,
    eRxAsmTail2
  };

  Stream *_s;
#if defined(ESP8266) || defined(ARDUINO_AVR_UNO)
  SoftwareSerial *_serial;
#else
  HardwareSerial *_serial;
#endif
  uint32_t          _baud;
  uint8_t           _rxpin;
  uint8_t           _txpin;
  sPacket_t         _rxPacket;
  bool              _heartbeat;
  bool              _initFinished;
  ePresenceState_t  _presence;
  eMotionState_t    _motionState;
  uint8_t           _trajectoryLed;
  uint8_t           _motionLed;
  sTargetInfo_t     _targets[C4004_MAX_TARGETS];
  uint8_t           _targetCount;
  sTagInfo_t        _tagInfo;
  bool              _tagInfoValid;
  eDetectionRangeMode_t _rangeMode;
  sFourSidedRange_t _rangeInfo;
  uint8_t           _peopleCount;
  uint8_t           _rxRing[C4004_RX_RING_SIZE];
  uint16_t          _rxHead;
  uint16_t          _rxTail;
  eRxAsmState_t     _asmState;
  uint8_t           _asmChecksum;
  uint16_t          _asmIdx;
  uint8_t           _asmRecvChecksum;
  sPacket_t         _pendingPacket;
  bool              _pendingValid;
};

#endif
