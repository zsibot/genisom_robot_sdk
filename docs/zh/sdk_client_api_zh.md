# Robot SDK — `SDKClient` 类接口文档

## 概述

`SDKClient` 是机器人 SDK 的核心类，负责连接管理与全部命令下发，
覆盖运动姿态控制、速度控制、灯光、数据上报配置、控制权管理、相机、回充/离桩任务、外设电源与 LED 灯效等功能。

- 命名空间：`robot_sdk`，头文件：`robot_sdk/sdk_client.hpp`
- 各接口的机型支持情况见 [API 机型能力表](sdk_api_capability_zh.md)
- 回调接口（机器人主动上报与命令应答）见 [Callback 回调接口文档](sdk_callback_zh.md)

## 阅读本文档前必知的四件事

### 1. 同步与异步两种调用模式

除 `Connect`/`Disconnect` 外，所有命令接口都有两个公共参数：

| 参数名 | 类型 | 默认值 | 说明 |
|:--|:--|:--|:--|
| `timeout_ms` | `int` | `0` | `0`：异步模式；`> 0`：同步模式，等待发送完成的最长时间（毫秒） |
| `handler` | `WriteHandler` | 空回调 | 异步模式下的发送结果回调；同步模式不使用 |

- **异步模式**（`timeout_ms = 0`）：函数立即返回，返回值仅表示命令**是否进入发送流程**，发送结果通过 `handler` 通知。
- **同步模式**（`timeout_ms > 0`）：函数阻塞直到发送完成或超时，返回值即发送结果。

> **不要在回调函数中使用同步模式**。发送完成事件也依赖当前 I/O 线程，调用将无法完成并一直等待到超时，详见 [回调文档的线程模型](sdk_callback_zh.md)。

### 2. 返回值的两层语义："发出去了" ≠ "做成了"

- 接口返回的 `std::error_code` 只表示**命令是否成功发出**（第一层）。
- 命令的**业务结果**（第二层）通过 `IControlCallback` 的应答回调返回，
  例如 `TakeControl()` 是否成功要看 `OnTakeControlAck()` 的 `error_code`。
- 大部分姿态命令的应答只表示"机器人已收到"；动作是否完成需观察 `RobotState` / `OnTaskStateData()`。

### 3. 错误码机制

所有接口返回 `std::error_code`：成功时 `!ec` 为真（`ec.value() == 0`），失败时可用 `ec.message()` 查看描述。
完整定义见 [SDK 错误码文档](sdk_error_zh.md)。

常用标准错误码：

| 错误码 | 含义 |
|:--|:--|
| `std::errc::success` | 操作成功 |
| `std::errc::invalid_argument` | 参数无效 |
| `std::errc::not_connected` | 未连接 |
| `std::errc::timed_out` | 操作超时 |
| `std::errc::operation_canceled` | 操作被取消 |
| `std::errc::operation_in_progress` | 相同操作正在进行中 |

SDK 扩展错误码：

| 错误码 | 整数值 | 含义 |
|:--|:--:|:--|
| `robot_sdk::Errc::ShakeHandFailed` | 10000 | 握手失败 |
| `robot_sdk::Errc::ProtocolMismatch` | 10001 | 协议版本不匹配（常见于旧版本机器） |
| `robot_sdk::Errc::ControlledDenial` | 10002 | 控制被拒绝（常见于多客户端连接） |
| `robot_sdk::Errc::ConnectFailed` | 10003 | 连接失败 |
| `robot_sdk::Errc::UnsupportedDeviceOperation` | 10004 | 当前机型不支持该操作 |

### 4. 控制权前提

普通控制命令只有在 SDK 拥有控制权时才会被机器人执行；软件急停除外。
详见 [控制权说明](sdk_control_ownership_zh.md)。

## 接口总览

| 分组 | 接口 |
|:--|:--|
| [连接管理](#连接管理) | `Connect`、`Disconnect`、`IsConnected`、`GetConnectionState` |
| [回调设置](#回调设置) | `SetControlCallback`、`SetDataCallback` |
| [安全控制](#安全控制) | `SoftEmergencyStop` |
| [姿态控制](#姿态控制) | `StandUp`、`BalanceStandUp`、`LieDown`、`Stair`、`Crawl`、`CrawlWalk`、`Climb`、`Slim`、`Gait`、`DSB`、`PosControl`、`SkWalk`、`Sand`、`ReverseHeadTail`、`Locked` |
| [运动控制](#运动控制) | `Move`、`PosMove`、`Turn`、`ControlHead`、`HighLowStance`、`SetSpeed` |
| [灯光与感知开关](#灯光与感知开关) | `FrontLight`、`BackLight`、`AutoModeLight`、`ObstacleAvoidance` |
| [数据上报配置](#数据上报配置) | `SetImuConfig`、`SetLuxConfig`、`SetMcConfig`、`SetSpeedReportConfig`、`SetJointStateConfig` |
| [控制权管理](#控制权管理) | `TakeControl`、`ReleaseControl` |
| [相机](#相机) | `UpdateCameraBitrate` |
| [任务与状态切换](#任务与状态切换) | `StartRechargeTask`、`StopRechargeTask`、`StartUnDockTask`、`StopUnDockTask`、`SwitchRemoteState`、`SwitchIdleState` |
| [外设电源](#外设电源) | `SetPeriphPower`、`GetPeriphPower` |
| [LED 灯效](#led-灯效) | `SetLedAutoMode`、`GetLedAutoMode`、`SetLedCommand` |
| [版本与设备信息](#版本与设备信息) | `Version`、`ProtocolVersion`、`SystemVersion`、`GetDeviceInfo` |

## 类型定义

| 类型名 | 定义 | 说明 |
|:--|:--|:--|
| `ConnectHandler` | `std::function<void(const std::error_code&)>` | 连接完成回调 |
| `DisConnectHandler` | `std::function<void(const std::error_code&)>` | 断开完成回调 |
| `ErrorHandler` | `std::function<void(const std::error_code&)>` | SDK 内部错误回调（构造时传入） |
| `WriteHandler` | `std::function<void(const std::error_code&, std::size_t)>` | 命令发送完成回调 |

---

## 构造与析构

### 构造函数

```cpp
SDKClient(ErrorHandler error_callback = [](const std::error_code&) {},
          ConnectionConfig connection_config = ConnectionConfig(),
          TransportProtocol type = TransportProtocol::Udp)
```

**参数：**

| 参数名 | 类型 | 默认值 | 说明 |
|:--|:--|:--|:--|
| `error_callback` | `ErrorHandler` | 空回调 | SDK 内部通信异常回调 |
| `connection_config` | `ConnectionConfig` | 默认配置 | 连接超时、自动重连等配置，见 [连接配置文档](sdk_connection_zh.md) |
| `type` | `TransportProtocol` | `Udp` | 传输协议：`Udp`（端口 8082）或 `WebSocket`（端口 8081） |

### 析构函数

```cpp
~SDKClient()
```

自动停止 I/O 线程并断开连接、释放资源。无需手动调用 `Disconnect()`，但建议显式断开以获得确定性的退出行为。

---

## 连接管理

### Connect — 连接机器人

```cpp
std::error_code Connect(std::string ip, std::string port,
                        bool block = false,
                        ConnectHandler handler = [](const std::error_code&) {})
```

发起连接并完成握手。连接过程包含"建立传输层连接 → 握手协商"两个阶段，只有握手成功才算连接成功。

**参数：**

| 参数名 | 类型 | 默认值 | 说明 |
|:--|:--|:--|:--|
| `ip` | `std::string` | — | 机器人 IP 地址，如 `"192.168.234.1"` |
| `port` | `std::string` | — | 端口：UDP 默认 `"8082"`，WebSocket 默认 `"8081"` |
| `block` | `bool` | `false` | `false`：异步；`true`：同步（阻塞直到连接成功或失败） |
| `handler` | `ConnectHandler` | 空回调 | 异步模式下的连接结果回调；同步模式不使用 |

**返回值：**

- 异步模式：仅表示连接请求是否受理，结果以 `handler` 为准
- 同步模式：返回即最终结果
- 常见失败：`Errc::ShakeHandFailed`（握手失败）、`Errc::ProtocolMismatch`（协议不匹配）、`Errc::ControlledDenial`（控制被拒绝）、`std::errc::operation_in_progress`（正在连接中）

> 注意：连接成功 ≠ 拥有控制权。若 APP 已连接并控制机器人，SDK 只能作为观察方，详见 [控制权说明](sdk_control_ownership_zh.md)。

---

### Disconnect — 断开连接

```cpp
std::error_code Disconnect(bool block = false,
                           DisConnectHandler handler = [](const std::error_code&) {})
```

断开与机器人的连接（UDP 模式下会先发送挥手命令通知机器人）。

**参数：** 同 `Connect` 的 `block` / `handler`。

**返回值：** 异步模式表示断开请求是否受理；同步模式返回即结果。连接已断开时重复调用返回相应错误码。

---

### IsConnected — 是否已连接

```cpp
bool IsConnected() const
```

返回 `true` 表示当前处于 `ConnectionState::CONNECTED`（已完成握手）。

### GetConnectionState — 获取详细连接状态

```cpp
ConnectionState GetConnectionState() const
```

返回 `ConnectionState` 枚举（`DISCONNECTED` / `CONNECTING` / `HANDSHAKING` / `CONNECTED` / `DISCONNECTING` / `RECONNECTING`），
详见 [连接配置文档](sdk_connection_zh.md)。

---

## 回调设置

### SetControlCallback — 设置控制应答回调

```cpp
void SetControlCallback(std::shared_ptr<IControlCallback> control_callback)
```

注册控制命令的应答回调（机器人"已收到命令"的确认及业务结果），接口清单见 [回调文档](sdk_callback_zh.md)。

### SetDataCallback — 设置数据上报回调

```cpp
void SetDataCallback(std::shared_ptr<IDataCallback> data_callback)
```

注册机器人主动上报数据的回调（传感器、状态、故障、控制权事件等），接口清单见 [回调文档](sdk_callback_zh.md)。

> 建议在 `Connect()` 之前完成注册，避免遗漏连接初期的上报数据。

---

## 安全控制

### SoftEmergencyStop — 软件急停

```cpp
std::error_code SoftEmergencyStop(bool on, int timeout_ms = 0,
                                  WriteHandler handler = [](const std::error_code&, std::size_t) {})
```

触发或解除软件急停。触发后机器人不响应其他运动命令并将速度置 0。
**急停不受控制权限制**，观察方也可调用——安全逻辑无需等待获得控制权。

**参数：**

| 参数名 | 类型 | 说明 |
|:--|:--|:--|
| `on` | `bool` | `true`：触发急停；`false`：解除急停 |

**应答：** `OnSoftEmergencyStop(bool on)`。**状态确认：** `RobotState::software_emergency_status`。

---

## 姿态控制

以下接口均为姿态切换命令，参数只有公共的 `timeout_ms` / `handler`（见[同步与异步](#1-同步与异步两种调用模式)），
应答回调与命令一一对应（见 [回调文档](sdk_callback_zh.md)）。

```cpp
std::error_code StandUp(int timeout_ms = 0, WriteHandler handler = ...);        // 站立
std::error_code BalanceStandUp(int timeout_ms = 0, WriteHandler handler = ...); // 平衡站立
std::error_code LieDown(int timeout_ms = 0, WriteHandler handler = ...);        // 卧倒
std::error_code Stair(int timeout_ms = 0, WriteHandler handler = ...);          // 登阶姿态
std::error_code Crawl(int timeout_ms = 0, WriteHandler handler = ...);          // 匍匐
std::error_code CrawlWalk(int timeout_ms = 0, WriteHandler handler = ...);      // 匍匐行走
std::error_code Climb(int timeout_ms = 0, WriteHandler handler = ...);          // 爬高台
std::error_code Slim(int timeout_ms = 0, WriteHandler handler = ...);           // 瘦身过窄道
std::error_code Gait(int timeout_ms = 0, WriteHandler handler = ...);           // 步态
std::error_code DSB(int timeout_ms = 0, WriteHandler handler = ...);            // 过挡鼠板
std::error_code PosControl(int timeout_ms = 0, WriteHandler handler = ...);     // 切入位控姿态
std::error_code SkWalk(int timeout_ms = 0, WriteHandler handler = ...);         // 同膝行走
std::error_code Sand(int timeout_ms = 0, WriteHandler handler = ...);           // 沙地姿态
std::error_code ReverseHeadTail(int timeout_ms = 0, WriteHandler handler = ...);// 调转头尾
std::error_code Locked(int timeout_ms = 0, WriteHandler handler = ...);         // 锁定
```

要点说明：

- **StandUp**：站立过程中 `RobotState::motion_status` 为 `MOTION_STATUS_STAND_UP`，完成后变为 `MOTION_STATUS_WALK`。
- **Locked**：各关节保持当前位置不动；下发站立/卧倒/匍匐等其他姿态命令会自动解锁。
- **Sand**：底层协议动作为 `action/snow`，设备上报的 `snow` 状态在 SDK 中解析为 `MOTION_STATUS_SAND`。
- **机型限制**：`Slim`、`DSB`、`SkWalk`、`Sand` 不支持 L2 系列；`ReverseHeadTail` 不支持 L2 系列及 `DeviceType::M1_AIR`、`DeviceType::M1F_AIR`；`Gait` 不支持 L2F 点足系列（`DeviceType::L2F`、`DeviceType::L2F_ULTRA`）。
  在不支持的机型上调用时 SDK 不会发送命令，直接返回 `robot_sdk::Errc::UnsupportedDeviceOperation`。完整对照见 [API 机型能力表](sdk_api_capability_zh.md)。

```cpp
auto ec = client.SkWalk();
if (ec == robot_sdk::Errc::UnsupportedDeviceOperation) {
    // 当前机型不支持同膝行走
}
```

---

## 运动控制

### Move — 移动

```cpp
std::error_code Move(float left_right, float forward_back, float yaw,
                     int timeout_ms = 0,
                     WriteHandler handler = [](const std::error_code&, std::size_t) {})
```

通用模式下控制机器人移动。三个参数均为归一化比例值，实际速度由当前速度等级（`SetSpeed`）决定，
映射关系见 [SpeedLevel 限速表](sdk_type_zh.md#speedlevel--速度等级)。最新一条 Move 指令在机器人侧维持 1 秒，
持续运动需要周期发送（建议 10–20 Hz）。

**参数：**

| 参数名 | 类型 | 范围 | 说明 |
|:--|:--|:--|:--|
| `left_right` | `float` | [-1.0, 1.0] | 左右平移速度比例；正数左移，负数右移 |
| `forward_back` | `float` | [-1.0, 1.0] | 前后移动速度比例；正数前进，负数后退 |
| `yaw` | `float` | [-1.0, 1.0] | 旋转速度比例；正数左转，负数右转 |

**返回值：** 参数越界返回 `std::errc::invalid_argument`；其余见[错误码机制](#3-错误码机制)。
**应答：** 无对应应答回调，发送结果看 `WriteHandler`。

---

### PosMove — 位控移动

```cpp
std::error_code PosMove(PosControlCmd cmd, int timeout_ms = 0,
                        WriteHandler handler = [](const std::error_code&, std::size_t) {})
```

位控模式下设定机器人身体的目标位姿，需先通过 `PosControl()` 切入位控姿态。

**参数：**

| 参数名 | 类型 | 说明 |
|:--|:--|:--|
| `cmd` | `PosControlCmd` | 目标位姿：`x/y/z`（m）与 `roll/pitch/yaw`（rad），见 [PosControlCmd](sdk_type_zh.md#位控参数) |

---

### Turn — 机体翻滚

```cpp
std::error_code Turn(int direction, int timeout_ms = 0,
                     WriteHandler handler = [](const std::error_code&, std::size_t) {})
```

原地模式下控制机体左右翻滚。

**参数：**

| 参数名 | 类型 | 说明 |
|:--|:--|:--|
| `direction` | `int` | `0`：恢复；`1`：左翻滚；`2`：右翻滚（范围 [0, 2]） |

**返回值：** `direction` 越界返回 `std::errc::invalid_argument`。

---

### ControlHead — 控制头部

```cpp
std::error_code ControlHead(float left_right, float up_down, int timeout_ms = 0,
                            WriteHandler handler = [](const std::error_code&, std::size_t) {})
```

原地模式下控制机器人"左右探头"与"抬头/低头"。

**参数：**

| 参数名 | 类型 | 范围 | 说明 |
|:--|:--|:--|:--|
| `left_right` | `float` | [-1.0, 1.0] | 正数左探头，负数右探头 (rad/s) |
| `up_down` | `float` | [-1.0, 1.0] | 正数抬头，负数低头 (rad/s) |

**返回值：** 参数越界返回 `std::errc::invalid_argument`。

---

### HighLowStance — 高低站姿

```cpp
std::error_code HighLowStance(int stance, int timeout_ms = 0,
                              WriteHandler handler = [](const std::error_code&, std::size_t) {})
```

原地模式下调整身体高低站姿。

**参数：**

| 参数名 | 类型 | 说明 |
|:--|:--|:--|
| `stance` | `int` | `0`：恢复；`1`：高站姿；`2`：低站姿（范围 [0, 2]） |

---

### SetSpeed — 设置速度等级

```cpp
std::error_code SetSpeed(int speed_level, int timeout_ms = 0,
                         WriteHandler handler = [](const std::error_code&, std::size_t) {})
```

设置速度等级，影响通用模式下 `Move()` 的实际速度上限，默认为低速。
各等级下归一化参数到实际速度的映射见 [SpeedLevel 限速表](sdk_type_zh.md#speedlevel--速度等级)。

**参数：**

| 参数名 | 类型 | 说明 |
|:--|:--|:--|
| `speed_level` | `int` | `1`：低速；`2`：中速；`3`：高速（范围 [1, 3]） |

**返回值：** 档位越界返回 `std::errc::invalid_argument`。
**应答：** `OnSpeed(int speed_level)`；当前等级也可从 `RobotState::speed_level` 读取。

---

## 灯光与感知开关

```cpp
std::error_code FrontLight(bool on, int timeout_ms = 0, WriteHandler handler = ...);        // 前补光灯
std::error_code BackLight(bool on, int timeout_ms = 0, WriteHandler handler = ...);         // 后补光灯
std::error_code AutoModeLight(bool on, int timeout_ms = 0, WriteHandler handler = ...);     // 自动补光灯模式
std::error_code ObstacleAvoidance(bool on, int timeout_ms = 0, WriteHandler handler = ...); // 停障开关
```

**参数：**

| 参数名 | 类型 | 说明 |
|:--|:--|:--|
| `on` | `bool` | `true`：开启；`false`：关闭 |

要点说明：

- 手动调用 `FrontLight` / `BackLight` 后，自动补光模式会关闭。
- **机型限制**：`BackLight` 不支持 L2 系列及 `DeviceType::M1_AIR`、`DeviceType::M1F_AIR`；`ObstacleAvoidance` 不支持 L2 系列。调用返回 `Errc::UnsupportedDeviceOperation`。
- 当前状态可从 `RobotState` 的 `front_fill_light` / `back_fill_light` / `auto_mode_light` / `obstacle_avoidance` 读取。

---

## 数据上报配置

机器人数据默认只上报 `RobotState`（1 Hz）和故障信息，以下接口按需开启更多数据流：

```cpp
std::error_code SetImuConfig(int freq, int timeout_ms = 0, WriteHandler handler = ...);                 // IMU
std::error_code SetLuxConfig(bool on, int timeout_ms = 0, WriteHandler handler = ...);                  // 光强
std::error_code SetMcConfig(bool on, int timeout_ms = 0, WriteHandler handler = ...);                   // 运动数据
std::error_code SetSpeedReportConfig(bool on, uint32_t frequency, int timeout_ms = 0, WriteHandler handler = ...); // 速度
std::error_code SetJointStateConfig(bool on, int timeout_ms = 0, WriteHandler handler = ...);           // 关节状态
```

| 接口 | 参数 | 数据回调 | 上报频率 |
|:--|:--|:--|:--|
| `SetImuConfig` | `freq`：[0, 100]，0 表示关闭 | `OnImuData` | 按配置 |
| `SetLuxConfig` | `on`：开关 | `OnLuxData` | 固定 1 Hz |
| `SetMcConfig` | `on`：开关 | `OnMcData` | 固定 50 Hz |
| `SetSpeedReportConfig` | `on` + `frequency`：[1, 50] Hz | `OnSpeedData` | 按配置 |
| `SetJointStateConfig` | `on`：开关 | `OnJointStateData` | 定频 |

各配置均有对应的配置应答回调（`OnImuConfig` 等），数据结构定义见 [数据类型文档](sdk_type_zh.md)。

---

## 控制权管理

### TakeControl — 申请控制权

```cpp
std::error_code TakeControl(int timeout_ms = 0,
                            WriteHandler handler = [](const std::error_code&, std::size_t) {})
```

申请机器人的控制权。注意：

- 函数返回值只表示申请命令是否发出，**是否获得控制权以 `OnTakeControlAck()` 的 `error_code` 为准**（`0` 为成功，失败原因见 `reason`）。
- 若 APP 仍连接在机器人上，申请会失败——需等待收到 `OnControlAvailable()` 后再申请。
- 完整规则与推荐处理流程见 [控制权说明](sdk_control_ownership_zh.md)。

### ReleaseControl — 释放控制权

```cpp
std::error_code ReleaseControl(int timeout_ms = 0,
                               WriteHandler handler = [](const std::error_code&, std::size_t) {})
```

主动释放当前持有的控制权，结果以 `OnReleaseControlAck()` 为准。
建议在退出程序或不再需要控制时调用，让控制权尽快对其他客户端可用。

---

## 相机

### UpdateCameraBitrate — 更新摄像头码率

```cpp
std::error_code UpdateCameraBitrate(CameraBitrateCmd cmd, int timeout_ms = 0,
                                    WriteHandler handler = [](const std::error_code&, std::size_t) {})
```

**参数：**

| 参数名 | 类型 | 说明 |
|:--|:--|:--|
| `cmd` | `CameraBitrateCmd` | `camera_name`（`"camera_front"` / `"camera_back"`）+ `camera_bps`（50000–100000000 bps） |

**应答：** `OnUpdateCameraBitrateAck()`，其中 `camera_bps` 以设备实际生效值为准。

---

## 任务与状态切换

```cpp
std::error_code StartRechargeTask(int timeout_ms = 0, WriteHandler handler = ...);  // 启动回充
std::error_code StopRechargeTask(int timeout_ms = 0, WriteHandler handler = ...);   // 停止回充
std::error_code StartUnDockTask(int timeout_ms = 0, WriteHandler handler = ...);    // 启动离桩
std::error_code StopUnDockTask(int timeout_ms = 0, WriteHandler handler = ...);     // 停止离桩
std::error_code SwitchRemoteState(int timeout_ms = 0, WriteHandler handler = ...);  // 切到远程控制状态
std::error_code SwitchIdleState(int timeout_ms = 0, WriteHandler handler = ...);    // 切到空闲状态
```

要点说明：

- 任务类接口（回充/离桩）的"已进入/已退出"通过 `OnStartRechargeTask()` 等应答确认；
  **任务的过程状态与最终结果**通过 `OnTaskStateData()`（`TaskStateInfo`）上报。
  完整的状态流转与使用建议见 [充电与离桩任务使用说明](sdk_recharge_task_zh.md)。
- `SwitchRemoteState` / `SwitchIdleState` 切换的是 `MachineStatus`（`REMOTE` ↔ `IDLE`），
  应答为 `OnSwitchRemote()` / `OnSwitchIdle()`。

---

## 外设电源

### SetPeriphPower — 设置外设电源

```cpp
std::error_code SetPeriphPower(const PowerCtrlCfg& cfg, int timeout_ms = 0,
                               WriteHandler handler = [](const std::error_code&, std::size_t) {})
```

控制指定外设电源通道的通断。

**参数：**

| 参数名 | 类型 | 说明 |
|:--|:--|:--|
| `cfg` | `PowerCtrlCfg` | `power`：`M1_12V` / `M1_24V` / `M1_48V`；`enable`：`true` 上电 / `false` 断电 |

**返回值：** `power` 非法返回 `std::errc::invalid_argument`。
**应答：** `OnSetPeriphPower()` 回显下发的通道与状态。

### GetPeriphPower — 查询外设电源状态

```cpp
std::error_code GetPeriphPower(const PowerCtrlCfg& cfg, int timeout_ms = 0,
                               WriteHandler handler = [](const std::error_code&, std::size_t) {})
```

查询指定电源通道当前状态（`cfg` 中只需填 `power` 字段）。
**查询结果通过 `OnGetPeriphPower()` 返回**（`ack.enable` 为当前实际状态），函数本身不直接返回查询值。

---

## LED 灯效

```cpp
std::error_code SetLedAutoMode(bool auto_mode, int timeout_ms = 0, WriteHandler handler = ...);  // 设置自动/手动模式
std::error_code GetLedAutoMode(int timeout_ms = 0, WriteHandler handler = ...);                  // 查询自动/手动模式
std::error_code SetLedCommand(const LedCommand& cmd, int timeout_ms = 0, WriteHandler handler = ...); // 设置灯效
```

要点说明：

- **自动模式**下灯效由机器人按自身状态切换；**手动模式**下由 `SetLedCommand()` 控制。
- `SetLedCommand`：`cmd.id` 或 `cmd.effect` 为 `UNKNOWN` 时返回 `std::errc::invalid_argument`。
- 查询与设置结果分别通过 `OnGetLedAutoMode()` / `OnSetLedAutoMode()` / `OnSetLedCommand()` 返回。
- 灯效参数（分组、效果、颜色、周期）与完整示例见 [LED 灯效控制文档](sdk_led_control_zh.md)。

```cpp
LedCommand cmd{LedId::ALL, LedEffect::BLINK, {255, 128, 0, 255}, 300};
client.SetLedCommand(cmd);
```

---

## 版本与设备信息

### Version — SDK 版本号

```cpp
const std::string& Version() const
```

返回 SDK 自身的版本号字符串（如 `"1.2.0"`），任何时候可用。

### ProtocolVersion — 协议版本号

```cpp
const std::string& ProtocolVersion() const
```

返回 SDK 使用的通信协议版本号，任何时候可用。握手时机器人会校验该版本，
不匹配时连接失败并返回 `Errc::ProtocolMismatch`。

### SystemVersion — 机器人系统版本号

```cpp
const std::string& SystemVersion() const
```

返回机器人端系统版本号。**仅在握手成功后有效**，未连接时为空字符串。

### GetDeviceInfo — 获取设备信息

```cpp
DeviceInfo GetDeviceInfo() const
```

返回最近一次成功握手缓存的设备信息（本地读取，不发网络请求）：

```cpp
struct DeviceInfo {
  DeviceType device_type = DeviceType::UNKNOWN;
  std::string sn;
};
```

SDK 会将握手返回的协议机型转换为使用 M1/L2 产品名称的 `DeviceType` 枚举。连接成功前、断开后、
旧版机器人未返回机型或返回未知机型时，`device_type` 为 `DeviceType::UNKNOWN`；`sn` 为空字符串。
完整枚举见 [数据类型文档](sdk_type_zh.md#devicetype)。

```cpp
auto ec = client.Connect("192.168.234.1", "8082", true);
if (!ec) {
    const auto info = client.GetDeviceInfo();
    std::cout << "device_type=" << DeviceTypeName(info.device_type)
              << ", sn=" << info.sn << std::endl;
}
```

---

## 完整示例

```cpp
#include <iostream>
#include "robot_sdk/sdk_client.hpp"
using namespace robot_sdk;

int main() {
    SDKClient client([](const std::error_code& ec) {
        if (ec) std::cerr << "SDK Error: " << ec.message() << std::endl;
    });

    auto ec = client.Connect("192.168.234.1", "8082", true);  // 同步连接
    if (ec) {
        std::cerr << "Connect failed: " << ec.message() << std::endl;
        return -1;
    }

    client.StandUp(2000);        // 同步等待发送完成
    client.Move(0.0f, 0.5f, 0.0f);  // 异步发送：半速前进
    client.Disconnect(true);
    return 0;
}
```

更多示例见工程 `example/` 目录：`control.cpp`（运动控制）、`take_control.cpp`（控制权）、
`led.cpp`（灯效）、`recharge.cpp`（回充任务）等。

## 相关文档

- [Callback 回调接口](sdk_callback_zh.md) — 数据上报与命令应答
- [数据类型文档](sdk_type_zh.md) — 接口参数与状态结构体定义
- [连接配置文档](sdk_connection_zh.md) — 连接参数、超时与自动重连
- [SDK 错误码文档](sdk_error_zh.md) — 错误码定义与判断方法
- [控制权说明](sdk_control_ownership_zh.md) — SDK 与 APP 控制权归属
- [API 机型能力表](sdk_api_capability_zh.md) — 各机型接口支持情况
- [充电与离桩任务使用说明](sdk_recharge_task_zh.md) — 任务接口详细使用说明
