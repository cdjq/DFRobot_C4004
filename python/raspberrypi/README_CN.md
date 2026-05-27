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

  def get_product_model(self):
    '''!
      @brief 获取产品型号。
      @return 字符串
    '''

  def get_product_id(self):
    '''!
      @brief 获取产品 ID。
      @return 整数 ID
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

  def set_install_height(self, height_cm):
    '''! @brief set_install_high 的拼写修正别名。 '''

  def get_install_height(self):
    '''! @brief get_install_high 的拼写修正别名。 '''

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

  def get_presence_state(self):
    '''!
      @brief 读取存在状态。
      @return NO_PRESENCE / PRESENCE / PRESENCE_UNKNOWN
    '''

  def get_motion_state(self):
    '''!
      @brief 读取运动状态。
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

  def get_target_list(self, mode=GET_DATA_ACTIVE):
    '''!
      @brief 获取目标列表。
      @param mode: 数据模式
      @n   GET_DATA_ACTIVE: 主动查询
      @n   GET_DATA_REPORT: 读取上报缓存
      @return List[TargetInfo]
    '''

  def get_target_info(self, index=0, mode=GET_DATA_ACTIVE):
    '''!
      @brief 获取单个目标信息。
      @param index: 目标索引
      @param mode: 数据模式
      @return TargetInfo or None
    '''

  def get_target_count(self):
    '''!
      @brief 获取缓存目标数量。
      @return 整数
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

  def get_tags(self, mode=GET_DATA_ACTIVE):
    '''!
      @brief 获取标签配置缓存列表。
      @param mode: 数据模式
      @n   GET_DATA_ACTIVE: 先主动查询再读缓存
      @n   GET_DATA_REPORT: 仅读取缓存
      @return List[TagConfig]
    '''

  def set_tag(self, tag):
    '''!
      @brief 单标签设置（尺寸模式）。
      @param tag: TagConfig 对象
      @return 标签设置状态码
      @n   TAG_SET_COMM_ERROR
      @n   TAG_SET_SUCCESS
      @n   TAG_SET_TRACK_COUNT_ERROR
      @n   TAG_SET_ALREADY_USED
      @n   TAG_SET_INDEX_OUT_OF_RANGE
    '''

  def clear_tag(self, tag_index):
    '''!
      @brief 清除单个标签。
      @param tag_index: 标签索引
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
      @return True or False
    '''

  def get_tag_info(self):
    '''!
      @brief 获取最近一次标签上报事件（缓存）。
      @return TagInfo 对象；若无有效事件则返回 None。
    '''

  def set_boundary_detection_range(self, range_info):
    '''!
      @brief 设置四边探测范围（模式 0x04）。
      @param range_info: BoundaryDetectionRange 对象
      @return True or False
    '''

  def get_boundary_detection_range(self, range_info):
    '''!
      @brief 读取四边探测范围。
      @param range_info: 范围输出对象
      @return True or False
    '''

  def set_trajectory_detection_range(self, enable):
    '''!
      @brief 开启或关闭轨迹探测范围模式（模式 0x05）。
      @param enable: True/False
      @return True or False
    '''

  def set_config_file_mode_points(self, points):
    '''!
      @brief 设置配置文件模式多点范围（模式 0x06）。
      @param points: Point 可迭代对象
      @return True or False
      @n   数据格式: 0x06 + 2B 点数 + n*(2B X + 2B Y)
    '''

  def get_trajectory_detection_range(self, points, point_count):
    '''!
      @brief 读取轨迹模式点位（模式 0x05）。
      @param points: 点位输出 list
      @param point_count: 点数输出容器(list/dict/object.value)
      @return True or False
    '''

  def get_config_file_mode_points(self, points, point_count):
    '''!
      @brief 读取配置文件模式点位（模式 0x06）。
      @param points: 点位输出 list
      @param point_count: 点数输出容器(list/dict/object.value)
      @return True or False
    '''

  def get_detection_range_mode(self):
    '''!
      @brief 获取当前探测范围模式。
      @return 模式值
    '''

  def get_people_count_info(self, mode=GET_DATA_ACTIVE):
    '''!
      @brief 获取人数统计值。
      @param mode: 数据模式
      @return 人数值
    '''

  def set_people_report_interval(self, interval):
    '''!
      @brief 设置人数上报周期。
      @param interval: 秒
      @return True or False
    '''

  def get_people_report_interval(self):
    '''!
      @brief 获取人数上报周期。
      @return 秒
    '''

  def clear_people_count(self):
    '''!
      @brief 清除人数统计。
      @return True or False
    '''

  def set_trajectory_generate_distance(self, distance_cm):
    '''!
      @brief 设置轨迹生成距离阈值。
      @param distance_cm: 阈值(cm)
      @return True or False
    '''

  def get_trajectory_generate_distance(self):
    '''!
      @brief 获取轨迹生成距离阈值。
      @return 阈值(cm)
    '''

  def set_trajectory_hold_time(self, hold_time):
    '''!
      @brief 设置轨迹保持时间。
      @param hold_time: 秒
      @return True or False
    '''

  def get_trajectory_hold_time(self):
    '''!
      @brief 获取轨迹保持时间。
      @return 秒
    '''

  def set_no_person_delay(self, delay_time):
    '''!
      @brief 设置无人延迟时间。
      @param delay_time: 秒
      @return True or False
    '''

  def get_no_person_delay(self):
    '''!
      @brief 获取无人延迟时间。
      @return 秒
    '''
```

## 历史

- 2026/05/22 - V1.0.0 版本

## 创作者

Written by JiaLi(zhixin.liu@dfrobot.com), 2026. (Welcome to our [website](https://www.dfrobot.com/))
