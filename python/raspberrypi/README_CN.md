# DFRobot_C4004 Raspberry Pi 库
- [English Version](./README.md)

该目录包含 DFRobot C4004 的 Raspberry Pi Python 驱动。该雷达为 60GHz 4T4R 多区域存在检测毫米波方案，支持区域内静止/运动人数实时输出及 6 路 IO 联动控制。

## 安装

```bash
pip3 install pyserial
```

如果使用 `read_zone_state_by_gpio.py`，请根据需要启用 Raspberry Pi GPIO 并安装 `RPi.GPIO`。

## 接线

DFRobot C4004 引脚 | Raspberry Pi
---------- | ------------
VCC        | 5V
GND        | GND
TX         | RXD
RX         | TXD

示例默认串口为 `/dev/ttyAMA0`，波特率 `115200`。

## 方法

```python
  def begin(self):
    '''!
      @brief 初始化模块并验证通信。
      @return True or False
    '''

  def close(self):
    '''!
      @brief 关闭串口。
    '''

  def is_init_finished(self):
    '''!
      @brief 查询模块是否初始化完成。
      @return True or False
    '''

  def is_connected(self):
    '''!
      @brief 检查模块是否在线。
      @return True or False
    '''

  def reset(self):
    '''!
      @brief 重启模块。
      @return True or False
    '''

  def factory_reset(self):
    '''!
      @brief 恢复出厂设置。
      @return True or False
    '''

  def get_heartbeat(self, mode=GET_DATA_ACTIVE):
    '''!
      @brief 获取心跳状态。
      @param mode: 数据模式
      @n   GET_DATA_ACTIVE: 主动查询
      @n   GET_DATA_REPORT: 读取上报缓存
      @return True or False
    '''

  def get_reported_info(self, timeout=0.05):
    '''!
      @brief 读取并解析一帧上报事件。
      @param timeout: 超时时间（秒）
      @return 事件类型
    '''

  def get_hardware_version(self):
    '''!
      @brief 获取硬件版本。
      @return 字符串
    '''

  def get_firmware_version(self):
    '''!
      @brief 获取固件版本。
      @return 字符串
    '''

  def set_install_info(self, info):
    '''!
      @brief 设置安装信息。
      @param info: InstallInfo 对象
      @n   mode: INSTALL_MODE_SIDE / INSTALL_MODE_TOP
      @n   height_cm: 安装高度(cm)
      @n   x_angle/y_angle/z_angle: 安装角度(度)
      @return True or False
    '''

  def get_install_info(self, info):
    '''!
      @brief 读取安装信息。
      @param info: 安装信息输出对象
      @return True or False
    '''

  def set_install_high(self, hight):
    '''!
      @brief 设置安装高度。
      @param hight: 安装高度(cm)
      @return True or False
    '''

  def get_install_high(self):
    '''!
      @brief 读取安装高度。
      @return 安装高度(cm)，失败时返回 0。
    '''

  def set_presence_enable(self, enable):
    '''!
      @brief 开启或关闭存在检测。
      @param enable: True/False
      @return True or False
    '''

  def get_presence_enable(self, enable):
    '''!
      @brief 读取存在检测开关状态。
      @param enable: 输出容器(list/dict/object.value)
      @return True or False
    '''

  def get_presence_state(self, mode=GET_DATA_ACTIVE):
    '''!
      @brief 读取存在状态。
      @param mode: 数据获取方式。
      @n   GET_DATA_ACTIVE: 主动查询最新数据并更新缓存。
      @n   GET_DATA_REPORT: 直接返回最近一次上报缓存的数据。
      @return NO_PRESENCE / PRESENCE / PRESENCE_UNKNOWN
    '''

  def get_motion_state(self, mode=GET_DATA_ACTIVE):
    '''!
      @brief 读取运动状态。
      @param mode: 数据获取方式。
      @n   GET_DATA_ACTIVE: 主动查询最新数据并更新缓存。
      @n   GET_DATA_REPORT: 直接返回最近一次上报缓存的数据。
      @return MOTION_NONE / MOTION_STATIC / MOTION_ACTIVE / MOTION_UNKNOWN
    '''

  def set_trajectory_track_enable(self, enable):
    '''!
      @brief 开启或关闭轨迹跟踪功能。
      @param enable: True/False
      @return True or False
    '''

  def get_trajectory_track_enable(self, enable):
    '''!
      @brief 读取轨迹跟踪开关状态。
      @param enable: 输出容器(list/dict/object.value)
      @return True or False
    '''

  def set_check_to_active_frames(self, frames):
    '''!
      @brief 设置 check 状态切换到 active 状态的确认帧数。
      @param frames: 帧数，有效范围 1-7
      @return True or False
    '''

  def get_check_to_active_frames(self, frames):
    '''!
      @brief 读取 check 状态切换到 active 状态的确认帧数。
      @param frames: 输出容器(list/dict/object.value)
      @return True or False
    '''

  def get_target_list(self, mode=GET_DATA_ACTIVE):
    '''!
      @brief 获取目标列表。
      @param mode: 数据模式
      @n   GET_DATA_ACTIVE: 主动查询
      @n   GET_DATA_REPORT: 读取上报缓存
      @return List[TargetInfo]
    '''

  def set_trajectory_led(self, enable):
    '''!
      @brief 设置轨迹指示灯开关。
      @param enable: True/False
      @return True or False
    '''

  def set_motion_led(self, enable):
    '''!
      @brief 设置运动指示灯开关。
      @param enable: True/False
      @return True or False
    '''

  def get_trajectory_led(self):
    '''!
      @brief 获取轨迹指示灯状态。
      @return True or False
    '''

  def get_motion_led(self):
    '''!
      @brief 获取运动指示灯状态。
      @return True or False
    '''

  def get_tags(self, mode=GET_DATA_ACTIVE, max_tags=None):
    '''!
      @brief 从设备获取全部标签配置。
      @param mode: 数据模式，保留用于兼容
      @n   GET_DATA_ACTIVE: 从设备主动查询
      @n   GET_DATA_REPORT: 当前行为与 GET_DATA_ACTIVE 相同
      @param max_tags: 返回的最大标签数量。None 表示返回全部已解析标签。
      @return List[TagConfig]
    '''

  def set_tag(self, tag):
    '''!
      @brief 单标签设置（尺寸模式）。
      @param tag: TagConfig 对象
      @n   tag.io_index: IO 联动索引，0 表示不使用，2-6 表示绑定 IO2-IO6。
      @n   tag.width: 标签宽度或圆形半径，单位为 cm
      @n   tag.height: 标签高度，单位为 cm
      @return 标签设置状态码
      @n   TAG_SET_COMM_ERROR
      @n   TAG_SET_SUCCESS
      @n   TAG_SET_TRACK_COUNT_ERROR
      @n   TAG_SET_ALREADY_USED
      @n   TAG_SET_INDEX_OUT_OF_RANGE
      @note 此 API 忽略 center_x/center_y 字段。
      @note 使用此 API 设置标签时，需确保轨迹数量为 1。
      @note 最多设置 32 个标签。
    '''

  def clear_tag(self, tag_index):
    '''!
      @brief 清除单个标签。
      @param tag_index: 标签索引（协议载荷中的 2 字节索引）
      @return True or False
    '''

  def clear_all_tags(self):
    '''!
      @brief 清除全部标签。
      @return True or False
    '''

  def set_tags_from_config(self, tags):
    '''!
      @brief 坐标模式批量设置标签。
      @param tags: TagConfig 可迭代对象
      @n   每个 tag.io_index: IO 联动索引，0 表示不使用，2-6 表示绑定 IO2-IO6。
      @return True or False
      @note 坐标模式设置标签时，无需满足轨迹数量为 1 的要求。
      @note 最多设置 32 个标签。
    '''

  def get_tag_info(self):
    '''!
      @brief 获取从主动上报包（CTRL 0x07, CMD 0x1B）解码的最新标签事件。
      @return TagInfo 对象；若无有效事件则返回 None。
      @note 此 API 仅读取上报缓存。请先调用 get_reported_info() 接收新的上报数据。
      @note 标签事件上报包含 info.io_index。
    '''

  def set_four_sided_range_mode(self, range_info):
    '''!
      @brief 设置四边探测范围（模式 0x04）。
      @param range_info: FourSidedRange_t 对象
      @return True or False
      @note 位置值使用符号位 int16 编码（bit15: 0=正数，1=负数）。
    '''

  def get_four_sided_range_mode(self, range_info):
    '''!
      @brief 读取四边探测范围。
      @param range_info: FourSidedRange_t 输出对象
      @return True or False
    '''

  def set_trajectory_range_mode(self, learning):
    '''!
      @brief 开始轨迹范围学习，或使用已学习的轨迹范围模式（模式 0x05）。
      @param learning: True 表示开始学习；False 表示使用轨迹范围模式但不学习。
      @return True or False
    '''

  def set_config_file_mode_points(self, points):
    '''!
      @brief 设置配置文件模式多点范围（模式 0x06）。
      @param points: Point 可迭代对象
      @return True or False
      @n   数据格式: 0x06 + 2B 点数 + n*(2B X + 2B Y)
      @note 点值使用符号位 int16 编码（bit15: 0=正数，1=负数）。
      @note 点数上限为 MAX_POINTS（150）。
    '''

  def get_trajectory_range_mode(self, points, point_count):
    '''!
      @brief 读取轨迹模式点位（模式 0x05）。
      @param points: 点位输出 list
      @param point_count: 点数输出容器(list/dict/object.value)
      @return True or False
      @note points 列表必须能够容纳至少 MAX_POINTS 个点。
    '''

  def get_config_file_mode_points(self, points, point_count):
    '''!
      @brief 读取配置文件模式点位（模式 0x06）。
      @param points: 点位输出 list
      @param point_count: 点数输出容器(list/dict/object.value)
      @return True or False
      @note points 列表必须能够容纳至少 MAX_POINTS 个点。
    '''

  def get_detection_range_mode(self):
    '''!
      @brief 查询当前探测范围模式。
      @return 模式值
    '''

  def get_people_time(self, mode=GET_DATA_ACTIVE):
    '''!
      @brief 获取人数统计值。
      @param mode: 数据模式
      @n   GET_DATA_ACTIVE: 主动查询最新数据并更新缓存
      @n   GET_DATA_REPORT: 直接返回缓存数据
      @return 人数值（模块上报的最大计数值）
    '''

  def set_real_time_people_time(self, interval):
    '''!
      @brief 设置人数上报周期。
      @param interval: 秒
      @return True or False
    '''

  def get_real_time_people_time(self):
    '''!
      @brief 获取人数上报周期。
      @return 秒
    '''

  def clear_people_count(self):
    '''!
      @brief 清除人数统计。
      @return True or False
    '''

  def set_track_meters(self, distance_cm):
    '''!
      @brief 设置轨迹生成距离阈值。
      @param distance_cm: 阈值(cm)
      @return True or False
    '''

  def get_track_meters(self):
    '''!
      @brief 获取轨迹生成距离阈值。
      @return 阈值(cm)
    '''

  def set_track_exists_time(self, time):
    '''!
      @brief 设置轨迹保持时间。
      @param time: 秒
      @return True or False
    '''

  def get_track_exists_time(self):
    '''!
      @brief 获取轨迹保持时间。
      @return 秒
    '''

  def set_unmanned_time(self, delay_time):
    '''!
      @brief 设置无人延迟时间。
      @param delay_time: 秒
      @return True or False
    '''

  def get_unmanned_time(self):
    '''!
      @brief 获取无人延迟时间。
      @return 秒
    '''
```

## 历史

- 2026/05/22 - V1.0.0 版本

## 创作者

Written by JiaLi(zhixin.liu@dfrobot.com), 2026. (Welcome to our [website](https://www.dfrobot.com/))
