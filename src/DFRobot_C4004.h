/*!
 * @file DFRobot_C4004.h
 * @brief Arduino driver for the DFRobot C4004 sensor.
 * @details Provides common configuration, query, and report parsing APIs.
 * @copyright Copyright (c) 2026 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @license The MIT License (MIT)
 * @author JiaLi(zhixin.liu@dfrobot.com)
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
#define DBG(...)                    \
  {                                 \
    Serial.print("[");              \
    Serial.print(__FUNCTION__);     \
    Serial.print("(): ");           \
    Serial.print(__LINE__);         \
    Serial.print(" ] ");            \
    Serial.println(__VA_ARGS__);    \
  }
#else
#define DBG(...)
#endif

#define MAX_TARGETS           8
#define MAX_TAGS              12
#define MAX_POINTS            150
#define MAX_PAYLOAD           (3 + MAX_POINTS * 4)
#define QUERY_DATA            0x0F
#define FRAME_HEAD1           0x53
#define FRAME_HEAD2           0x59
#define FRAME_TAIL1           0x54
#define FRAME_TAIL2           0x43
#define DEFAULT_TIMEOUT       200
#define HEARTBEAT_TIMEOUT     90000UL

#define CTRL_SYSTEM           0x01
#define CTRL_PRODUCT_INFO     0x02
#define CTRL_OTA              0x03
#define CTRL_WORK_STATUS      0x05
#define CTRL_INSTALL_INFO     0x06
#define CTRL_DETECTION_RANGE  0x07
#define CTRL_PRESENCE         0x80
#define CTRL_TRAJECTORY       0x82
#define CTRL_FALL_DETECTION   0x83
#define CTRL_PEOPLE_COUNT     0x86

#define CMD_SYSTEM_HEARTBEAT_REPORT      0x01
#define CMD_SYSTEM_RESET                 0x02
#define CMD_SYSTEM_FACTORY_RESET         0x03
#define CMD_SYSTEM_HEARTBEAT_QUERY       0x80

#define CMD_PRODUCT_MODEL_QUERY               0xA1
#define CMD_PRODUCT_ID_QUERY                  0xA2
#define CMD_PRODUCT_HARDWARE_VERSION_QUERY    0xA3
#define CMD_PRODUCT_FIRMWARE_VERSION_QUERY    0xA4

#define CMD_WORK_STATUS_INIT_FINISHED_REPORT  0x01
#define CMD_WORK_STATUS_INIT_FINISHED_QUERY   0x81

#define CMD_INSTALL_SET_ANGLE            0x01
#define CMD_INSTALL_SET_HEIGHT           0x02
#define CMD_INSTALL_SET_MODE             0x06
#define CMD_INSTALL_QUERY_ANGLE          0x81
#define CMD_INSTALL_QUERY_HEIGHT         0x82
#define CMD_INSTALL_QUERY_MODE           0x86

#define CMD_PRESENCE_SET_ENABLE          0x00
#define CMD_PRESENCE_REPORT              0x01
#define CMD_PRESENCE_MOTION_REPORT       0x02
#define CMD_PRESENCE_QUERY_ENABLE        0x80
#define CMD_PRESENCE_QUERY_STATE         0x81
#define CMD_PRESENCE_QUERY_MOTION        0x82

#define CMD_TRAJECTORY_SET_ENABLE             0x00
#define CMD_TRAJECTORY_TARGET_REPORT          0x02
#define CMD_TRAJECTORY_QUERY_ENABLE           0x80
#define CMD_TRAJECTORY_QUERY_TARGET           0x82
#define CMD_TRAJECTORY_SET_TRAJECTORY_LED     0x0B
#define CMD_TRAJECTORY_SET_MOTION_LED         0x0C
#define CMD_TRAJECTORY_SET_CHECK_TO_ACTIVE_FRAMES    0x0D
#define CMD_TRAJECTORY_QUERY_TRAJECTORY_LED   0x8B
#define CMD_TRAJECTORY_QUERY_MOTION_LED       0x8C
#define CMD_TRAJECTORY_QUERY_CHECK_TO_ACTIVE_FRAMES  0x8D

#define CMD_DETECTION_RANGE_QUERY_TAGS            0x91
#define CMD_DETECTION_RANGE_SET_TAG               0x11
#define CMD_DETECTION_RANGE_CLEAR_TAG             0x13
#define CMD_DETECTION_RANGE_SET_TAGS_FROM_CONFIG  0x19
#define CMD_DETECTION_RANGE_SET_RANGE             0x1A
#define CMD_DETECTION_RANGE_QUERY_RANGE           0x9A
#define CMD_DETECTION_RANGE_TAG_REPORT            0x1B

#define CMD_PEOPLE_COUNT_REPORT                     0x0A
#define CMD_PEOPLE_COUNT_QUERY_COUNT                0x8A
#define CMD_PEOPLE_COUNT_SET_REPORT_INTERVAL        0x0B
#define CMD_PEOPLE_COUNT_QUERY_REPORT_INTERVAL      0x8B
#define CMD_PEOPLE_COUNT_CLEAR_COUNT                0x11
#define CMD_PEOPLE_COUNT_SET_TRAJECTORY_DISTANCE    0x0E
#define CMD_PEOPLE_COUNT_QUERY_TRAJECTORY_DISTANCE  0x8E
#define CMD_PEOPLE_COUNT_SET_TRAJECTORY_HOLD_TIME   0x15
#define CMD_PEOPLE_COUNT_QUERY_TRAJECTORY_HOLD_TIME 0x95
#define CMD_PEOPLE_COUNT_SET_NO_PERSON_DELAY        0x17
#define CMD_PEOPLE_COUNT_QUERY_NO_PERSON_DELAY      0x97

/**
 * @enum eReportedEvent_t
 * @brief Reported event type returned by getReportedInfo().
 */
typedef enum {
  eEventNone         = 0x00,
  eEventTrajectory   = 0x01,
  eEventPresence     = 0x02,
  eEventMotion       = 0x03,
  eEventTag          = 0x04,
  eEventHeartbeat    = 0x05,
  eEventInitFinished = 0x06,
  eEventPeopleCount  = 0x07,
  eEventUnknown      = 0xFE,
  eEventError        = 0xFF
} eReportedEvent_t;

/**
 * @enum eGetDataMode_t
 * @brief Data acquisition mode used by getter APIs.
 */
typedef enum {
  eGetDataActive = 0x00,
  eGetDataReport = 0x01
} eGetDataMode_t;

/**
 * @enum eInstallMode_t
 * @brief DFRobot C4004 mounting mode.
 */
typedef enum {
  eInstallModeUnknown = 0x00,
  eInstallModeSide    = 0x01,
  eInstallModeTop     = 0x02
} eInstallMode_t;

/**
 * @enum ePresenceState_t
 * @brief Presence detection result.
 */
typedef enum {
  eNoPresence      = 0x00,
  ePresence        = 0x01,
  ePresenceUnknown = 0xFF
} ePresenceState_t;

/**
 * @enum eMotionState_t
 * @brief Human motion state.
 */
typedef enum {
  eMotionNone    = 0x00,
  eMotionStatic  = 0x01,
  eMotionActive  = 0x02,
  eMotionUnknown = 0xFF
} eMotionState_t;

/**
 * @enum eTargetFeature_t
 * @brief Target feature type in trajectory target data.
 */
typedef enum {
  eStatic    = 0x00,
  eMotion    = 0x01,
  eUncertain = 0x02,
  eUnknown   = 0xFF
} eTargetFeature_t;

/**
 * @enum eTagType_t
 * @brief Tag property type.
 */
typedef enum {
  eTagTypeNone            = 0x00,
  eTagTypeEnterExit       = 0x01,  ///> Enter or exit mode
  eTagTypeApproachAway    = 0x02,  ///> Near and far mode
  eTagTypePeopleCounting  = 0x03,  ///> Moving, stationary, people counting
  eTagTypeNoise           = 0x04   ///> Noise
} eTagType_t;

/**
 * @enum eTagRangeType_t
 * @brief Tag range shape.
 */
typedef enum {
  eTagRangeCircle    = 0x00,
  eTagRangeRectangle = 0x01
} eTagRangeType_t;

/**
 * @enum eTagSetStatus_t
 * @brief Tag set status code returned by CMD_DETECTION_RANGE_SET_TAG.
 */
typedef enum {
  eTagSetCommError       = 0x00,
  eTagSetSuccess         = 0x01,
  eTagSetTrackCountError = 0x02,
  eTagSetAlreadyUsed     = 0x03,
  eTagSetIndexOutOfRange = 0x04
} eTagSetStatus_t;

/**
 * @enum eTrajRangeStatus_t
 * @brief Point-range query status returned by trajectory/config-file point query APIs.
 */
typedef enum {
  eTrajRangeErrComm  = 0x00,
  eTrajRangeOk       = 0x01,
  eTrajRangeErrParam = 0x02,
  eTrajRangeErrMode  = 0x03,
  eTrajRangeErrRes   = 0x04,
  eTrajRangeErrData  = 0x05
} eTrajRangeStatus_t;

/**
 * @enum eDetectionRangeMode_t
 * @brief Detection boundary mode defined by the DFRobot C4004 protocol.
 */
typedef enum {
  eRangeSideDefault      = 0x00,
  eRangeSideLeftEdge     = 0x01,
  eRangeSideRightEdge    = 0x02,
  eRangeHotelCorridor    = 0x03,
  eRangeFourSide = 0x04,
  eRangeTrajectory       = 0x05,
  eRangeConfigFile       = 0x06,
  eRangeNoBoundary       = 0x07,
  eRangeTopDefault       = 0x08,
  eRangeTopLeftEdge      = 0x09,
  eRangeTopRightEdge     = 0x0A,
  eRangeUnknown          = 0xFF
} eDetectionRangeMode_t;

/**
 * @struct sInstallInfo_t
 * @brief Complete installation information.
 */
typedef struct {
  eInstallMode_t mode;
  uint16_t heightCm;
  int16_t xAngle;         // Unit: degree
  int16_t yAngle;
  int16_t zAngle;
} sInstallInfo_t;

/**
 * @struct sTargetInfo_t
 * @brief One tracked target information block.
 */
typedef struct {
  uint8_t index;
  union {
    uint8_t targetSize;
    uint8_t kinesia;     // Backward-compatible alias for targetSize.
  };
  eTargetFeature_t targetFeature;
  int16_t x;
  int16_t y;
  int16_t height;
  int16_t speed;
} sTargetInfo_t;

/**
 * @struct sPoint_t
 * @brief One point used by polygon/config boundary modes.
 */
typedef struct {
  int16_t x;
  int16_t y;
} sPoint_t;

/**
 * @struct sTagConfig_t
 * @brief Tag configuration used by tag query and batch config APIs.
 */
typedef struct {
  uint8_t tagIndex;
  eTagType_t tagType;
  eTagRangeType_t scopeType;
  uint8_t ioIndex;       // 0: unused; 2-6: IO2-IO6 linkage
  int16_t centerX;
  int16_t centerY;
  uint16_t width;
  uint16_t height;
} sTagConfig_t;

/**
 * @struct sTagInfo_t
 * @brief Last tag event decoded from an active report.
 * @note Tag event reports do not include ioIndex; IO linkage belongs to tag configuration.
 */
typedef struct {
  uint8_t tagIndex;
  eTagType_t tagType;
  int16_t centerX;
  int16_t centerY;
  uint8_t enterExit;
  uint8_t motionDir;
  uint8_t motionNum;
  uint8_t staticNum;
} sTagInfo_t;

/**
 * @struct sFourSidedRange
 * @brief Four-side detection boundary settings.
 */
typedef struct {
  eDetectionRangeMode_t mode;
  int16_t xPositiveCm;
  int16_t xNegativeCm;
  int16_t yPositiveCm;
  int16_t yNegativeCm;
} sFourSidedRange;

/**
 * @struct sPacket_t
 * @brief UART frame packet used internally by the driver.
 */
typedef struct {
  uint8_t control;
  uint8_t cmd;
  uint16_t len;
  uint8_t data[MAX_PAYLOAD];
} sPacket_t;

class DFRobot_C4004 {
public:

#if defined(ESP8266) || defined(ARDUINO_AVR_UNO)
  DFRobot_C4004(SoftwareSerial *sSerial, uint32_t baud);
#else
  DFRobot_C4004(HardwareSerial *hSerial, uint32_t baud, uint8_t rxpin = 0, uint8_t txpin = 0);
#endif

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
   * @fn setCheckToActiveFrames
   * @brief Set the frame count used to confirm transition from check state to active state.
   * @param frames: Frame count, valid range: 1-7.
   * @return true: Set succeeded, false: Set failed.
  */
  bool setCheckToActiveFrames(uint8_t frames);

  /**
   * @fn getCheckToActiveFrames
   * @brief Query the frame count used to confirm transition from check state to active state.
   * @param frames: Pointer to receive frame count.
   * @return true: Query succeeded, false: Query failed.
  */
  bool getCheckToActiveFrames(uint8_t *frames);

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
   * @n          ioIndex in each tag: 0 means unused; 2-6 maps to IO2-IO6.
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
   * @note This API reads report cache only. Call getReportedInfo() to receive new report data first.
   * @note Tag event reports do not include ioIndex. Query tag configuration with getTags() when IO linkage is needed.
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
   * @fn getPeopleTime
   * @brief Get people count.
   * @param mode: Data acquisition mode.
   * @n          eGetDataActive: Query latest data and update cache.
   * @n          eGetDataReport: Return cached data directly.
   * @return uint8_t: People count (maximum count reported by module).
  */
  uint8_t getPeopleTime(eGetDataMode_t mode = eGetDataActive);

  /**
   * @fn setRealTimePeopleTime
   * @brief Set people count report interval.
   * @param time: Report interval, in seconds.
   * @return true: Set succeeded, false: Set failed.
  */
  bool setRealTimePeopleTime(uint32_t time);

  /**
   * @fn getRealTimePeopleTime
   * @brief Get people count report interval.
   * @param time: Pointer to receive report interval, in seconds.
   * @return true: Get succeeded, false: Get failed.
  */
  bool getRealTimePeopleTime(uint32_t *time);

  /**
   * @fn clearPeopleCount
   * @brief Clear people count statistics.
   * @return true: Clear succeeded, false: Clear failed.
  */
  bool clearPeopleCount(void);

  /**
   * @fn setTrackMeters
   * @brief Set trajectory generation distance threshold.
   * @param distanceCm: Distance threshold, in cm.
   * @return true: Set succeeded, false: Set failed.
  */
  bool setTrackMeters(uint32_t distanceCm);

  /**
   * @fn getTrackMeters
   * @brief Get trajectory generation distance threshold.
   * @param distanceCm: Pointer to receive distance threshold, in cm.
   * @return true: Get succeeded, false: Get failed.
  */
  bool getTrackMeters(uint32_t *distanceCm);

  /**
   * @fn setTrackExistsTime
   * @brief Set trajectory hold time.
   * @param time: Hold time, in seconds.
   * @return true: Set succeeded, false: Set failed.
  */
  bool setTrackExistsTime(uint32_t time);

  /**
   * @fn getTrackExistsTime
   * @brief Get trajectory hold time.
   * @param time: Pointer to receive hold time, in seconds.
   * @return true: Get succeeded, false: Get failed.
  */
  bool getTrackExistsTime(uint32_t *time);

  /**
   * @fn setUnmannedTime
   * @brief Set no-person delay time.
   * @param delayTime: Delay time, in seconds.
   * @return true: Set succeeded, false: Set failed.
  */
  bool setUnmannedTime(uint32_t delayTime);

  /**
   * @fn getUnmannedTime
   * @brief Get no-person delay time.
   * @param delayTime: Pointer to receive delay time, in seconds.
   * @return true: Get succeeded, false: Get failed.
  */
  bool getUnmannedTime(uint32_t *delayTime);

protected:
  void setTimeout(uint16_t timeoutMs);
  bool isInitFinished(void);
  bool sendCommand(uint8_t control, uint8_t cmd, const uint8_t *data, uint16_t len);
  bool requestFrame(uint8_t control, uint8_t cmd, const uint8_t *data, uint16_t len, sPacket_t *response, uint16_t timeoutMs = 200);
  bool readFrame(sPacket_t *packet, uint16_t timeoutMs);
  bool readByte(uint8_t *value, uint16_t timeoutMs);
  void flushInput(void);
  eReportedEvent_t handlePacket(const sPacket_t *packet);
  eReportedEvent_t classifyPacket(const sPacket_t *packet);

  bool queryByte(uint8_t control, uint8_t cmd, uint8_t *value);
  bool setByte(uint8_t control, uint8_t cmd, uint8_t value);
  bool queryUint32(uint8_t control, uint8_t cmd, uint32_t *value);
  bool setUint32(uint8_t control, uint8_t cmd, uint32_t value);
  String queryString(uint8_t control, uint8_t cmd);

  void parseTargets(const uint8_t *data, uint16_t len);
  void parseTagList(const uint8_t *data, uint16_t len);
  void parseTagEvent(const uint8_t *data, uint16_t len);
  void parseBoundaryRange(const uint8_t *data, uint16_t len);
  void parsePeopleCount(const uint8_t *data, uint16_t len);

  uint16_t readUint16(const uint8_t *data) const;
  int16_t readInt16(const uint8_t *data) const;
  int16_t readSignBitInt16(const uint8_t *data) const;
  uint32_t readUint32(const uint8_t *data) const;
  void writeUint16(uint8_t *data, uint16_t value) const;
  void writeInt16(uint8_t *data, int16_t value) const;
  void writeSignBitInt16(uint8_t *data, int16_t value) const;
  void writeUint32(uint8_t *data, uint32_t value) const;
  void initObject(void);

private:
  Stream *_s;
#if defined(ESP8266) || defined(ARDUINO_AVR_UNO)
  SoftwareSerial *_serial;
#else
  HardwareSerial *_serial;
#endif
  uint32_t _baud;
  uint8_t _rxpin;
  uint8_t _txpin;
  uint16_t _timeoutMs;
  uint32_t _lastHeartbeatMs;
  bool _heartbeat;
  bool _initFinished;
  uint8_t _presenceEnable;
  ePresenceState_t _presence;
  eMotionState_t _motionState;
  uint8_t _trajectoryLed;
  uint8_t _motionLed;
  sTargetInfo_t _targets[MAX_TARGETS];
  uint8_t _targetCount;
  sTagConfig_t _tags[MAX_TAGS];
  uint8_t _tagCount;
  sTagInfo_t _tagInfo;
  bool _tagInfoValid;
  sFourSidedRange _rangeInfo;
  uint8_t _peopleCount;
};

#endif
