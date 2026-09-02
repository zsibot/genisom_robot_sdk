# Robot SDK — 数据类型文档

## 概述

本文档描述 Robot SDK 公开头文件中的所有数据类型：枚举、结构体及其字段含义。
这些类型定义位于 `robot_sdk/sdk_type.hpp`，命名空间为 `robot_sdk`。

可以把这些类型分成三类来理解：

| 类别 | 包含类型 | 用途 |
|:--|:--|:--|
| **命令参数（入参）** | `CameraBitrateCmd`、`LedCommand`、`PowerCtrlCfg`、`PosControlCmd` | 调用 API 时由用户填写，告诉机器人"做什么" |
| **应答结果（出参）** | `TakeControlAck`、`PowerCtrlAck`、`LedAutoModeAck` 等 | 通过 `IControlCallback` 回调返回，告诉用户"做得怎么样" |
| **数据上报（推送）** | `RobotState`、`ImuData`、`FaultData`、`TaskStateInfo` 等 | 通过 `IDataCallback` 回调持续推送，反映机器人"当前的状态" |

## 快速导航

| 分组 | 类型 |
|:--|:--|
| [基础类型](#基础类型) | `TransportProtocol`、`DeviceType`、`DeviceInfo` |
| [故障类型](#故障类型) | `FaultCode`、`FaultLevel`、`FaultData` |
| [传感器与数据上报](#传感器与数据上报) | `ImuData`、`LuxData`、`MotionData`、`SpeedData`、`JointStateData` |
| [机器人综合状态](#机器人综合状态) | `RobotState`、`Speed`、`BatteryData` 及各状态枚举 |
| [控制权](#控制权) | `CtrlSource`、`TakeControlAck`、`ReleaseControlAck` 等 |
| [相机](#相机) | `CameraBitrateCmd` 等 |
| [任务](#任务) | `TaskType`、`TaskStatus`、`TaskStateInfo` |
| [外设电源](#外设电源) | `PeripheralPower`、`PowerCtrlCfg`、`PowerCtrlAck` |
| [LED 灯效](#led-灯效) | `LedId`、`LedEffect`、`LedColor`、`LedCommand` 等 |
| [位控参数](#位控参数) | `PosControlCmd` |

---

## 基础类型

### TransportProtocol

```cpp
enum class TransportProtocol { WebSocket, Udp };
```

SDK 支持的传输协议，在构造 `SDKClient` 时指定。

| 枚举值 | 说明 | 默认端口 |
|:--|:--|:--|
| `Udp` | UDP 协议（默认） | 8082 |
| `WebSocket` | WebSocket 协议 | 8081 |

---

### DeviceType

```cpp
enum class DeviceType : uint8_t {
  UNKNOWN = 0,
  M1,
  M1F,
  M1_PRO,
  M1F_PRO,
  M1_ULTRA,
  M1F_ULTRA,
  M1_AIR,
  M1F_AIR,
  L2,
  L2_ULTRA,
  L2F,
  L2F_ULTRA,
};
```

机器人机型枚举。

| 枚举值 | 机型 | 枚举值 | 机型 |
|:--|:--|:--|:--|
| `M1` | 中狗轮足 | `M1_AIR` | 中狗轮足 Air 版 |
| `M1F` | 中狗点足 | `M1F_AIR` | 中狗点足 Air 版 |
| `M1_PRO` | 中狗轮足激光版 | `L2` | 小狗轮式运动版 |
| `M1F_PRO` | 中狗点足激光版 | `L2_ULTRA` | 小狗轮式环视版 |
| `M1_ULTRA` | 中狗轮足环视版 | `L2F` | 小狗点足运动版 |
| `M1F_ULTRA` | 中狗点足环视版 | `L2F_ULTRA` | 小狗点足环视版 |
| `UNKNOWN` | 未识别、未连接或旧版机器人未返回机型 | | |

使用 `DeviceTypeName()` 可获得适合日志展示的名称，例如 `M1-Pro`、`L2F-Ultra`。

### DeviceInfo

```cpp
struct DeviceInfo {
  DeviceType device_type = DeviceType::UNKNOWN;
  std::string sn;
};
```

握手应答返回的设备信息，通过 `SDKClient::GetDeviceInfo()` 获取（本地缓存，不发网络请求）。
连接成功前或断开后，`device_type` 为 `DeviceType::UNKNOWN`，`sn` 为空字符串。

```cpp
const auto info = client.GetDeviceInfo();
if (info.device_type == DeviceType::L2) {
    // 小狗 L2 轮式运动版
}
std::cout << DeviceTypeName(info.device_type) << std::endl;
```

> 各机型支持的接口见 [API 机型能力表](sdk_api_capability_zh.md)。

---

## 故障类型

### FaultCode

```cpp
enum class FaultCode
```

机器人系统中可能出现的故障码。除 `Unknown` 外，数值从 `10` 开始连续递增。

| 枚举值 | 整数值 | 说明 |
|:--|:--:|:--|
| `Unknown` | 0 | 未知故障 |
| `ActuatorDisabled` | 10 | 执行器失能 |
| `ActuatorEncoderError` | 11 | 执行器编码器错误 |
| `ActuatorOffline` | 12 | 执行器掉线 |
| `ActuatorOverVoltage` | 13 | 执行器过压 |
| `ActuatorOverheat` | 14 | 执行器过热 |
| `ActuatorTempWarn` | 15 | 执行器温度预警 |
| `ActuatorTimeout` | 16 | 执行器控制超时 |
| `ActuatorUndervolt` | 17 | 执行器欠压 |
| `PowerControlOverTemp` | 18 | 电池过热预警 |
| `PowerControlPowerEmpty` | 19 | 电池电量低于 10% |
| `PowerControlPowerLow` | 20 | 电池电量低于 20%（高于 10%） |
| `PowerControlOffline` | 21 | 电源控制板 MCU 连接失败 |
| `CANBroken` | 22 | CAN 通信异常 |
| `RobotRemoteKeepAliveFailure` | 23 | 遥控器断链 |
| `SystemClockSanityError` | 24 | 系统时间跳变 |
| `SystemRobotStatusError` | 25 | 机器人状态异常 |
| `IMUConnectError` | 26 | IMU 连接异常 |
| `IMUDataNotUpdated` | 27 | IMU 数据不更新 |

---

### FaultLevel

```cpp
enum class FaultLevel
```

故障等级，严重程度从高到低为 `FatalError` > `Error` > `Warn`。

| 枚举值 | 整数值 | 说明 |
|:--|:--:|:--|
| `Unknown` | 0 | 未知 |
| `FatalError` | 1 | 致命错误，机器人无法继续正常工作 |
| `Error` | 2 | 一般错误，部分功能受影响 |
| `Warn` | 3 | 警告，需要关注但暂不影响运行 |

---

### FaultData

```cpp
struct FaultData {
  FaultCode code;        // 故障码
  FaultLevel level;      // 故障等级
  std::string message;   // 故障描述文本
};
using FaultDatas = std::vector<FaultData>;
```

单条故障信息。`OnFaultData()` 回调一次可能携带多条故障（`FaultDatas`），
仅在故障发生时主动上报。

---

## 传感器与数据上报

以下数据均需先通过对应的配置接口开启上报（`RobotState`、`FaultData` 除外），
然后由机器人按固定频率推送，经 `IDataCallback` 分发给用户。

### ImuData

```cpp
struct ImuData {
  float acc_x, acc_y, acc_z;             // 加速度计三轴 (m/s²)
  float gyro_x, gyro_y, gyro_z;          // 陀螺仪三轴 (rad/s)
  float quat_x, quat_y, quat_z, quat_w;  // 姿态四元数 (x, y, z, w)
};
```

IMU（惯性测量单元）数据。通过 `SetImuConfig(freq)` 配置上报频率（[0, 100] Hz，0 为关闭），
经 `OnImuData()` 回调上报。

---

### LuxData

```cpp
struct LuxData {
  float lux;  // 光照强度 (lux)
};
```

光照强度数据。通过 `SetLuxConfig(true)` 开启后以固定 **1 Hz** 上报，经 `OnLuxData()` 回调上报。

---

### MotionData

```cpp
struct MotionData {
  float quat[4];         // 姿态四元数 [w, x, y, z]
  float v_world[3];      // 世界坐标系速度 [x, y, z] (m/s)
  float position[3];     // 世界坐标系位置 [x, y, z] (m)
  float omega_world[3];  // 世界坐标系角速度 [x, y, z] (rad/s)
  float v_body[3];       // 机体坐标系速度 [x, y, z] (m/s)
  float omega_body[3];   // 机体坐标系角速度 [x, y, z] (rad/s)
  uint64_t time_stamp;   // 时间戳 (ns)
};
```

运动控制详细数据（里程计信息）。通过 `SetMcConfig(true)` 开启后以固定 **50 Hz** 上报，
经 `OnMcData()` 回调上报。适合用来做状态估计、轨迹记录等高频数据处理。

---

### SpeedData

```cpp
struct SpeedData {
  float x;    // 前后速度 (m/s)
  float y;    // 左右平移速度 (m/s)
  float yaw;  // 旋转角速度 (rad/s)
};
```

速度数据。通过 `SetSpeedReportConfig(true, frequency)` 配置上报频率（[1, 50] Hz），
经 `OnSpeedData()` 回调上报。

---

### JointStateData

```cpp
struct JointStateData {
  std::vector<std::string> names;       // 关节名称
  std::vector<double> positions;        // 关节位置 (rad)
  std::vector<double> velocities;       // 关节速度 (rad/s)
  std::vector<double> efforts;          // 关节力矩 (N·m)
};
```

关节状态数据。通过 `SetJointStateConfig(true)` 开启后定频上报，经 `OnJointStateData()` 回调上报。
四个数组按 `names` 顺序一一对应。关节命名规则见 [关节温度说明](#关节命名规则)。

---

## 机器人综合状态

### RobotState

```cpp
struct RobotState
```

机器人综合状态快照，连接后由机器人以 **1 Hz** 主动上报（无需配置），
经 `OnRobotStateData()` 回调接收。是了解机器人"当前在干什么"的主要入口。

| 成员 | 类型 | 说明 |
|:--|:--|:--|
| `head_angle` | `double` | 头部角度 (rad) |
| `front_fill_light` | `FillLightStatus` | 前补光灯状态 |
| `back_fill_light` | `FillLightStatus` | 后补光灯状态 |
| `auto_mode_light` | `bool` | 是否为自动灯光模式 |
| `obstacle_avoidance` | `bool` | 是否开启停障 |
| `charging_pile_connected` | `bool` | 是否连接充电桩 |
| `speed_level` | `SpeedLevel` | 当前速度等级 |
| `software_emergency_status` | `EmergencyStatus` | 软件急停状态 |
| `hardware_emergency_status` | `EmergencyStatus` | 硬件急停状态 |
| `head_direction` | `HeadDirection` | 当前头尾方向 |
| `motion_status` | `MotionStatus` | 当前运动（姿态）状态 |
| `machine_status` | `MachineStatus` | 机器运行（任务）状态 |
| `battery` | `BatteryData` | 电池信息 |
| `speed` | `Speed` | 当前速度 |
| `mile_data` | `float` | 累计里程 (m) |
| `joint_temps` | `std::unordered_map<std::string, double>` | 关节温度表（关节名 → 温度 °C） |
| `control_source` | `CtrlSource` | 当前控制来源 |

> `motion_status` 与 `machine_status` 的区别：前者描述**身体姿态**（站着、趴着、爬行…），
> 后者描述**机器在跑什么业务**（空闲、远程控制、回充、导航…）。两者相互独立。

---

### Speed

```cpp
struct Speed {
  double line;         // 前后速度 (m/s)
  double translation;  // 左右平移速度 (m/s)
  double angle;        // 转向角速度 (rad/s)
};
```

`RobotState::speed` 的类型，表示机器人当前实际速度。

---

### BatteryData

```cpp
struct BatteryData
```

双电池信息。字段以 `1` / `2` 结尾分别对应电池 1 和电池 2。

| 成员 | 类型 | 说明 |
|:--|:--|:--|
| `power1` / `power2` | `float` | 电量（百分比 0–100） |
| `present1` / `present2` | `bool` | 电池是否在位 |
| `voltage1` / `voltage2` | `float` | 电压 (V) |
| `temperature1` / `temperature2` | `float` | 温度 (°C) |
| `current1` / `current2` | `float` | 电流 (A) |
| `power_supply_status1` / `power_supply_status2` | `PowerSupplyStatus` | 电源状态 |

#### PowerSupplyStatus

| 枚举值 | 整数值 | 说明 |
|:--|:--:|:--|
| `UNKNOWN` | 0 | 电源状态未知 |
| `CHARGING` | 1 | 充电中 |
| `DISCHARGING` | 2 | 放电中 |
| `FULL` | 4 | 已充满 |

---

### 关节命名规则

`RobotState::joint_temps` 和 `JointStateData::names` 使用统一的关节命名。
以轮足机型为例：

| 名称 | 说明 |
|:--|:--|
| `fl1` – `fl4` | 左前腿 1–4 号关节 |
| `fr1` – `fr4` | 右前腿 1–4 号关节 |
| `bl1` – `bl4` | 左后腿 1–4 号关节 |
| `br1` – `br4` | 右后腿 1–4 号关节 |

命名规律：`f/b` = 前/后，`l/r` = 左/右，数字为从髋到足的关节序号。

---

### 状态枚举

#### FillLightStatus — 补光灯状态

| 枚举值 | 整数值 | 说明 |
|:--|:--:|:--|
| `FILL_LIGHT_STATUS_UNKNOWN` | 0 | 未知 |
| `FILL_LIGHT_STATUS_ON` | 1 | 补光灯开 |
| `FILL_LIGHT_STATUS_OFF` | 2 | 补光灯关 |

---

#### SpeedLevel — 速度等级

| 枚举值 | 整数值 | 说明 |
|:--|:--:|:--|
| `SPEED_LEVEL_UNKNOWN` | 0 | 未知 |
| `SPEED_LEVEL_SLOW` | 1 | 低速 |
| `SPEED_LEVEL_MEDIUM` | 2 | 中速 |
| `SPEED_LEVEL_HIGH` | 3 | 高速 |

速度等级决定 `Move()` 接口中归一化参数 `[-1.0, 1.0]` 映射到的实际速度上限：

| 等级 | 前后 (forward_back) | 左右 (left_right) | 旋转 (yaw) |
|:--|:--|:--|:--|
| 低速 | ±1.0 m/s | ±0.5 m/s | ±1.5 rad/s |
| 中速 | ±2.0 m/s | 前后速度 < 1 m/s 时 ±0.5 m/s，否则锁为 0 | 前后速度 < 1 m/s 时 ±1.5 rad/s，否则 ±1.0 rad/s |
| 高速 | ±3.0 m/s | 前后速度 < 1 m/s 时 ±0.5 m/s，否则锁为 0 | 前后速度 < 1 m/s 时 ±1.5 rad/s；< 2 m/s 时 ±1.0 rad/s；≥ 2 m/s 时 ±0.5 rad/s |

> 规律：跑得越快，允许同时横向移动和转向的幅度越小——这是运动控制的安全限速策略。

---

#### EmergencyStatus — 急停状态

| 枚举值 | 整数值 | 说明 |
|:--|:--:|:--|
| `EMERGENCY_STATUS_UNKNOWN` | 0 | 未知 |
| `EMERGENCY_STATUS_RECOVER` | 1 | 急停解除 |
| `EMERGENCY_STATUS_STOP` | 2 | 急停触发 |

`RobotState` 中软件急停与硬件急停各有一份该状态。

---

#### HeadDirection — 头尾方向

| 枚举值 | 整数值 | 说明 |
|:--|:--:|:--|
| `HEAD_DIRECTION_UNKNOWN` | 0 | 未知 |
| `HEAD_DIRECTION_HEAD` | 1 | 狗头方向为前 |
| `HEAD_DIRECTION_TAIL` | 2 | 狗尾方向为前（调转头尾后） |

---

#### MotionStatus — 运动（姿态）状态

| 枚举值 | 整数值 | 说明 | 对应控制接口 |
|:--|:--:|:--|:--|
| `MOTION_STATUS_UNKNOWN` | 0 | 未知 | — |
| `MOTION_STATUS_STAND_UP` | 1 | 站立过程中 | `StandUp()` |
| `MOTION_STATUS_WALK` | 2 | 行走（可移动状态） | `Move()` |
| `MOTION_STATUS_BALANCE_STAND` | 3 | 平衡站立 | `BalanceStandUp()` |
| `MOTION_STATUS_LIE_DOWN` | 4 | 卧倒 | `LieDown()` |
| `MOTION_STATUS_CRAWL` | 5 | 匍匐 | `Crawl()` |
| `MOTION_STATUS_CRAWL_WALK` | 6 | 匍匐行走 | `CrawlWalk()` |
| `MOTION_STATUS_LOCKED` | 7 | 锁定 | `Locked()` |
| `MOTION_STATUS_CLIMB` | 8 | 爬高台 | `Climb()` |
| `MOTION_STATUS_STAIR` | 9 | 登阶 | `Stair()` |
| `MOTION_STATUS_SLIM` | 10 | 瘦身（过窄道） | `Slim()` |
| `MOTION_STATUS_GAIT` | 11 | 步态 | `Gait()` |
| `MOTION_STATUS_DSB` | 12 | 过挡鼠板 | `DSB()` |
| `MOTION_STATUS_POS_CONTROL` | 13 | 位控 | `PosControl()` |
| `MOTION_STATUS_SK_WALK` | 14 | 同膝行走 | `SkWalk()` |
| `MOTION_STATUS_SAND` | 15 | 沙地姿态 | `Sand()` |

> 注意：站立完成后上报的状态是 `MOTION_STATUS_WALK`（表示"已站好、可以走"），
> 而不是停留在 `MOTION_STATUS_STAND_UP`。

---

#### MachineStatus — 机器运行（任务）状态

| 枚举值 | 整数值 | 说明 |
|:--|:--:|:--|
| `UNKNOWN` | 0 | 未知 |
| `IDLE` | 1 | 空闲 |
| `REMOTE` | 2 | 远程控制 |
| `OTA` | 3 | OTA 升级中 |
| `RECHARGE` | 4 | 回充中 |
| `MAPPING` | 5 | 建图中 |
| `NAVIGATION` | 6 | 导航中 |
| `SAFETY` | 7 | 安全保护状态 |
| `SELFTEST` | 8 | 自检中 |
| `SOFT_SHUTDOWN` | 9 | 软关机 |
| `SILENCE` | 10 | 静音（待机） |
| `FOLLOW` | 11 | 跟随中 |
| `TRACK` | 12 | 跟踪中 |
| `UNDOCK` | 13 | 离桩中 |
| `DOCK_CALIBRATION` | 14 | 充电桩标定 |
| `ESTOP` | 15 | 急停 |
| `FALL` | 16 | 摔倒 |
| `LOCAL_REMOTE` | 17 | 本地遥控 |
| `LOW_LEVEL` | 18 | 低层控制 |

`SwitchRemoteState()` / `SwitchIdleState()` 就是在 `REMOTE` 与 `IDLE` 之间切换。
`REMOTE` 与 `LOCAL_REMOTE` 是两个独立状态，分别对应上报值 2 和 17。

---

#### CtrlSource — 控制来源

| 枚举值 | 整数值 | 说明 |
|:--|:--:|:--|
| `CTRL_SOURCE_UNKNOWN` | 0 | 未知 |
| `CTRL_SOURCE_APP` | 1 | APP 在控制 |
| `CTRL_SOURCE_SDK` | 2 | SDK 在控制 |
| `CTRL_SOURCE_OTHER` | 3 | 其他来源在控制 |

可用于被动确认当前控制方归属，详见 [控制权说明](sdk_control_ownership_zh.md)。

---

## 控制权

### ControlLostInfo / ControlAvailableInfo

```cpp
struct ControlLostInfo {};
struct ControlAvailableInfo {};
```

控制权事件的通知载体，当前为空结构体——事件本身（控制权丢失 / 控制权可用）就是全部信息。
分别通过 `OnControlLost()` / `OnControlAvailable()` 回调接收。

---

### TakeControlAck / ReleaseControlAck

```cpp
struct TakeControlAck {
  uint32_t error_code;  // 0 = 成功，非 0 = 失败
  std::string reason;   // 失败原因描述
};
struct ReleaseControlAck { /* 字段相同 */ };
```

控制权申请 / 释放的应答。`TakeControl()` / `ReleaseControl()` 的函数返回值只代表命令是否发出，
**是否真正获得 / 释放控制权，必须以此应答中的 `error_code` 为准**。
详见 [控制权说明](sdk_control_ownership_zh.md)。

---

## 相机

### CameraBitrateCmd / CameraBitrateAck

```cpp
struct CameraBitrateCmd {
  std::string camera_name;  // "camera_front" 前摄 / "camera_back" 后摄
  uint32_t camera_bps;      // 码率 (bps)，范围 50000–100000000
};
struct CameraBitrateAck { /* 字段相同 */ };
```

摄像头码率配置参数与应答。应答中的 `camera_bps` 以设备实际返回值为准。

---

## 任务

### TaskType — 任务类型

| 枚举值 | 整数值 | 说明 |
|:--|:--:|:--|
| `UNKNOWN` | 0 | 未知 |
| `SCAN_QR` | 1 | 扫描二维码 |
| `MAPPING` | 2 | 建图 |
| `NAV` | 3 | 导航 |
| `RECHARGING` | 4 | 回充 |
| `UNDOCK` | 5 | 脱离充电桩 |
| `UWB_FOLLOW` | 6 | UWB 跟随 |
| `VISUAL_TRACK` | 7 | 视觉跟踪 |

---

### TaskStatus — 任务状态

| 枚举值 | 整数值 | 说明 |
|:--|:--:|:--|
| `UNKNOWN` | 0 | 未知 |
| `STARTING` | 1 | 启动中 |
| `RUNNING` | 2 | 运行中 |
| `SUCCESS` | 3 | 成功（终态） |
| `FAILURE` | 4 | 失败（终态） |
| `STOPPED` | 5 | 已停止（终态） |

> 三个终态（`SUCCESS` / `FAILURE` / `STOPPED`）表示任务已结束，之后不会再收到该任务的状态更新。

---

### TaskStateInfo

```cpp
struct TaskStateInfo {
  TaskType task_type;    // 任务类型
  TaskStatus task_status;  // 任务状态
  std::string phase;     // 任务阶段描述（预留）
  uint32_t error_code;   // 0 = 成功，非 0 = 失败
};
```

任务状态变化时由机器人主动上报，经 `OnTaskStateData()` 回调接收。
主要用于回充、离桩等任务的过程监控，详见 [充电与离桩任务使用说明](sdk_recharge_task_zh.md)。

---

## 外设电源

### PeripheralPower

```cpp
enum class PeripheralPower
```

外设电源通道。

| 枚举值 | 整数值 | 说明 |
|:--|:--:|:--|
| `UNKNOWN` | 0 | 未知 |
| `M1_48V` | 1 | M1 48V 外设电源 |
| `M1_24V` | 2 | M1 24V 外设电源 |
| `M1_12V` | 3 | M1 12V 外设电源 |

---

### PowerCtrlCfg / PowerCtrlAck

```cpp
struct PowerCtrlCfg {
  PeripheralPower power;  // 外设电源通道
  bool enable;            // true = 上电，false = 断电
};
struct PowerCtrlAck { /* 字段相同 */ };
```

外设电源控制参数与应答。查询时只需填写 `power` 字段；
`OnGetPeriphPower()` 应答中的 `enable` 表示该通道当前实际上电状态。

---

## LED 灯效

### LedId — 灯分组

| 枚举值 | 整数值 | 说明 |
|:--|:--:|:--|
| `UNKNOWN` | 0 | 未知（非法，SDK 会拒绝发送） |
| `ALL` | 1 | 全部 LED |
| `FRONT` | 2 | 前部 LED |
| `BACK` | 3 | 后部 LED |

---

### LedEffect — 灯效

| 枚举值 | 协议值 | 说明 |
|:--|:--|:--|
| `UNKNOWN` | — | 未知（非法，SDK 会拒绝发送） |
| `OFF` | `off` | 关闭 |
| `ON` | `on` | 常亮 |
| `BREATH` | `breath` | 呼吸 |
| `BLINK` | `blink` | 闪烁 |
| `BLINK_TRANSIENT` | `blink_transient` | 瞬时闪烁 |

---

### LedColor

```cpp
struct LedColor {
  uint8_t r, g, b, a;  // RGBA 分量，各 0–255
};
```

LED 颜色。`a` 为亮度/透明度分量。

---

### LedCommand / LedCommandAck

```cpp
struct LedCommand {
  LedId id;              // 灯分组
  LedEffect effect;      // 灯效
  LedColor color;        // 颜色
  uint32_t duration_ms;  // 灯效周期 / 闪烁间隔 (ms)
};
struct LedCommandAck { /* 字段相同 */ };
```

LED 灯效命令与应答。详细使用方式见 [LED 灯效控制文档](sdk_led_control_zh.md)。

---

### LedAutoModeAck

```cpp
struct LedAutoModeAck {
  bool auto_mode;  // true = 自动模式，false = 手动模式
};
```

LED 自动/手动模式的设置与查询应答。自动模式下灯效由机器人根据状态自行切换；
手动模式下由 `SetLedCommand()` 决定。

---

## 位控参数

### PosControlCmd

```cpp
struct PosControlCmd {
  float x, y, z;          // 目标位置 (m)
  float roll, pitch, yaw; // 目标姿态角 (rad)
};
```

位控模式下的目标位姿，配合 `PosMove()` 使用（需先通过 `PosControl()` 切入位控姿态）。

---

## 相关文档

- [SDKClient API 文档](sdk_client_api_zh.md) — 客户端接口详细说明
- [Callback 回调接口](sdk_callback_zh.md) — 回调接口定义与使用约束
- [连接配置文档](sdk_connection_zh.md) — 连接参数与连接状态说明
- [控制权说明](sdk_control_ownership_zh.md) — SDK 与 APP 控制权归属
- [充电与离桩任务使用说明](sdk_recharge_task_zh.md) — 任务状态流转与使用建议
