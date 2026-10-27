# 中狗APP&SDK通信协议-1.3.0-对外开放版本

| 序号 | 版本 | 变更方式 | 变更内容 | 变更人 | 变更时间 |
| --- | --- | --- | --- | --- | --- |
| 1 | 1.2.0 | 编写 | 初版 | 李文熙 | 2025.12.09 |
| 2 | 1.3.0 | 修改 | 根据当前对外开放能力整理协议文档 | 朱文学 | 2026.07.24 |

## 1. 文档说明

本文档面向外部用户联调使用，描述当前对外开放版本中可直接使用的协议能力、报文格式和参数含义。

适用范围：

- 外部 APP / SDK / 客户端 与设备通信
- 按当前对外开放能力进行控制、查询和订阅

## 2. 相对上一版的主要差异

相对旧版对外文档，本版需要重点关注以下变化：

- 摄像头码率修改由旧写法调整为 `1019 target=805`
- 外设电源控制使用 `1019/1020 target=806/807/808`
- 支持任务相关协议：`1025`、`1026`、`1027`、`1028`
- 充电桩配对使用 `1022` 接收响应和 `1024` 终态推送，`1023` 保留为状态查询
- 本文档不包含未对外开放的配置项和扩展能力

## 3. 通用协议头

所有报文使用 JSON：

```json
{
  "head": {
    "type": 1000,
    "time": 1757310525610,
    "src": 3
  },
  "data": {}
}
```

通用字段说明：

| 字段 | 含义 | 类型 | 说明 |
| --- | --- | --- | --- |
| `head.type` | 消息类型 | `uint32` | 见各章节 |
| `head.time` | 时间戳 | `uint64` | 单位：毫秒 |
| `head.src` | 数据来源 | `uint32` | 普通外部客户端建议使用 `3`；自研遥控器使用 `5`；agent 使用 `6` |
| `code` | Gateway 统一结果码 | `int` | 仅响应/主动推送携带；`0` 最终成功；`1` 已接受但未确认完成；`>=1001` 失败 |
| `data` | 业务数据 | `object` | 见各章节 |

客户端请求只发送 `head + data`，不包含 `code`。Gateway 响应和主动推送使用 `head + code + data`；客户端统一以顶层 `code` 判断请求结果。普通状态推送使用 `code=0`，异步操作的失败终态推送使用对应非 0 结果码。

无业务请求字段的协议允许省略 `data`，Gateway 按 `data: {}` 处理；显式传 `null` 或非对象仍属于非法请求。错误响应保留旧客户端按 `head.type` 解析时需要的正常协议字段（例如 `1002.data.cmd`）。

通用结果码：

| code | 枚举 | 含义 |
| ---: | --- | --- |
| `0` | `SUCCESS` | 请求完成并成功；或主动推送报文本身有效 |
| `1` | `ACCEPTED` | 请求已接受/已下发，最终结果尚未确认 |
| `1001` | `UNSUPPORTED_DEVICE_CAPABILITY` | 当前机型不支持该能力 |
| `1002` | `INVALID_REQUEST` | 请求字段、范围或枚举非法 |
| `1003` | `UNAUTHENTICATED` | 未握手或 Session 已失效 |
| `1004` | `ACCESS_DENIED` | 无控制权或协议权限不足 |
| `1005` | `UNSUPPORTED_OPERATION` | action/target 不支持 |
| `1006` | `INVALID_STATE` | 当前机器人状态不允许执行 |
| `1101` | `RESOURCE_BUSY` | 互斥资源忙 |
| `1102` | `RESOURCE_NOT_FOUND` | 资源或 operation 不存在 |
| `1103` | `RATE_LIMITED` | 请求频率过高 |
| `1201` | `SERVICE_UNAVAILABLE` | 依赖服务不可用 |
| `1202` | `DISPATCH_FAILED` | Gateway 明确下发失败 |
| `1203` | `QUERY_TIMEOUT` | 查询或异步结果等待超时 |
| `1204` | `INVALID_UPSTREAM_RESPONSE` | 下层响应格式非法 |
| `1301` | `OPERATION_REJECTED` | 下层明确拒绝操作 |
| `1302` | `OPERATION_FAILED` | 操作明确失败 |
| `1303` | `RESULT_UNKNOWN` | 最终结果未知，应查询状态，禁止直接重发 |
| `1304` | `OPERATION_CANCELLED` | 操作被取消 |
| `1900` | `INTERNAL_ERROR` | Gateway 内部异常 |

同步请求返回最终的 `0/错误码`；异步请求先返回 `1`，最终结果由对应状态/结果协议给出；高频控制协议不逐帧响应。

`head.src` 推荐取值：

| 取值 | 含义 |
| --- | --- |
| `1` | 设备端 |
| `2` | APP |
| `3` | 外部 SDK / 客户端 |
| `4` | 第三方直连 |
| `5` | 自研遥控器 |
| `6` | agent |

`src=5` 和 `src=6` 均按 APP 协议权限处理。主动申请控制权时，优先级为：`APP（src=2） > agent（src=6） > 自研遥控器（src=5）`。

## 4. 开放能力总表

| type | 名称 | 方向 | 是否开放 | 说明 |
| --- | --- | --- | --- | --- |
| `1000` | 握手 | client <-> device | 是 | 连接后第一步 |
| `1001` | 心跳 | client <-> device | 是 | 保活 |
| `1002` | 命令 | client <-> device | 是 | 动作、补光灯、速度等 |
| `1003` | 遥控信息 | client -> device | 是 | 摇杆控制 |
| `1110` | 位控信息 | client -> device | 是 | 机体位置/姿态增量控制 |
| `1111` | 直接速度控制 | client -> device | 是 | 物理速度控制 |
| `1004` | 本体状态 | device -> client | 是 | 状态推送 |
| `1005` | 本体故障状态 | device -> client | 是 | 故障推送 |
| `1008` | 配置传感数据 | client <-> device | 是 | 订阅 IMU、光强、运控、速度、JointState |
| `1009` | 拍照 | client <-> device | 是 | 前后摄拍照 |
| `1013` | 接管控制权 | client <-> device | 是 | 获取控制权 |
| `1015` | 释放控制权 | client <-> device | 是 | 释放控制权 |
| `1016` | 控制权被接管通知 | device -> client | 是 | 通知类消息 |
| `1017` | 控制权被释放通知 | device -> client | 是 | 通知类消息 |
| `1019 target=805` | 摄像头码率修改 | client <-> device | 是 | 设置码率 |
| `1019/1020 target=806/807/808/826/827` | 外设电源设置/查询 | client <-> device | 是 | M1 48V / 24V / 12V；L2 24V / 航插口 |
| `1022` | 充电桩连接控制 | client <-> device | 是 | 请求接收响应，不代表执行完成 |
| `1023` | 充电桩信息查询 | client <-> device | 是 | 查询当前观测状态 |
| `1024` | 充电桩连接终态结果 | device -> client | 是 | 只推送给原请求会话 |
| `1025` | 状态切换 | client <-> device | 是 | `Idle` / `Remote` |
| `1026` | 启动任务 | client <-> device | 是 | `Recharge` / `Undock` |
| `1027` | 停止任务 | client <-> device | 是 | `Recharge` / `Undock` |
| `1028` | 任务状态 | device -> client | 是 | 任务进度与结果 |
| `1050` | 挥手 | client <-> device | 是 | 断开前发送 |
| `1100` | IMU 数据 | device -> client | 是 | 订阅后推送 |
| `1101` | 光强数据 | device -> client | 是 | 订阅后推送 |
| `1102` | 运控数据 | device -> client | 是 | 订阅后推送 |
| `1103` | 速度数据 | device -> client | 是 | 订阅后推送 |
| `1104` | JointState 数据 | device -> client | 是 | 订阅后推送 |

## 5. 握手 - 1000

### 5.1 请求字段

| 字段 | 含义 | 类型 | 说明 |
| --- | --- | --- | --- |
| `version` | 客户端版本 | `string` | 建议填写 |
| `protocol_version` | 协议版本 | `string` | 必填，当前填写 `1.3.0` |
| `device` | 设备名称 | `string` | 选填 |
| `platform` | 平台 | `string` | 选填 |
| `package_name` | 包名 | `string` | 选填 |
| `observe_only` | 只读会话 | `boolean` | 选填，默认 `false`；为 `true` 时不申请空闲控制权 |

请求示例：

```json
{
  "head": {
    "type": 1000,
    "time": 1757310525611,
    "src": 3
  },
  "data": {
    "version": "1.0.0",
    "protocol_version": "1.3.0",
    "device": "client",
    "platform": "linux",
    "package_name": "demo_client"
  }
}
```

### 5.2 响应字段

| 字段 | 含义 | 类型 | 说明 |
| --- | --- | --- | --- |
| `status_code` | 状态码 | `uint32` | 见下表 |
| `sn` | 设备 SN | `string` | |
| `ssid` | SSID | `string` | |
| `model` | 机型 | `string` | |
| `version.system` | 系统版本 | `string` | |
| `device_type` | 设备类型 | `string` | 设备原始类型字符串，当前已知取值见下表 |
| `has_brake_resistor` | 是否带刹车电阻 | `boolean` | |
| `control_meta` | 当前接收会话的控制权快照 | `object` | 成功握手时返回，以 `self_is_owner` 判断当前会话是否拥有控制权 |

`device_type` 当前已知取值如下。表格按产品系列、底盘类型和版本排序：

#### 中狗轮足系列

| 协议值 | 设备类型 |
| --- | --- |
| `ZSM-1` | 中狗轮足 |
| `ZSM-1-Pro` | 中狗轮足激光版 |
| `ZSM-1-Ultra` | 中狗轮足环视版 |
| `ZSM-1-Air` | 中狗轮足 Air 版 |

#### 中狗点足系列

| 协议值 | 设备类型 |
| --- | --- |
| `ZSM-1F` | 中狗点足 |
| `ZSM-1F-Pro` | 中狗点足激光版 |
| `ZSM-1F-Ultra` | 中狗点足环视版 |
| `ZSM-1F-Air` | 中狗点足 Air 版 |

#### 防爆狗轮足系列

| 协议值 | 设备类型 |
| --- | --- |
| `ZS-SP1` | 防爆狗轮足 |

#### 小狗 L2 轮式系列

| 协议值 | 设备类型 |
| --- | --- |
| `ZSL-2` | 小狗轮式运动版 |
| `ZSL-2-Ultra` | 小狗轮式环视版 |

#### 小狗 L2 点足系列

| 协议值 | 设备类型 |
| --- | --- |
| `ZSL-2F` | 小狗点足运动版 |
| `ZSL-2F-Ultra` | 小狗点足环视版 |

`device_type` 按字符串处理，不应按数值枚举解析。对于未在本表列出的新类型，客户端应保留原始字符串，并按未知机型处理。

`status_code` 取值：

| 取值 | 含义 |
| --- | --- |
| `0` | 成功 |
| `10` | 协议版本不匹配 |
| `20` | 握手准入失败：已有相同 `src` 的 Owner，或同一连接握手后修改 `src` |

已有不同 `src` 的 Owner 时，新连接仍返回 `status_code=0` 并作为读会话接入。`src` 只表示客户端类型，不能唯一标识会话；客户端必须使用 `control_meta.self_is_owner` 判断本连接是否拥有控制权。

成功响应示例：

```json
{
  "head": {"type": 1000, "time": 1757310525612, "src": 1},
  "code": 0,
  "data": {
    "status_code": 0,
    "sn": "ZG-DEMO-0001",
    "ssid": "robot-ap",
    "model": "G20",
    "version": {"system": "1.0.0"},
    "device_type": "ZSM-1-Pro",
    "has_brake_resistor": false,
    "control_meta": {"has_owner": true, "owner_src": 3, "priority": 100, "self_priority": 100, "self_is_owner": true, "owner_relation": "self", "can_takeover": true, "deny_code": null, "epoch": 1, "transition": "handshake_grant"}
  }
}
```

`control_meta` 主要字段：`has_owner`、`owner_src`、`priority`、`self_priority`、`self_is_owner`、`owner_relation`、`can_takeover`、`deny_code`、`epoch`、`transition`。

`owner_relation` 可能为 `none`、`self`、`same_src_other_session`、`other_src`；`transition` 会标识 `claim_unowned`、`higher_priority_takeover`、`release` 等控制权变化原因。

## 6. 心跳 - 1001

用途：连接保活。

请求示例：

```json
{
  "head": {
    "type": 1001,
    "time": 1757310525615,
    "src": 3
  },
  "data": {}
}
```

响应示例：

```json
{
  "head": {"type": 1001, "time": 1757310525616, "src": 1},
  "code": 0,
  "data": {}
}
```

## 7. 命令 - 1002

### 7.1 请求字段

| 字段 | 含义 | 类型 |
| --- | --- | --- |
| `cmd` | 命令字 | `string` |

### 7.2 当前开放命令

| 类别 | `cmd` | 说明 |
| --- | --- | --- |
| 急停 | `emergency/stop` | 软急停 |
| 急停恢复 | `emergency/recover` | 解除软急停 |
| 动作 | `action/stand_up` | 站立 |
| 动作 | `action/balance_stand` | 原地站立 |
| 动作 | `action/stair` | 登阶姿态 |
| 动作 | `action/crawl` | 匍匐 |
| 动作 | `action/crawl_walk` | 匍匐前进 |
| 动作 | `action/lie_down` | 卧倒 |
| 动作 | `action/locked` | 锁定 |
| 动作 | `action/climb` | 爬高台 |
| 动作 | `action/slim` | 瘦身 |
| 动作 | `action/gait_walk` | 步态 |
| 动作 | `action/dsb` | 挡鼠板 |
| 动作 | `action/pos_control` | 位控姿态 |
| 动作 | `action/skwalk` | 同膝姿态 |
| 方向 | `reverse_head_tail` | 头尾切换 |
| 速度 | `speed/low` | 低速 |
| 速度 | `speed/medium` | 中速 |
| 速度 | `speed/high` | 高速 |
| 补光灯 | `fill_light/front_light_on` | 前补光灯开 |
| 补光灯 | `fill_light/front_light_off` | 前补光灯关 |
| 补光灯 | `fill_light/back_light_on` | 后补光灯开 |
| 补光灯 | `fill_light/back_light_off` | 后补光灯关 |
| 补光灯 | `fill_light/light_auto_work_on` | 自动补光开 |
| 补光灯 | `fill_light/light_auto_work_off` | 自动补光关 |

请求示例：

```json
{
  "head": {
    "type": 1002,
    "time": 1757310525622,
    "src": 3
  },
  "data": {
    "cmd": "action/stand_up"
  }
}
```

成功响应示例：

```json
{
  "head": {
    "type": 1002,
    "time": 1757310525650,
    "src": 1
  },
    "code": 1,
  "data": {
    "cmd": "action/stand_up"
  }
}
```

说明：

- 响应表示命令已接收，不表示动作已执行完成

## 8. 遥控信息 - 1003

### 8.1 请求字段

| 字段 | 含义 | 类型 | 说明 |
| --- | --- | --- | --- |
| `lx` | 左右平移 | `number` | 范围 `[-1.0, 1.0]`；正数左移，负数右移 |
| `ly` | 前后移动 | `number` | 范围 `[-1.0, 1.0]`；正数前进，负数后退 |
| `rx` | 左右旋转 | `number` | 范围 `[-1.0, 1.0]` |
| `ry` | 俯仰 | `number` | 范围 `[-1.0, 1.0]` |
| `body.turn` | 左右翻滚 | `string` | `left` / `right` / `none` |
| `body.high_low` | 高低站姿 | `string` | `up` / `down` / `none` |

请求示例：

```json
{
  "head": {
    "type": 1003,
    "time": 1757310525622,
    "src": 3
  },
  "data": {
    "lx": 0.0,
    "ly": 0.5,
    "rx": 0.0,
    "ry": 0.0,
    "body": {
      "turn": "none",
      "high_low": "none"
    }
  }
}
```

### 8.2 位控信息 - 1110

用途：客户端发送机体位置和姿态增量。

| 字段 | 含义 | 类型 | Gateway 映射 |
| --- | --- | --- | --- |
| `x` | 机体 x 轴增量 | `number` | `delta_xyz[0]` |
| `y` | 机体 y 轴增量 | `number` | `delta_xyz[1]` |
| `z` | 机体 z 轴增量 | `number` | `delta_xyz[2]` |
| `roll` | 横滚增量 | `number` | `delta_rpy[0]` |
| `pitch` | 俯仰增量 | `number` | `delta_rpy[1]` |
| `yaw` | 偏航增量 | `number` | `delta_rpy[2]` |

请求示例：

```json
{
  "head": {
    "type": 1110,
    "time": 1757310525622,
    "src": 3
  },
  "data": {
    "x": 0.1,
    "y": 0.0,
    "z": 0.02,
    "roll": 0.0,
    "pitch": 0.0,
    "yaw": 0.1
  }
}
```

说明：

- gateway 将 `x/y/z` 映射到 `robot_common_interface/msg/RemoteMove.delta_xyz`
- gateway 将 `roll/pitch/yaw` 映射到 `RemoteMove.delta_rpy`，并发布到 `remote/move`
- gateway 不做单位换算和范围裁剪
- 当前实现对缺失、空值或不能转换为数字的字段按 `0.0` 处理
- 该高频控制消息不逐帧回复 ACK

### 8.3 直接速度控制 - 1111

用途：客户端使用物理速度控制机器人平面运动。

| 字段 | 含义 | 类型 | 单位 |
| --- | --- | --- | --- |
| `x` | 机体坐标系纵向速度，正数向前 | `number` | `m/s` |
| `y` | 机体坐标系横向速度，正数向左 | `number` | `m/s` |
| `yaw` | 偏航角速度，按右手定则为正 | `number` | `rad/s` |

请求示例：

```json
{
  "head": {
    "type": 1111,
    "time": 1757310525622,
    "src": 3
  },
  "data": {
    "x": 1.2,
    "y": 0.0,
    "yaw": -0.3
  }
}
```

说明：

- gateway 将数据转换为 `geometry_msgs/msg/Twist` 并发布到 `/remote/cmd_vel`
- `x`、`y`、`yaw` 必须全部存在且为有限数值
- RobotManager 不再按摇杆比例乘速度档位，但仍按当前档位上限裁剪
- 头尾切换、AEB 和移动超时仍由 RobotManager 处理
- 连续控制建议按 `20~50Hz` 发送；该高频消息不逐帧回复 ACK
- gateway 发布队列容量为 1，拥塞时覆盖尚未发布的旧值，只保留最新待发布指令

## 9. 本体状态 - 1004

用途：设备主动推送当前状态。

### 9.1 主要字段

| 字段 | 类型 | 说明 |
| --- | --- | --- |
| `head_angle` | `number` | 头部角度 |
| `head_direction` | `string` | `head` / `tail` |
| `motion_status` | `string` | 当前运动状态 |
| `current_state` | `int` | 当前总状态 |
| `mile_data` | `number` | 里程 |
| `ctrl_source` | `int` | 当前控制来源 |
| `fill_light.front` | `bool` | 前补光灯状态 |
| `fill_light.back` | `bool` | 后补光灯状态 |
| `fill_light.auto_work` | `bool` | 自动补光状态 |
| `estop.software` | `bool` | 软急停 |
| `estop.hardware` | `bool` | 硬急停 |
| `speed.level` | `string` | `slow` / `medium` / `high` |
| `speed.x` | `float` | 速度 x |
| `speed.y` | `float` | 速度 y |
| `speed.yaw` | `float` | 角速度 |
| `battery` | `object` | 电池信息 |
| `motor_temp` | `object` | 电机温度 |
| `control_meta` | `object` | 按接收会话生成的控制权快照；使用 `self_is_owner` 判断当前会话是否拥有控制权 |

低速档在设置和状态协议中的取值不同：`1002` 设置命令为 `speed/low`，`1004.speed.level` 状态值为 `slow`。客户端无需做兼容转换。

### 9.2 `current_state` 取值

| 取值 | 含义 |
| --- | --- |
| `0` | Unknown |
| `1` | Idle |
| `2` | Remote |
| `3` | Ota |
| `4` | Recharge |
| `5` | Mapping |
| `6` | Navigation |
| `7` | Safety |
| `8` | SelfTest |
| `9` | SoftShutdown |
| `10` | Silence |
| `11` | Follow |
| `12` | Track |
| `13` | Undock |
| `14` | dockCalibration |
| `15` | Estop |
| `16` | Fall |
| `17` | LocalRemote |
| `18` | LowLevel |

`current_state` 直接按 `/robot_manager/robot_manager_status.current_state` 的原始状态名一一映射。`Remote` 与 `LocalRemote` 不合并，分别为 `2` 和 `17`。

以上是 `1004` 状态上报枚举，不表示所有值都能作为 `1025.target_state` 请求；`1025` 的现有调用逻辑保持不变。

主动推送示例：

```json
{
  "head": {"type": 1004, "time": 1757310525800, "src": 1},
  "code": 0,
  "data": {
    "temp": 35.2,
    "head_angle": 1.234,
    "head_direction": "head",
    "motion_status": "walk",
    "current_state": 2,
    "obstacle_avoidance": true,
    "mile_data": 12.346,
    "ctrl_source": 1,
    "fill_light": {"front": false, "back": false, "auto_work": true},
    "estop": {"software": false, "hardware": false},
    "speed": {"level": "slow", "x": 0.1, "y": 0.0, "yaw": 0.2},
    "battery": {"power1": 85.0, "power2": 83.5, "present1": true, "present2": true},
    "motor_temp": {"fl1": 31.2, "fr1": 32.4},
    "charging_pile": {"connect": false},
    "uwb": {"connect": true},
    "control_meta": {"has_owner": true, "owner_src": 3, "priority": 100, "self_priority": 100, "self_is_owner": true, "owner_relation": "self", "can_takeover": true, "deny_code": null, "epoch": 1, "transition": "none"}
  }
}
```

### 9.3 常见 `motion_status`

| 取值 | 含义 |
| --- | --- |
| `stand_up` | 站立 |
| `walk` | 行走姿态 |
| `balance_stand` | 原地站立 |
| `lie_down` | 卧倒 |
| `crawl` | 匍匐 |
| `crawl_walk` | 匍匐前进 |
| `stair` | 登阶姿态 |
| `climb` | 爬高台 |
| `slim` | 瘦身 |
| `gait_walk` | 步态 |
| `dsb` | 挡鼠板 |
| `joint_lock` | 锁定 |
| `pos_control` | 位控姿态 |
| `sk_walk` | 同膝姿态 |

## 10. 本体故障状态 - 1005

### 10.1 字段说明

| 字段 | 含义 | 类型 |
| --- | --- | --- |
| `faults` | 故障列表 | `array` |
| `faults[].module` | 模块 | `string` |
| `faults[].submodule` | 子模块 | `string` |
| `faults[].fault` | 故障标识 | `string` |
| `faults[].error_code` | 故障码 | `uint32` |
| `faults[].level` | 故障等级 | `uint8` |
| `faults[].stamp` | 故障时间戳，单位为秒 | `uint32` |

### 10.2 `level` 取值

| 取值 | 含义 |
| --- | --- |
| `0` | 正常 |
| `1` | 告警 |
| `2` | 错误 |
| `3` | 致命错误 |

主动推送示例：

```json
{
  "head": {"type": 1005, "time": 1757310525900, "src": 1},
  "code": 0,
  "data": {
    "faults": [
      {
        "module": "camera",
        "submodule": "front",
        "fault": "offline",
        "error_code": 1001,
        "level": 2,
        "stamp": 1757310525
      }
    ]
  }
}
```

## 11. 配置传感数据 - 1008

### 11.1 请求字段

| 字段 | 含义 | 类型 | 说明 |
| --- | --- | --- | --- |
| `target` | 目标能力 | `uint32` | 见下表 |
| `enable` | 开关 | `bool` | `true` 开启，`false` 关闭 |
| `freq` | 目标推送频率 | `uint32` | 仅速度支持配置，默认 `10Hz`、范围 `1~50Hz`；IMU、光强、JointState、运控分别使用固定 `100Hz`、`1Hz`、`100Hz`、`50Hz` 目标上限。固定能力忽略请求值，应答返回对应目标频率。实际线速还受 ROS 源频率、线程调度和发送负载限制 |

### 11.2 支持的 `target`

| `target` | 含义 | 推送类型 |
| --- | --- | --- |
| `201` | IMU | `1100` |
| `202` | 光强 | `1101` |
| `203` | JointState | `1104` |
| `204` | 运控 | `1102` |
| `205` | 速度 | `1103` |

请求示例：

```json
{
  "head": {
    "type": 1008,
    "time": 1757310526000,
    "src": 3
  },
  "data": {
    "target": 201,
    "enable": true,
    "freq": 0
  }
}
```

响应示例：

```json
{
  "head": {"type": 1008, "time": 1757310526001, "src": 1},
  "code": 0,
  "data": {"target": 201, "enable": true, "freq": 100}
}
```

## 12. 拍照 - 1009

### 12.1 请求字段

| 字段 | 含义 | 类型 | 说明 |
| --- | --- | --- | --- |
| `task_id` | 任务 ID | `uint32` | 由调用方自定义 |
| `device_id` | 设备 ID | `uint32` | `0` 前摄，`1` 后摄 |
| `trigger` | 是否触发拍照 | `bool` | 当前填写 `true`；缺省时 Gateway 按 `true` 处理 |

请求示例：

```json
{
  "head": {
    "type": 1009,
    "time": 1757310526050,
    "src": 3,
    "frames": 19
  },
  "data": {
    "task_id": 1,
    "device_id": 0,
    "trigger": true
  }
}
```

### 12.2 响应字段

| 字段 | 含义 | 类型 |
| --- | --- | --- |
| `task_id` | 任务 ID | `uint32` |
| `device_id` | 设备 ID | `uint32` |
| `error_code` | 错误码 | `uint32` |
| `reason` | 失败原因 | `string` |
| `error_message` | 错误信息 | `string`，兼容字段，当前与 `reason` 相同 |

响应示例：

```json
{
  "head": {
    "type": 1009,
    "time": 1757310526150,
    "src": 1,
    "frames": 19
  },
  "code": 0,
  "data": {
    "task_id": 1,
    "device_id": 0,
    "error_code": 0,
    "reason": "",
    "error_message": ""
  }
}
```

## 13. 控制权相关 - 1013 / 1015 / 1016 / 1017

所有已握手且不是 `observe_only` 的控制客户端都可以申请控制权。当前无 Owner 时可以申请空位；已有 Owner 时，仅优先级更高的客户端可以抢占。优先级为 APP `500`、第三方直连 `400`、自研遥控器 `300`、Agent `200`、SDK `100`。

### 13.1 接管控制权 - 1013

请求：

```json
{
  "head": {
    "type": 1013,
    "time": 1757310526100,
    "src": 3
  },
  "data": {}
}
```

响应：

```json
{
  "head": {
    "type": 1013,
    "time": 1757310526101,
    "src": 1
  },
    "code": 0,
  "data": {
    "error_code": 0,
    "reason": "",
    "control_meta": {"has_owner": true, "owner_src": 3, "priority": 100, "self_priority": 100, "self_is_owner": true, "owner_relation": "self", "can_takeover": true, "deny_code": null, "epoch": 2, "transition": "claim_unowned"}
  }
}
```

### 13.2 释放控制权 - 1015

请求示例：

```json
{
  "head": {"type": 1015, "time": 1757310526200, "src": 3, "frames": 20},
  "data": {}
}
```

响应示例：

```json
{
  "head": {"type": 1015, "time": 1757310526201, "src": 1, "frames": 20},
  "code": 0,
  "data": {
    "error_code": 0,
    "reason": "OK",
    "control_meta": {"has_owner": false, "owner_src": null, "priority": null, "self_priority": 100, "self_is_owner": false, "owner_relation": "none", "can_takeover": true, "deny_code": null, "epoch": 3, "transition": "release"}
  }
}
```

### 13.3 通知类消息

- `1016`：控制权被接管通知
- `1017`：控制权被释放通知

当前 master 主动释放控制权或连接断开时，gateway 都会向其余已握手客户端发送 `1017`；非 master 断开不发送。

`1016` 只发送给被抢占的旧 Owner。两种通知都会携带按接收会话生成的 `data.control_meta`；收到 `1017` 不表示当前会话已自动获得控制权，需要控制时仍应发送 `1013`。

`1016` 主动推送示例：

```json
{
  "head": {"type": 1016, "time": 1757310526250, "src": 1},
  "code": 0,
  "data": {
    "control_meta": {"has_owner": true, "owner_src": 2, "priority": 500, "self_priority": 100, "self_is_owner": false, "owner_relation": "other_src", "can_takeover": false, "deny_code": "LOWER_PRIORITY", "epoch": 4, "transition": "higher_priority_takeover"}
  }
}
```

`1017` 主动推送示例：

```json
{
  "head": {"type": 1017, "time": 1757310526251, "src": 1},
  "code": 0,
  "data": {
    "control_meta": {"has_owner": false, "owner_src": null, "priority": null, "self_priority": 100, "self_is_owner": false, "owner_relation": "none", "can_takeover": true, "deny_code": null, "epoch": 5, "transition": "release"}
  }
}
```

## 14. 摄像头码率修改 - 1019 target=805

### 14.1 与旧版差异

旧版常见写法为 `1018`，本版统一使用：

- `type = 1019`
- `target = 805`

### 14.2 请求字段

| 字段 | 含义 | 类型 | 说明 |
| --- | --- | --- | --- |
| `target` | 参数项编号 | `uint32` | 固定为 `805` |
| `params.camera_name` | 摄像头名称 | `string` | 如 `camera_front` |
| `params.camera_bps` | 码率 | `uint32` | 单位：bps |

请求示例：

```json
{
  "head": {
    "type": 1019,
    "time": 1757310526300,
    "src": 3
  },
  "data": {
    "target": 805,
    "params": {
      "camera_name": "camera_front",
      "camera_bps": 2000000
    }
  }
}
```

响应示例：

```json
{
  "head": {
    "type": 1019,
    "time": 1757310526301,
    "src": 1
  },
    "code": 0,
  "data": {
    "target": 805,
    "params": {
      "camera_name": "camera_front",
      "camera_bps": 1
    }
  }
}
```

说明：

- 返回中的 `camera_bps` 以设备实际返回为准

## 15. 外设电源设置 / 查询 - 1019 / 1020

### 15.1 支持项

| `target` | 含义 |
| --- | --- |
| `806` | M1 48V 外设电源 |
| `807` | M1 24V 外设电源 |
| `808` | M1 12V 外设电源 |
| `826` | L2 24V 外设电源 |
| `827` | L2 航插口 |

### 15.2 设置 - 1019

请求字段：


说明：`806`-`808` 为 M1 系列通道，`826`/`827` 为 L2 系列通道，报文格式相同（`params.enable: bool`）。通道实际可用性取决于机型硬件配置，设备对当前机型不支持的 `target` 返回 `1001`。
| 字段 | 含义 | 类型 |
| --- | --- | --- |
| `target` | 参数项编号 | `uint32` |
| `params.enable` | 开关状态 | `bool` |

请求示例：

```json
{
  "head": {
    "type": 1019,
    "time": 1757310526400,
    "src": 3
  },
  "data": {
    "target": 806,
    "params": {
      "enable": true
    }
  }
}
```

响应示例：

```json
{
  "head": {"type": 1019, "time": 1757310526401, "src": 1},
  "code": 0,
  "data": {"target": 806, "params": {"enable": true}}
}
```

### 15.3 查询 - 1020

请求示例：

```json
{
  "head": {
    "type": 1020,
    "time": 1757310526402,
    "src": 3
  },
  "data": {
    "target": 806
  }
}
```

响应示例：

```json
{
  "head": {
    "type": 1020,
    "time": 1757310526403,
    "src": 1
  },
    "code": 0,
  "data": {
    "target": 806,
    "params": {
      "enable": true
    }
  }
}
```

## 16. 状态切换 - 1025

### 16.1 支持的目标状态

| `target_state` | 含义 |
| --- | --- |
| `1` | Idle |
| `2` | Remote |

请求示例：

```json
{
  "head": {
    "type": 1025,
    "time": 1757310526450,
    "src": 3
  },
  "data": {
    "target_state": 2
  }
}
```

响应示例：

```json
{
  "head": {"type": 1025, "time": 1757310526451, "src": 1},
  "code": 1,
  "data": {"target_state": 2}
}
```

## 17. 任务协议 - 1026 / 1027 / 1028

### 17.1 支持的任务类型

| `task_type` | 含义 |
| --- | --- |
| `4` | Recharge |
| `5` | Undock |

### 17.2 启动任务 - 1026

```json
{
  "head": {
    "type": 1026,
    "time": 1757310526500,
    "src": 3
  },
  "data": {
    "task_type": 4
  }
}
```

立即响应示例：

```json
{
  "head": {"type": 1026, "time": 1757310526501, "src": 1},
  "code": 1,
  "data": {}
}
```

### 17.3 停止任务 - 1027

```json
{
  "head": {
    "type": 1027,
    "time": 1757310526501,
    "src": 3
  },
  "data": {
    "task_type": 4
  }
}
```

立即响应示例：

```json
{
  "head": {"type": 1027, "time": 1757310526502, "src": 1},
  "code": 1,
  "data": {}
}
```

### 17.4 任务状态 - 1028

`1028` 是机器全局任务状态：实时变化广播给所有已握手客户端，
新连接会收到当前活动任务的最新快照；任务发起端断线不会清除仍在运行的任务状态。

响应字段：

| 字段 | 含义 | 类型 |
| --- | --- | --- |
| `task_type` | 任务类型 | `uint32` |
| `status` | 任务状态 | `uint32` |
| `phase` | 任务子状态 | `string` |
| `error_code` | 错误码 | `uint32` |

`status` 取值：

| 取值 | 含义 |
| --- | --- |
| `0` | UNKNOWN |
| `1` | STARTING |
| `2` | RUNNING |
| `3` | SUCCESS |
| `4` | FAILURE |
| `5` | STOPPED |

`phase` 由 RobotManager 的 `current_task_state` 原样透传，仅当 RobotManager 当前大状态与 `task_type` 对应任务一致时有效；旧版本 RobotManager 未提供该字段、任务尚未进入具体阶段或大状态不匹配时为空字符串。当前已定义值：

| `task_type` | 任务 | `phase` | 中文说明 |
| --- | --- | --- | --- |
| `3` | 导航 | `navigation_preparing` | 导航准备中 |
| `3` | 导航 | `navigation_locating` | 定位中 |
| `3` | 导航 | `navigation_locating_successfully` | 定位成功，等待启动导航 |
| `3` | 导航 | `navigation_navigating` | 导航执行中 |
| `4` | 回充 | `recharge_preparing` | 回充准备中 |
| `4` | 回充 | `recharge_docking` | 对桩、进桩中 |
| `4` | 回充 | `recharge_charging` | 充电中 |
| `4` | 回充 | `recharge_completed` | 回充完成，进入满电保持 |
| `5` | 出桩 | `undock_preparing` | 出桩准备中 |
| `5` | 出桩 | `undock_undocking` | 出桩执行中 |
| `5` | 出桩 | `undock_completed` | 出桩完成 |
| `6` | UWB 跟随 | `check_uwb` | 检查 UWB 连接状态 |
| `6` | UWB 跟随 | `check_posture` | 检查机器人是否为站立姿态 |
| `6` | UWB 跟随 | `execute_follow` | UWB 跟随执行中 |
| `7` | 视觉追踪 | `check_posture` | 检查机器人是否为站立姿态 |
| `7` | 视觉追踪 | `execute_track` | 视觉追踪执行中 |

`task_type=2`（建图）和 `task_type=8`（充电桩标定）当前未定义子状态，`phase` 为空字符串。后续 RobotManager 可新增值，客户端应按字符串向前兼容。

主动推送示例：

```json
{
  "head": {"type": 1028, "time": 1757310526550, "src": 1},
  "code": 0,
  "data": {"task_type": 4, "status": 2, "phase": "recharge_docking", "error_code": 0}
}
```

## 18. 挥手 - 1050

用途：主动断开前发送。

请求示例：

```json
{
  "head": {
    "type": 1050,
    "time": 1757310526600,
    "src": 3
  },
  "data": {}
}
```

响应示例：

```json
{
  "head": {"type": 1050, "time": 1757310526601, "src": 1},
  "code": 0,
  "data": {}
}
```

## 19. 传感器上报 - 1100 ~ 1104

### 19.1 IMU - 1100

发送 `1008 target=201 enable=true` 后，ACK 返回 `enable=true, freq=100`。
Gateway 通过原生高频入口订阅 `/imu_shm_publisher/imu_central` 并推送 `1100`。

字段：

- `a_x`
- `a_y`
- `a_z`
- `g_x`
- `g_y`
- `g_z`
- `q_x`
- `q_y`
- `q_z`
- `q_w`

主动推送示例：

```json
{
  "head": {"type": 1100, "time": 1757310526700, "src": 1},
  "code": 0,
  "data": {
    "a_x": 0.01, "a_y": 0.02, "a_z": 9.81,
    "g_x": 0.001, "g_y": 0.002, "g_z": 0.003,
    "q_w": 1.0, "q_x": 0.0, "q_y": 0.0, "q_z": 0.0
  }
}
```

### 19.2 光强 - 1101

开启方式：`1008 target=202`，固定目标推送频率上限 `1Hz`；忽略请求 `freq`，ACK 返回 `1`

字段：

- `lux`

主动推送示例：

```json
{
  "head": {"type": 1101, "time": 1757310526800, "src": 1},
  "code": 0,
  "data": {"lux": 125.5}
}
```

### 19.3 运控 - 1102

开启方式：`1008 target=204`，固定目标推送频率上限 `50Hz`；忽略请求 `freq`，ACK 返回 `50`

字段：

- `quat`
- `gyro`
- `acc`
- `rpy`
- `position`
- `v_world`
- `v_body`
- `omega_body`
- `omega_world`
- `time_stamp`

主动推送示例：

```json
{
  "head": {"type": 1102, "time": 1757310526900, "src": 1},
  "code": 0,
  "data": {
    "quat": [1.0, 0.0, 0.0, 0.0],
    "gyro": [0.0, 0.0, 0.01],
    "acc": [0.0, 0.0, 9.81],
    "rpy": [0.0, 0.0, 0.1],
    "position": [1.0, 2.0, 0.0],
    "v_world": [0.5, 0.0, 0.0],
    "v_body": [0.5, 0.0, 0.0],
    "omega_body": [0.0, 0.0, 0.2],
    "omega_world": [0.0, 0.0, 0.2],
    "time_stamp": 1757310526.9
  }
}
```

### 19.4 速度 - 1103

开启方式：`1008 target=205`，默认目标推送频率 `10Hz`，可配置 `1~50Hz`

字段：

- `x`
- `y`
- `yaw`

主动推送示例：

```json
{
  "head": {"type": 1103, "time": 1757310527000, "src": 1},
  "code": 0,
  "data": {"x": 0.5, "y": 0.0, "yaw": 0.2}
}
```

### 19.5 JointState - 1104

开启方式：`1008 target=203`，固定目标推送频率上限 `100Hz`；忽略请求 `freq`，ACK 返回 `100`

字段：

- `names`
- `positions`
- `velocities`
- `efforts`

主动推送示例：

```json
{
  "head": {"type": 1104, "time": 1757310527100, "src": 1},
  "code": 0,
  "data": {
    "names": ["fl1", "fr1"],
    "positions": [0.1, 0.2],
    "velocities": [0.01, 0.02],
    "efforts": [1.2, 1.3]
  }
}
```

`1100~1104` 频率由 Gateway 按最小发送间隔限流，不是独立定时重采样。
实际客户端线速可能因 ROS 源频率、线程调度、序列化和网络负载低于目标值；发送拥塞时同类型待发送数据只保留最新值。

## 20. 充电桩配对 - 1022 / 1023 / 1024

### 20.1 连接或断开请求 - 1022

请求字段：

| 字段 | 含义 | 类型 | 说明 |
| --- | --- | --- | --- |
| `action` | 操作 | `string` | `connect` 或 `disconnect` |
| `address` | 充电桩 BLE MAC | `string` | `connect` 时必填，`disconnect` 时可为空 |

接收响应固定使用以下结构，成功和失败不增删字段：

```json
{
  "head": {"type": 1022, "time": 1761291951200, "src": 1, "frames": 29},
  "code": 1,
  "data": {
    "operation_id": "cM3vWJvCvwNvV9Hj",
    "accepted": true,
    "error_code": 0,
    "error_message": ""
  }
}
```

原有 `accepted/error_code/error_message` 字段保持不变，只新增 `operation_id`。
`accepted=true` 只表示 Gateway 已生成 token 并成功发布 ROS 请求，不代表桩节点执行成功。未受理时
`operation_id` 固定为空字符串；已有操作执行中时返回 `code=1101`、`error_code=4`。

### 20.2 当前状态查询 - 1023

请求 `data` 为空对象。返回当前 `/dock/state` 观测值，包括 `ble_address`、`name`、`dock_connected`、
`has_contact`、`charging_active`、补光灯状态、桩版本和错误信息。该协议只查询状态，不承担异步结果关联。

### 20.3 终态结果推送 - 1024

桩节点完成操作后，Gateway 按 ROS token 找到原请求会话，只向该会话推送一次：

```json
{
  "head": {"type": 1024, "time": 1761291951250, "src": 1, "frames": 31},
  "code": 0,
  "data": {
    "operation_id": "cM3vWJvCvwNvV9Hj",
    "success": true,
    "error_code": 0
  }
}
```

`1024.data` 始终固定为 `operation_id/success/error_code`。成功时 `error_code=0`，失败时必须非 0；
35 秒未收到结果时返回 `code=1203`、`success=false`、`error_code=9001`。未知、重复或过期 token 会被忽略。
ROS 结果 topic 当前暂定为 `/dock/select_device_result`，消息类型为
`robot_common_interface/msg/OperationResult`。

## 21. 使用建议

- 连接成功后，先完成 `1000` 握手，再开始发送业务消息
- 业务连接期间，持续发送 `1001` 心跳
- 需要持续状态时，结合 `1004`、`1005`、`1028` 和已启用的 `1101~1104` 使用
- 需要兼容旧版本时，优先检查本版“相对上一版的主要差异”
