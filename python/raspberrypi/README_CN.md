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
      @brief 初始化传感器模块。
      @n 打开串口（如需要）并等待初始化完成。
      @return True: 初始化成功，False: 初始化失败。
    '''

  def close(self):
    '''!
      @brief 关闭串口。
    '''

  def is_init_finished(self):
    '''!
      @brief 查询模块是否初始化完成。
      @return 初始化完成返回 True，否则返回 False。
    '''

  def is_connected(self):
    '''!
      @brief 检查传感器是否已连接。
      @return True: 已连接，False: 未连接。
    '''

  def reset(self):
    '''!
      @brief 复位传感器。
      @return True: 复位成功，False: 复位失败。
    '''

  def factory_reset(self):
    '''!
      @brief 恢复传感器出厂设置。
      @return True: 复位成功，False: 复位失败。
    '''

  def get_heartbeat(self, mode=GET_DATA_ACTIVE):
    '''!
      @brief 获取传感器的心跳状态。
      @param mode: 数据获取模式。
      @n          GET_DATA_ACTIVE: 主动获取最新心跳状态。
      @n          GET_DATA_REPORT: 从最近一次上报中获取最新心跳状态。
      @return True: 检测到心跳，False: 未检测到心跳。
    '''

  def get_reported_event(self, timeout=0.05):
    '''!
      @brief 等待并解析传感器主动上报的一帧数据。
      @param timeout: 等待完整 UART 上报帧的最长时间，单位秒（默认：0.05）。
      @n          调用最多阻塞 timeout。若超时仍无完整帧，返回 EVENT_NONE。
      @n          若帧更早到达，解码完成后立即返回（可能短于 timeout）。
      @return 上报事件类型。
      @n          EVENT_NONE: 本轮无完整帧（含等待超时）。
      @n          EVENT_TRAJECTORY: 检测到轨迹跟踪事件。
      @n          EVENT_PRESENCE: 检测到存在检测事件。
      @n          EVENT_MOTION: 检测到人体运动事件。
      @n          EVENT_TAG: 检测到标签事件。
      @n          EVENT_HEARTBEAT: 检测到心跳事件。
      @n          EVENT_INIT_FINISHED: 检测到初始化完成事件。
      @n          EVENT_PEOPLE_COUNT: 检测到人数统计事件。
      @n          EVENT_UNKNOWN: 收到完整帧，但事件类型无法识别。
      @n          EVENT_ERROR: 内部错误（如空指针）；应用层较少见。
    '''

  def get_hardware_version(self):
    '''!
      @brief 获取传感器的硬件版本。
      @return 硬件版本字符串。
    '''

  def get_firmware_version(self):
    '''!
      @brief 获取传感器的固件版本。
      @return 固件版本字符串。
    '''

  def set_install_info(self, info):
    '''!
      @brief 设置传感器的安装信息。
      @param info: 安装信息。
      @n          mode: 安装模式，INSTALL_MODE_SIDE 或 INSTALL_MODE_TOP。
      @n          height_cm: 安装高度，单位 cm。
      @n            - 侧装（z_angle 0°）：默认 180 cm，建议 180±20 cm（过低易被遮挡）。
      @n            - 顶装（z_angle 90°）：建议 220-280 cm（2.2-2.8 m）。
      @n          z_angle: 俯仰倾角，单位度（默认 0°）。0° = 侧装（沿 +Y 看），90° = 顶装（向下看）。
      @n            坐标系说明见 DFRobot_InstallInfo（相对目标位置的传感器 X/Y）。
      @return True: 设置成功，False: 设置失败。
      @note 无效 mode 或 height 返回 False。超出范围的角度会被钳位。
      @note 安装高度过低时容易被遮挡
    '''

  def get_install_info(self, info):
    '''!
      @brief 获取传感器的安装信息。
      @param info: 安装信息。
      @n          mode: 安装模式，INSTALL_MODE_SIDE 或 INSTALL_MODE_TOP。
      @n          height_cm: 安装高度，单位 cm。
      @n            - 侧装（z_angle 0°）：默认 180 cm，建议 180±20 cm（过低易被遮挡）。
      @n            - 顶装（z_angle 90°）：建议 220-280 cm（2.2-2.8 m）。
      @n          z_angle: 安装倾角，单位度。定义安装方式：0° = 侧装，90° = 顶装。
      @return True: 获取成功，False: 获取失败。
    '''

  def set_install_height(self, height):
    '''!
      @brief 设置传感器的安装高度。
      @param height: 安装高度，单位 cm。
      @n            - 侧装（z_angle 0°）：默认 180 cm，建议 180±20 cm。
      @n            - 顶装（z_angle 90°）：建议 220-280 cm（2.2-2.8 m）。
      @return True: 设置成功，False: 设置失败。
      @note 安装高度过低时容易被遮挡
    '''

  def get_install_height(self):
    '''!
      @brief 获取传感器的安装高度。
      @return 安装高度（cm）。失败时返回 0。
    '''

  def set_presence_enable(self, enable):
    '''!
      @brief 开启或关闭传感器的存在检测功能。
      @param enable: 开启或关闭存在检测功能。
      @n          True: 开启，False: 关闭。
      @return True: 设置成功，False: 设置失败。
    '''

  def get_presence_enable(self, enable):
    '''!
      @brief 获取存在检测功能是否已开启。
      @param enable: 用于接收使能状态的输出容器。
      @n          True: 已开启，False: 已关闭。
      @n          支持的容器：list / dict / 带 value 字段的对象。
      @return True: 获取成功，False: 获取失败。
    '''

  def get_presence_state(self, mode=GET_DATA_ACTIVE):
    '''!
      @brief 获取探测范围内当前是否有人存在。
      @param mode: 数据获取模式。
      @n          GET_DATA_ACTIVE: 查询最新数据并更新缓存。
      @n          GET_DATA_REPORT: 返回最近一次上报的缓存数据。
      @return 存在检测结果。
      @n          NO_PRESENCE: 未检测到存在。
      @n          PRESENCE: 检测到存在。
    '''

  def get_motion_state(self, mode=GET_DATA_ACTIVE):
    '''!
      @brief 获取探测范围内当前人体运动状态。
      @param mode: 数据获取模式。
      @n          GET_DATA_ACTIVE: 查询最新数据并更新缓存。
      @n          GET_DATA_REPORT: 返回最近一次上报的缓存数据。
      @return 运动状态。
      @n          MOTION_NONE: 无运动状态。
      @n          MOTION_STATIC: 静止。
      @n          MOTION_ACTIVE: 运动。
    '''

  def set_trajectory_track_enable(self, enable):
    '''!
      @brief 开启或关闭传感器的轨迹跟踪功能。
      @param enable: 开启或关闭轨迹跟踪功能。
      @n          True: 开启，False: 关闭。
      @return True: 设置成功，False: 设置失败。
    '''

  def get_trajectory_track_enable(self, enable):
    '''!
      @brief 查询轨迹跟踪功能是否已开启。
      @param enable: 用于接收使能状态的输出容器。
      @n          True: 已开启，False: 已关闭。
      @n          支持的容器：list / dict / 带 value 字段的对象。
      @return True: 查询成功，False: 查询失败。
    '''

  def set_frame_generate_count(self, frames):
    '''!
      @brief 设置检查状态切换到活跃状态的确认帧数。
      @n 数值越大，抑制噪点越明显，同时会影响触发距离。
      @param frames: 帧数，有效范围：1-7，默认：7。
      @return True: 设置成功，False: 设置失败。
    '''

  def get_frame_generate_count(self, frames):
    '''!
      @brief 查询检查状态切换到活跃状态的确认帧数。
      @n 数值越大，抑制噪点越明显，同时会影响触发距离。
      @param frames: 用于接收帧数的输出容器。
      @n          支持的容器：list / dict / 带 value 字段的对象。
      @return True: 查询成功，False: 查询失败。
    '''

  def get_target_list(self, mode=GET_DATA_ACTIVE):
    '''!
      @brief 获取传感器的目标信息列表。
      @param mode: 数据获取模式。
      @n          GET_DATA_ACTIVE: 读取前查询最新目标信息。
      @n          GET_DATA_REPORT: 从上报缓存读取目标信息。
      @return DFRobot_TargetInfo 对象列表。
    '''

  def set_trk_led(self, enable):
    '''!
      @brief 使能或关闭轨迹指示灯功能。
      @n 使能后，仅在学习/生成轨迹范围过程中点亮，其它时刻均不点亮。
      @param enable: True 使能，False 关闭。
      @return True: 设置成功，False: 设置失败。
    '''

  def set_occ_led(self, enable):
    '''!
      @brief 使能或关闭占用指示灯功能。
      @n 使能后，当探测范围内有人存在（空间被占用）时，LED 会点亮。
      @param enable: True 使能，False 关闭。
      @return True: 设置成功，False: 设置失败。
    '''

  def get_trk_led(self):
    '''!
      @brief 获取轨迹指示灯功能是否使能。
      @n 使能后，仅在学习/生成轨迹范围过程中点亮，其它时刻均不点亮。
      @return True: 指示灯功能已使能，False: 指示灯功能已关闭。
    '''

  def get_occ_led(self):
    '''!
      @brief 获取占用指示灯功能是否使能。
      @n 使能后，当探测范围内有人存在（空间被占用）时，LED 会点亮。
      @return True: 指示灯功能已使能，False: 指示灯功能已关闭。
    '''

  def get_tags(self, mode=GET_DATA_ACTIVE, max_tags=None):
    '''!
      @brief 获取全部标签配置信息。
      @param mode: 数据获取模式，保留用于兼容。
      @n          GET_DATA_ACTIVE: 从设备查询最新标签配置。
      @n          GET_DATA_REPORT: 当前行为与 GET_DATA_ACTIVE 相同。
      @n          每个标签的 io_index：0 表示不使用；2-6 对应 IO2-IO6。
      @param max_tags: 返回的最大标签数量。None 表示返回全部已解析标签。
      @return DFRobot_TagConfig 对象列表。
    '''

  def set_tag(self, tag):
    '''!
      @brief 以尺寸模式设置单个标签。
      @param tag: 标签配置。
      @n          tag_index: 标签索引。
      @n          tag_type: 标签类型。
      @n          scope_type: 标签范围类型。
      @n          io_index: IO 联动索引。0 表示不使用；2-6 对应 IO2-IO6。
      @n          width: 标签宽度或圆形半径，单位 cm。
      @n          height: 标签高度，单位 cm。
      @return 标签设置状态。
      @n          TAG_SET_COMM_ERROR: 通信失败或响应不匹配。
      @n          TAG_SET_SUCCESS: 标签设置成功。
      @n          TAG_SET_TRACK_COUNT_ERROR: 轨迹数量不等于 1。
      @n          TAG_SET_ALREADY_USED: 标签已被占用。
      @n          TAG_SET_INDEX_OUT_OF_RANGE: 标签索引越界。
      @note 此 API 忽略 DFRobot_TagConfig 中的 center_x/center_y。
      @note 无效的 tag_type、scope_type 或 io_index 在发送命令前返回 TAG_SET_COMM_ERROR。
      @note 使用此 API 设置标签时，需确保轨迹数量为 1。
      @note 最多设置 32 个标签。
    '''

  def clear_tag(self, tag_index):
    '''!
      @brief 清除标签配置。
      @param tag_index: 标签索引（协议载荷中的 1 字节索引，0-254）。
      @return True: 清除成功，False: 清除失败。
      @note 0xFF 保留给 clear_all_tags()；请勿传给 clear_tag()。
      @note 标签索引越界时设备响应返回 0xFE。
    '''

  def clear_all_tags(self):
    '''!
      @brief 清除全部标签配置。
      @return True: 清除成功，False: 清除失败。
    '''

  def set_tags_from_config(self, tags):
    '''!
      @brief 以坐标模式从列表批量设置标签配置。
      @param tags: DFRobot_TagConfig 对象的可迭代对象。
      @n          每个标签的 io_index：0 表示不使用；2-6 对应 IO2-IO6。
      @return True: 设置成功，False: 设置失败。
      @note 可以按坐标形式设置标签，无需满足轨迹数量为 1 的要求
      @note 最多设置 32 个标签。
      @note 任一标签的 tag_type、scope_type 或 io_index 无效时，发送命令前返回 False。
    '''

  def get_tag_info(self):
    '''!
      @brief 获取从主动上报包（CTRL 0x07, CMD 0x1B）解码的最新标签事件。
      @return 有效时返回 DFRobot_TagInfo 对象，否则返回 None。
      @note 此 API 仅读取上报缓存。请先调用 get_reported_event() 接收新的上报数据。
      @note 标签事件上报包含 io_index。
    '''

  def set_four_sided_range_mode(self, range_info):
    '''!
      @brief 设置四边边界探测范围。
      @param range_info: 边界范围设置。
      @n          x_max: X 最大边界，单位 cm。
      @n          x_min: X 最小边界，单位 cm。
      @n          y_max: Y 最大边界，单位 cm。
      @n          y_min: Y 最小边界，单位 cm。
      @return True: 设置成功，False: 设置失败。
    '''

  def get_four_sided_range_mode(self, range_info):
    '''!
      @brief 查询并获取四边边界探测范围。
      @param range_info: 用于接收边界范围设置的 FourSidedRange 对象。
      @return True: 获取成功，False: 获取失败。
    '''

  def set_trajectory_range_mode(self, learning):
    '''!
      @brief 开始生成轨迹探测范围，或使用此前已生成的探测范围。
      @n 若开启生成/学习（True）：传感器在确认只有一个轨迹后，
      @n 开始生成/学习探测范围。
      @n 若在学习过程中关闭（False）：传感器停止学习，
      @n 保存并启用自动生成的探测范围。
      @n 若当前未启用轨迹范围模式，可调用 set_trajectory_range_mode(False)
      @n 启用并使用此前已生成/保存的探测范围。
      @param learning: 轨迹范围生成/学习开关。
      @n          True: 开始生成/学习探测范围。
      @n          False: 停止学习并保存/启用已生成范围，或使用此前已保存的范围。
    '''

  def get_trajectory_range_mode(self, points, point_count):
    '''!
      @brief 查询并获取轨迹模式下的范围点。
      @param points: 用于接收轨迹模式点位的缓冲区/列表。
      @param point_count: 用于接收点数的输出容器（list/dict/带 value 的对象）。
      @return True: 查询成功，False: 查询失败。
      @note points 缓冲区必须能够容纳至少 MAX_POINTS 个点。
    '''

  def set_config_file_mode_points(self, points):
    '''!
      @brief 使用配置文件模式设置探测范围点。
      @param points: DFRobot_Point 对象的可迭代对象。
      @return True: 设置成功，False: 设置失败。
      @note 点值使用符号位 int16 编码（bit15: 0=正数，1=负数）。
      @note 点数上限为 MAX_POINTS。
    '''

  def get_config_file_mode_points(self, points, point_count):
    '''!
      @brief 查询并获取配置文件模式下的范围点。
      @param points: 用于接收配置文件模式点位的缓冲区/列表。
      @param point_count: 用于接收点数的输出容器（list/dict/带 value 的对象）。
      @return True: 查询成功，False: 查询失败。
      @note points 缓冲区必须能够容纳至少 MAX_POINTS 个点。
    '''

  def get_detection_range_mode(self):
    '''!
      @brief 查询当前探测范围模式。
      @return 当前探测范围模式。
    '''

  def get_people_count(self, mode=GET_DATA_ACTIVE):
    '''!
      @brief 获取实时人数。仅统计已确认为真实目标人的数量。
      @param mode: 数据获取模式。
      @n          GET_DATA_ACTIVE: 查询最新数据并更新缓存。
      @n          GET_DATA_REPORT: 直接返回缓存数据。
      @return 过滤后的实时人数。
    '''

  def set_real_time_people_time(self, interval):
    '''!
      @brief 设置人数统计上报间隔。
      @param interval: 上报间隔，单位秒。默认：1 s。有效范围：1-3600 秒。
      @return True: 设置成功，False: 设置失败。
    '''

  def get_real_time_people_time(self):
    '''!
      @brief 获取人数统计上报间隔。
      @return 上报间隔（秒）。失败时返回 0。
    '''

  def clear_people_count(self):
    '''!
      @brief 清除传感器检测到的人数，并从 0 重新开始检测/跟踪。
      @n 当探测范围内仍有干扰物、传感器
      @n 无法自行确认或清除时，可调用本接口刷新人数状态。
      @return True: 清除成功，False: 清除失败。
    '''

  def set_track_meters(self, distance_cm):
    '''!
      @brief 设置轨迹运动距离阈值。
      @n 轨迹产生后，轨迹还需运动该距离才会确认为人。
      @n 用于调整实时人数统计接口的判断条件。
      @param distance_cm: 距离阈值，单位 cm。默认 0 cm，有效范围：0-1000 cm。
      @return True: 设置成功，False: 设置失败。
    '''

  def get_track_meters(self):
    '''!
      @brief 获取轨迹运动距离阈值。
      @n 轨迹产生后，轨迹还需运动该距离才会确认为人。
      @n 用于调整实时人数统计接口的判断条件。
      @return 距离阈值（cm）。失败时返回 0。
    '''

  def set_track_exists_time(self, time):
    '''!
      @brief 设置轨迹保持时间。
      @n 用于调整实时人数统计接口的判断条件。
      @param time: 保持时间，单位秒。默认 0 秒，有效范围：0-600 秒。
      @return True: 设置成功，False: 设置失败。
    '''

  def get_track_exists_time(self):
    '''!
      @brief 获取轨迹保持时间。
      @n 用于调整实时人数统计接口的判断条件。
      @return 保持时间（秒）。失败时返回 0。
    '''

  def set_unmanned_time(self, delay_time):
    '''!
      @brief 设置无人延迟时间。
      @n 判断目标点是否为真实目标人的周期时间。
      @n 若非真实目标人，等待该周期后自动清除。
      @param delay_time: 周期时间，单位秒。默认 30 秒，有效范围：5-3600 秒。
      @return True: 设置成功，False: 设置失败。
    '''

  def get_unmanned_time(self):
    '''!
      @brief 获取无人延迟时间。
      @n 判断目标点是否为真实目标人的周期时间。
      @n 若非真实目标人，等待该周期后自动清除。
      @return 周期时间（秒）。失败时返回 0。
    '''
```

## 示例

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

## 历史

- 2026/05/22 - V1.0.0 版本

## 创作者

Written by JiaLi(jia.li@dfrobot.com), 2026. (Welcome to our [website](https://www.dfrobot.com/))
