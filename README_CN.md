# DFRobot_C4004
- [English](./README.md)

DFRobot C4004 是一款 60GHz 4T4R 多区域存在感知毫米波雷达，适用于智能空间管理。它不仅能够上报人员占用状态，还能实时统计配置区域内的静止人数与运动人数，内置区域逻辑与 6 路 IO 联动，可快速实现自动化部署。

## 产品链接(www.dfrobot.com)

    SKU:SEN0753

## 目录

* [概述](#概述)
* [安装](#安装)
* [方法](#方法)
* [兼容性](#兼容性)
* [历史](#历史)
* [致谢](#致谢)

## 概述

* 支持在 Arduino UNO、FireBeetle ESP32 和树莓派上进行 UART 通信。
* 支持系统心跳、初始化状态查询、复位和恢复出厂设置。
* 支持产品型号、产品 ID、硬件版本和固件版本查询。
* 支持安装模式、安装高度和 Z 轴角度配置。
* 支持人体存在检测和运动状态查询/上报。
* 支持目标轨迹跟踪和目标信息上报。
* 支持标签区域配置、标签清除、批量标签配置和标签事件上报。
* 支持四边检测边界和轨迹检测范围模式设置。
* 支持人数统计查询/上报、上报间隔、轨迹距离、保持时间和无人延迟设置。

## 安装

本库有两种使用方式：

1. 打开 Arduino IDE，在 工具 -> 管理库 中搜索 `DFRobot_C4004` 并安装该库。
2. 下载本库，将其复制到 Arduino `libraries` 文件夹中，然后打开 examples 文件夹运行示例。

对于树莓派，使用 `python/raspberrypi/` 中的驱动，若未安装 `pyserial` 请先安装。

```bash
pip3 install pyserial
```

## 方法

```C++

  /**
   * @fn begin
   * @brief 初始化 DFRobot C4004 传感器。
   * @return true: 初始化成功，false: 初始化失败。
   */
  bool begin(void);

  /**
   * @fn isConnected
   * @brief 检测 DFRobot C4004 传感器是否连接。
   * @return true: 已连接，false: 未连接。
   */
  bool isConnected(void);

  /**
   * @fn reset
   * @brief 复位 DFRobot C4004 传感器。
   * @return true: 复位成功，false: 复位失败。
   */
  bool reset(void);

  /**
   * @fn factoryReset
   * @brief 恢复 DFRobot C4004 传感器出厂设置。
   * @return true: 复位成功，false: 复位失败。
   */
  bool factoryReset(void);

  /**
   * @fn getHeartbeat
   * @brief 获取 DFRobot C4004 传感器的心跳状态。
   * @param mode: 数据获取模式。
   * @n          eGetDataActive: 主动获取最新心跳状态。
   * @n          eGetDataReport: 从最近一次上报中获取最新心跳状态。
   * @return true: 检测到心跳，false: 未检测到心跳。
   */
  bool getHeartbeat(eGetDataMode_t mode = eGetDataActive);

  /**
   * @fn getReportedInfo
   * @brief 获取 DFRobot C4004 传感器最新上报的事件。
   * @param timeoutMs: 等待上报的最大时间，单位为毫秒，默认值为 50。
   * @return eReportedEvent_t: 上报的事件类型。
   * @n          eEventNone: 未检测到事件。
   * @n          eEventTrajectory: 检测到轨迹跟踪事件。
   * @n          eEventPresence: 检测到存在检测事件。
   * @n          eEventMotion: 检测到人体运动事件。
   * @n          eEventTag: 检测到标签事件。
   * @n          eEventHeartbeat: 检测到心跳事件。
   * @n          eEventInitFinished: 检测到初始化完成事件。
   * @n          eEventPeopleCount: 检测到人数统计事件。
   * @n          eEventUnknown: 检测到未知事件。
   * @n          eEventError: 发生错误。
   */
  eReportedEvent_t getReportedInfo(uint16_t timeoutMs = 50);

  /**
   * @fn getHardwareVersion
   * @brief 获取 DFRobot C4004 传感器的硬件版本。
   * @return String: 硬件版本。
   */
  String getHardwareVersion(void);

  /**
   * @fn getFirmwareVersion
   * @brief 获取 DFRobot C4004 传感器的固件版本。
   * @return String: 固件版本。
   */
  String getFirmwareVersion(void);

  /**
   * @fn setInstallInfo
   * @brief 设置 DFRobot C4004 传感器的安装信息。
   * @param info: 安装信息。
   * @n          mode: DFRobot C4004 安装模式。
   * @n          heightCm: DFRobot C4004 安装高度，单位为 cm。
   * @n          zAngle: DFRobot C4004 安装 Z 轴角度，单位为度。
   * @return true: 设置成功，false: 设置失败。
   */
  bool setInstallInfo(sInstallInfo_t &info);

  /**
   * @fn getInstallInfo
   * @brief 获取 DFRobot C4004 传感器的安装信息。
   * @param info: 安装信息。
   * @n          mode: DFRobot C4004 安装模式。
   * @n          heightCm: DFRobot C4004 安装高度，单位为 cm。
   * @n          zAngle: DFRobot C4004 安装 Z 轴角度，单位为度。
   * @return true: 获取成功，false: 获取失败。
  */
  bool getInstallInfo(sInstallInfo_t *info);

  /**
   * @fn setInstallHigh
   * @brief 设置 DFRobot C4004 传感器的安装高度。
   * @param hight: 安装高度，单位为 cm。
   * @return true: 设置成功，false: 设置失败。
  */
  bool setInstallHigh(int32_t hight);

  /**
   * @fn getInstallHigh
   * @brief 获取 DFRobot C4004 传感器的安装高度。
   * @param hight: 接收安装高度的指针，单位为 cm。
   * @return true: 获取成功，false: 获取失败。
  */
  bool getInstallHigh(int *hight);

  /**
   * @fn setPresenceEnable
   * @brief 启用或禁用传感器的存在检测功能。
   * @param enable: 启用或禁用存在检测功能。
   * @n          true: 启用，false: 禁用。
   * @return true: 设置成功，false: 设置失败。
   */
  bool setPresenceEnable(bool enable);

  /**
   * @fn getPresenceEnable
   * @brief 获取存在检测功能是否已启用。
   * @param enable: 接收启用状态的指针。
   * @n          true: 已启用，false: 已禁用。
   * @return true: 获取成功，false: 获取失败。
   */
  bool getPresenceEnable(bool *enable);

  /**
   * @fn getPresenceState
   * @brief 获取当前的存在检测结果。
   * @param mode: 数据获取方式。
   * @n          eGetDataActive: 主动查询最新数据并更新缓存。
   * @n          eGetDataReport: 直接返回最近一次上报缓存的数据。
   * @return ePresenceState_t: 存在检测结果。
   * @n          eNoPresence: 未检测到存在。
   * @n          ePresence: 检测到存在。
   * @n          ePresenceUnknown: 存在状态未知。
   */
  ePresenceState_t getPresenceState(eGetDataMode_t mode = eGetDataActive);

  /**
   * @fn getMotionState
   * @brief 获取当前的人体运动状态。
   * @param mode: 数据获取方式。
   * @n          eGetDataActive: 主动查询最新数据并更新缓存。
   * @n          eGetDataReport: 直接返回最近一次上报缓存的数据。
   * @return eMotionState_t: 运动状态。
   * @n          eMotionNone: 无运动状态。
   * @n          eMotionStatic: 静止。
   * @n          eMotionActive: 活跃运动。
   * @n          eMotionUnknown: 运动状态未知。
   */
  eMotionState_t getMotionState(eGetDataMode_t mode = eGetDataActive);

  /**
   * @fn setTrajectoryTrackEnable
   * @brief 启用或禁用传感器的轨迹跟踪功能。
   * @param enable: 启用或禁用轨迹跟踪功能。
   * @n          true: 启用，false: 禁用。
   * @return true: 设置成功，false: 设置失败。
   */
  bool setTrajectoryTrackEnable(bool enable);

  /**
   * @fn getTrajectoryTrackEnable
   * @brief 查询轨迹跟踪功能是否已启用。
   * @param enable: 接收启用状态的指针。
   * @n          true: 已启用，false: 已禁用。
   * @return true: 查询成功，false: 查询失败。
  */
  bool getTrajectoryTrackEnable(bool *enable);

  /**
   * @fn setCheckToActiveFrames
   * @brief 设置检查状态切换到活跃状态的确认帧数。
   * @param frames: 帧数，有效范围：1-7。
   * @return true: 设置成功，false: 设置失败。
   */
  bool setCheckToActiveFrames(uint8_t frames);

  /**
   * @fn getCheckToActiveFrames
   * @brief 查询检查状态切换到活跃状态的确认帧数。
   * @param frames: 接收帧数的指针。
   * @return true: 查询成功，false: 查询失败。
   */
  bool getCheckToActiveFrames(uint8_t *frames);

  /**
   * @fn getTargetList
   * @brief 获取 DFRobot C4004 传感器的目标信息列表。
   * @param targetBuf: 接收目标信息列表的指针。
   * @param maxCount: 可读取的最大目标数量。
   * @param mode: 数据获取模式。
   * @n          eGetDataActive: 读取前主动查询最新目标信息。
   * @n          eGetDataReport: 从缓存的上报数据中读取目标信息。
   * @return uint8_t: 读取到的目标数量。
   */
  uint8_t getTargetList(sTargetInfo_t *targetBuf, uint8_t maxCount, eGetDataMode_t mode = eGetDataActive);

  /**
   * @fn setTrajectoryLed
   * @brief 启用或禁用 DFRobot C4004 传感器在轨迹跟踪时的 LED。
   * @param enable: 启用或禁用标签检测功能。
   */
  bool setTrajectoryLed(bool enable);

  /**
   * @fn setMotionLed
   * @brief 启用或禁用 DFRobot C4004 传感器在人体运动检测时的 LED。
   * @param enable: 启用或禁用标签检测功能。
   * @return true: 设置成功，false: 设置失败。
   */
  bool setMotionLed(bool enable);

  /**
   * @fn getTrajectoryLed
   * @brief 获取 DFRobot C4004 传感器在轨迹跟踪时的 LED 状态。
   * @return true: LED 已启用，false: LED 已禁用。
   */
  bool getTrajectoryLed(void);

  /**
   * @fn getMotionLed
   * @brief 获取 DFRobot C4004 传感器在人体运动检测时的 LED 状态。
   * @return true: LED 已启用，false: LED 已禁用。
   */
  bool getMotionLed(void);

  /**
   * getTags
   * @brief 获取所有标签配置信息。
   * @param tags: 接收标签配置的指针。
   * @param maxTags: 写入 tags 缓冲区的最大标签数量。
   * @param mode: 数据获取模式，保留用于兼容。
   * @n          eGetDataActive: 从设备主动查询最新标签配置。
   * @n          eGetDataReport: 当前行为与 eGetDataActive 相同。
   * @n          每个标签中的 ioIndex: IO 联动索引，0 表示不使用，2-6 表示绑定 IO2-IO6。
   * @return uint8_t: 设备实际返回的标签数量。
   */
  uint8_t getTags(sTagConfig_t *tags, uint8_t maxTags, eGetDataMode_t mode = eGetDataActive);

  /**
   * @fn setTag
   * @brief 使用尺寸模式设置一个标签。
   * @param tag: 标签配置。
   * @n          tagIndex: 标签索引。
   * @n          tagType: 标签类型。
   * @n          scopeType: 标签范围类型。
   * @n          ioIndex: IO 联动索引，0 表示不使用，2-6 表示绑定 IO2-IO6。
   * @n          width: 标签宽度或圆形半径，单位为 cm。
   * @n          height: 标签高度，单位为 cm。
   * @return eTagSetStatus_t: 标签设置状态。
   * @n          eTagSetCommError: 通信失败或响应不匹配。
   * @n          eTagSetSuccess: 标签设置成功。
   * @n          eTagSetTrackCountError: 轨迹数量不等于 1。
   * @n          eTagSetAlreadyUsed: 标签已被占用。
   * @n          eTagSetIndexOutOfRange: 标签索引超出范围。
   * @note 此 API 忽略 sTagConfig_t 中的 centerX/centerY 字段。
   * @note 使用此 API 设置标签时，需确保轨迹数量为 1。
   * @note 最多设置 32 个标签。
   */
  eTagSetStatus_t setTag(const sTagConfig_t &tag);

  /**
   * @fn clearTag
   * @brief 清除标签配置。
   * @param tagIndex: 标签索引（协议载荷中的 2 字节索引）。
   * @return true: 清除成功，false: 清除失败。
   */
  bool clearTag(uint16_t tagIndex);

  /**
   * @fn clearAllTags
   * @brief 清除所有标签配置。
   * @return true: 清除成功，false: 清除失败。
   */
  bool clearAllTags(void);

  /**
   * @fn setTagsFromConfig
   * @brief 使用坐标模式从列表中批量设置标签配置。
   * @param tags: 标签配置列表的指针。
   * @param tagCount: 列表中的标签数量。
   * @n          每个标签中的 ioIndex: IO 联动索引，0 表示不使用，2-6 表示绑定 IO2-IO6。
   * @return true: 设置成功，false: 设置失败。
   * @note 可以以坐标形式设置标签，无需满足轨迹数量为 1 的要求。
   * @note 最多设置 32 个标签。
   */
  bool setTagsFromConfig(const sTagConfig_t *tags, uint8_t tagCount);

  /**
   * @fn getTagInfo
   * @brief 获取从主动上报包（CTRL 0x07, CMD 0x1B）解码的最新标签事件。
   * @param tagInfo: 接收上报标签事件信息的指针。
   * @return true: 获取成功，false: 无有效上报标签事件或参数无效。
   * @note 标签事件上报包含 ioIndex。
   * @note 此 API 仅读取上报缓存。请先调用 getReportedInfo() 接收新的上报数据。
   */
  bool getTagInfo(sTagInfo_t *tagInfo);

  /**
   * @fn setFourSidedRangeMode
   * @brief 设置四边边界检测范围。
   * @param range: 边界范围设置。
   * @n          xPositiveCm: X 轴正方向边界，单位为 cm。
   * @n          xNegativeCm: X 轴负方向边界，单位为 cm。
   * @n          yPositiveCm: Y 轴正方向边界，单位为 cm。
   * @n          yNegativeCm: Y 轴负方向边界，单位为 cm。
   * @return true: 设置成功，false: 设置失败。
   */
  bool setFourSidedRangeMode(sFourSidedRange_t &range);

  /**
   * @fn getFourSidedRangeMode
   * @brief 查询并获取四边边界检测范围。
   * @param range: 接收边界范围设置的指针。
   * @return true: 获取成功，false: 获取失败。
   */
  bool getFourSidedRangeMode(sFourSidedRange_t *range);

  /**
   * @fn setTrajectoryRangeMode
   * @brief 开始轨迹范围学习，或使用已学习的轨迹范围模式。
   * @param learning: 轨迹范围学习开关。
   * @n          true: 开始学习轨迹范围，false: 使用轨迹范围模式但不学习。
   */
  void setTrajectoryRangeMode(bool learning);

  /**
   * @fn getTrajectoryRangeMode
   * @brief 查询并获取轨迹模式（模式 0x05）下的范围点。
   * @param points: 接收轨迹模式点的指针。
   * @param pointCount: 接收点数量的指针。
   * @return true: 查询成功，false: 查询失败。
   * @note points 缓冲区必须能够容纳至少 MAX_POINTS 个点。
   */
  bool getTrajectoryRangeMode(sPoint_t *points, uint16_t *pointCount);

  /**
   * @fn setConfigFileModePoints
   * @brief 使用配置文件模式（模式 0x06）设置检测范围点。
   * @param points: 配置文件模式点的指针。
   * @param pointCount: 点的数量。
   * @return true: 设置成功，false: 设置失败。
   * @note 点值使用符号位 int16 编码（bit15: 0=正数，1=负数）。
   * @note pointCount 上限为 MAX_POINTS。
   */
  bool setConfigFileModePoints(const sPoint_t *points, uint16_t pointCount);

  /**
   * @fn getConfigFileModePoints
   * @brief 查询并获取配置文件模式（模式 0x06）下的范围点。
   * @param points: 接收配置文件模式点的指针。
   * @param pointCount: 接收点数量的指针。
   * @return true: 查询成功，false: 查询失败。
   * @note points 缓冲区必须能够容纳至少 MAX_POINTS 个点。
   */
  bool getConfigFileModePoints(sPoint_t *points, uint16_t *pointCount);

  /**
   * @fn getDetectionRangeMode
   * @brief 查询当前检测范围模式。
   * @return eDetectionRangeMode_t: 当前检测范围模式。
   */
  eDetectionRangeMode_t getDetectionRangeMode(void);

  /**
   * @fn getPeopleTime
   * @brief 获取人数统计。
   * @param mode: 数据获取模式。
   * @n          eGetDataActive: 主动查询最新数据并更新缓存。
   * @n          eGetDataReport: 直接返回缓存数据。
   * @return uint8_t: 人数统计（模块上报的最大计数值）。
   */
  uint8_t getPeopleTime(eGetDataMode_t mode = eGetDataActive);

  /**
   * @fn setRealTimePeopleTime
   * @brief 设置人数统计上报间隔。
   * @param time: 上报间隔，单位为秒。
   * @return true: 设置成功，false: 设置失败。
   */
  bool setRealTimePeopleTime(uint32_t time);

  /**
   * @fn getRealTimePeopleTime
   * @brief 获取人数统计上报间隔。
   * @param time: 接收上报间隔的指针，单位为秒。
   * @return true: 获取成功，false: 获取失败。
   */
  bool getRealTimePeopleTime(uint32_t *time);

  /**
   * @fn clearPeopleCount
   * @brief 清除人数统计数据。
   * @return true: 清除成功，false: 清除失败。
   */
  bool clearPeopleCount(void);

  /**
   * @fn setTrackMeters
   * @brief 设置轨迹生成距离阈值。
   * @param distanceCm: 距离阈值，单位为 cm。
   * @return true: 设置成功，false: 设置失败。
   */
  bool setTrackMeters(uint32_t distanceCm);

  /**
   * @fn getTrackMeters
   * @brief 获取轨迹生成距离阈值。
   * @param distanceCm: 接收距离阈值的指针，单位为 cm。
   * @return true: 获取成功，false: 获取失败。
   */
  bool getTrackMeters(uint32_t *distanceCm);

  /**
   * @fn setTrackExistsTime
   * @brief 设置轨迹保持时间。
   * @param time: 保持时间，单位为秒。
   * @return true: 设置成功，false: 设置失败。
   */
  bool setTrackExistsTime(uint32_t time);

  /**
   * @fn getTrackExistsTime
   * @brief 获取轨迹保持时间。
   * @param time: 接收保持时间的指针，单位为秒。
   * @return true: 获取成功，false: 获取失败。
   */
  bool getTrackExistsTime(uint32_t *time);

  /**
   * @fn setUnmannedTime
   * @brief 设置无人延迟时间。
   * @param delayTime: 延迟时间，单位为秒。
   * @return true: 设置成功，false: 设置失败。
   */
  bool setUnmannedTime(uint32_t delayTime);

  /**
   * @fn getUnmannedTime
   * @brief 获取无人延迟时间。
   * @param delayTime: 接收延迟时间的指针，单位为秒。
   * @return true: 获取成功，false: 获取失败。
   */
  bool getUnmannedTime(uint32_t *delayTime);
```

## 兼容性

MCU                | 正常工作    | 工作异常    | 未测试     | 备注
------------------ | :----------: | :----------: | :---------: | :----:
Arduino Uno        |      √       |              |             |
Arduino MEGA2560   |      √       |              |             |
Arduino Leonardo   |      √       |              |             |
FireBeetle-ESP32   |      √       |              |             |
Micro:bit          |              |              |      √      |

## 历史

- 2026/05/22 - V1.0.0 版本

## 致谢

由 JiaLi(zhixin.liu@dfrobot.com) 编写，2026 年。（欢迎访问我们的[网站](https://www.dfrobot.com/)）
