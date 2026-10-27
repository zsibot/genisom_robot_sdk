# Robot SDK — 回调接口文档

## 概述

SDK 与机器人之间的通信是双向的：用户调用接口**下发命令**，机器人则通过回调**推送结果和数据**。
SDK 提供两个回调接口，分工明确：

| 接口 | 职责 | 典型内容 |
|:--|:--|:--|
| `IDataCallback` | 接收机器人**推送**的数据与异步终态 | 传感器数据、机器人状态、故障、控制权事件、任务状态、充电桩扫描与连接终态 |
| `IControlCallback` | 接收控制命令的**应答（ACK）**与查询结果 | "机器人已收到你的命令"的确认，以及命令或查询携带的业务结果 |

两者都是纯虚基类（带默认空实现），用户按需重写关注的回调即可，然后分别通过
`SDKClient::SetDataCallback()` / `SetControlCallback()` 注册（`std::shared_ptr` 传递，SDK 内部持有该引用）。

## 回调的两种语义：通知 vs 应答

理解回调的关键是区分两类消息：

- **通知与异步终态（IDataCallback）**：机器人单向推送数据、事件或耗时操作的最终结果，
  不是命令接收确认。例如 `OnRobotStateData()` 以 1 Hz 持续到来，
  `OnDockConnectionResult()` 则在充电桩连接或断开操作结束时到来。
- **应答（IControlCallback）**：机器人对某条控制命令的回执，表示"命令已收到"。
  部分查询结果也通过该接口返回，例如 `OnDockInfo()`。

> **重要：** 大部分应答只表示"机器人**收到**了命令"，不代表动作**已完成**。
> 例如收到 `OnStandUp()` 时机器人可能还在站立过程中——判断动作是否完成，
> 应观察 `RobotState::motion_status`；判断任务是否完成，应观察 `OnTaskStateData()`。
>
> 少数应答携带真正的业务结果，如 `OnTakeControlAck()`（控制权申请结果）、
> `OnGetPeriphPower()`（查询到的电源状态）。

## 线程模型与使用约束（务必阅读）

**所有回调函数都在 SDK 内部的 I/O 线程上执行**，请遵守以下约束：

1. **必须轻量**：回调中只做数据拷贝、标志置位等快速操作。
   耗时操作（数据库写入、文件 I/O、复杂计算、网络请求）应把数据复制后投递到自己的线程/线程池处理。
2. **禁止在回调中调用 SDK 的同步（阻塞）接口**（`timeout_ms > 0` 的形式、`Connect(..., true)`、`Disconnect(true)`）：
   同步接口需要等待 I/O 线程上的后续事件才能完成，而 I/O 线程正被你的回调占用——调用无法完成，只能等待到超时。
   如果确实需要在回调中触发 SDK 操作，请投递到其他线程执行，或使用异步形式。
3. **注意数据竞争**：回调线程与用户主线程并发运行，共享数据需加锁或使用原子变量。
4. **不得抛出异常**：不要让任何异常越过回调边界。SDK 会捕获 `std::exception` 和未知异常、
   输出错误日志并丢弃当前回调事件，以保护内部 I/O 线程；但异常的业务恢复、告警和数据补偿仍应由应用自行处理。

```cpp
// 推荐写法：回调里只拷贝数据、置标志，业务逻辑在自己的线程里处理
void OnImuData(const ImuData& data) override {
  std::lock_guard<std::mutex> lock(mtx_);
  latest_imu_ = data;          // 快速拷贝
  imu_updated_ = true;         // 置标志
}
```

---

## IDataCallback — 数据上报回调

```cpp
class ROBOT_EXPORT_API IDataCallback {
 public:
  virtual void OnImuData(const ImuData& data) {}
  virtual void OnLuxData(const LuxData& data) {}
  virtual void OnMcData(const MotionData& data) {}
  virtual void OnSpeedData(const SpeedData& data) {}
  virtual void OnJointStateData(const JointStateData& data) {}
  virtual void OnRobotStateData(const RobotState& data) {}
  virtual void OnFaultData(const FaultDatas& data) {}
  virtual void OnControlLost(const ControlLostInfo& info) {}
  virtual void OnControlAvailable(const ControlAvailableInfo& info) {}
  virtual void OnTaskStateData(const TaskStateInfo& info) {}
  virtual void OnDockScanResult(const DockScanResult& result) {}
  virtual void OnDockConnectionResult(const DockConnectionResult& result) {}
  virtual ~IDataCallback() = default;
};
```

### 传感器数据（需先配置开启）

| 回调 | 说明 | 上报频率 | 开启方式 |
|:--|:--|:--|:--|
| `OnImuData(const ImuData&)` | IMU 数据（加速度、陀螺仪、四元数） | 按配置 [0, 100] Hz | `SetImuConfig(freq)` |
| `OnLuxData(const LuxData&)` | 光照强度 | 固定 1 Hz | `SetLuxConfig(true)` |
| `OnMcData(const MotionData&)` | 运动控制数据（里程计） | 固定 50 Hz | `SetMcConfig(true)` |
| `OnSpeedData(const SpeedData&)` | 速度数据 | 按配置 [1, 50] Hz | `SetSpeedReportConfig(true, freq)` |
| `OnJointStateData(const JointStateData&)` | 关节位置/速度/力矩 | 定频 | `SetJointStateConfig(true)` |

### 状态与事件（无需配置，主动上报）

| 回调 | 说明 | 触发时机 |
|:--|:--|:--|
| `OnRobotStateData(const RobotState&)` | 机器人综合状态快照 | 固定 1 Hz 持续上报 |
| `OnFaultData(const FaultDatas&)` | 故障信息（可一次多条） | 故障发生时 |
| `OnTaskStateData(const TaskStateInfo&)` | 任务状态（回充、离桩等） | 任务状态变化时 |

回充与出桩任务失败时，错误原因由该回调的 `TaskStateInfo.error_code` 上报；
完整对照见 [回充与出桩任务错误码](sdk_recharge_errorcode_zh.md)。
`TaskStateInfo.phase` 提供回充/出桩的细分进度，可能为空或出现后续版本新增值；
不能用它替代 `task_status` 判断终态。阶段值见
[充电与离桩任务使用说明](sdk_recharge_task_zh.md#phase--任务子阶段)。

### 充电桩扫描与连接终态

| 回调 | 协议号 | 触发时机 | 关键结果 |
|:--|:--:|:--|:--|
| `OnDockScanResult(const DockScanResult& result)` | `1018` | 调用 `SetDockScanConfig(true)` 开启扫描后，由机器人被动推送；扫描期间可能多次触发 | `result.devices` 为本次设备列表，按 `device.address` 标识设备；`device.connectable` 表示是否可连接 |
| `OnDockConnectionResult(const DockConnectionResult& result)` | `1024` | `ConnectDock()` 或 `DisconnectDock()` 的异步操作结束时 | `result.success` 为最终结果；失败原因见 `result.error_code` |

`OnDockConnectionResult()` 才是连接或断开操作的终态；此前收到的 `OnDockConnectionAck()` 只表示请求是否被接受。同一时间只应发起一个连接或断开操作。

完整使用流程见 [充电桩配对接口使用指南](sdk_dock_pairing_zh.md)。

### 控制权事件

| 回调 | 说明 | 触发时机 |
|:--|:--|:--|
| `OnControlLost(const ControlLostInfo&)` | 控制权被其他客户端（如 APP）接管 | 控制权被夺走时 |
| `OnControlAvailable(const ControlAvailableInfo&)` | 控制权变为可申请 | 通常是控制方断开时 |

> 控制权事件的正确处理方式是状态机编程的关键，详见 [控制权说明](sdk_control_ownership_zh.md)。

---

## IControlCallback — 控制命令应答

```cpp
class ROBOT_EXPORT_API IControlCallback {
 public:
  virtual void OnSoftEmergencyStop(bool on) {}
  virtual void OnStandUp() {}
  virtual void OnBalanceStandUp() {}
  virtual void OnLieDown() {}
  virtual void OnStair() {}
  virtual void OnCrawl() {}
  virtual void OnCrawlWalk() {}
  virtual void OnClimb() {}
  virtual void OnSlim() {}
  virtual void OnGait() {}
  virtual void OnWalk() {}
  virtual void OnPosControl() {}
  virtual void OnSkWalk() {}
  virtual void OnSand() {}
  virtual void OnReverseHeadTail() {}
  virtual void OnSpeed(int speed_level) {}
  virtual void OnLocked() {}
  virtual void OnFrontLight(bool on) {}
  virtual void OnBackLight(bool on) {}
  virtual void OnAutoModeLight(bool on) {}
  virtual void OnObstacleAvoidance(bool on) {}
  virtual void OnLuxConfig(bool on) {}
  virtual void OnImuConfig(int freq) {}
  virtual void OnMcConfig(bool on) {}
  virtual void OnSpeedReportConfig(bool on, uint32_t frequency) {}
  virtual void OnJointStateConfig(bool on) {}
  virtual void OnDockScanConfig(bool on) {}
  virtual void OnTakeControlAck(const TakeControlAck& ack) {}
  virtual void OnReleaseControlAck(const ReleaseControlAck& ack) {}
  virtual void OnUpdateCameraBitrateAck(const CameraBitrateAck& ack) {}
  virtual void OnDockConnectionAck(const DockConnectionAck& ack) {}
  virtual void OnDockInfo(const DockInfo& info) {}
  virtual void OnSwitchRemote() {}
  virtual void OnSwitchIdle() {}
  virtual void OnStartRechargeTask() {}
  virtual void OnStopRechargeTask() {}
  virtual void OnStartUnDockTask() {}
  virtual void OnStopUnDockTask() {}
  virtual void OnSetPeriphPower(const PowerCtrlAck& ack) {}
  virtual void OnGetPeriphPower(const PowerCtrlAck& ack) {}
  virtual void OnSetLedAutoMode(const LedAutoModeAck& ack) {}
  virtual void OnGetLedAutoMode(const LedAutoModeAck& ack) {}
  virtual void OnSetLedCommand(const LedCommandAck& ack) {}
  virtual ~IControlCallback() = default;
};
```

### 运动与姿态命令应答

| 回调 | 对应接口 | 参数说明 |
|:--|:--|:--|
| `OnSoftEmergencyStop(bool on)` | `SoftEmergencyStop(on)` | `true` = 急停已触发；`false` = 急停已解除 |
| `OnStandUp()` | `StandUp()` | — |
| `OnBalanceStandUp()` | `BalanceStandUp()` | — |
| `OnLieDown()` | `LieDown()` | — |
| `OnStair()` | `Stair()` | — |
| `OnCrawl()` | `Crawl()` | — |
| `OnCrawlWalk()` | `CrawlWalk()` | — |
| `OnClimb()` | `Climb()` | — |
| `OnSlim()` | `Slim()` | — |
| `OnGait()` | `Gait()` | — |
| `OnWalk()` | `Walk()` | — |
| `OnPosControl()` | `PosControl()` | — |
| `OnSkWalk()` | `SkWalk()` | — |
| `OnSand()` | `Sand()` | — |
| `OnReverseHeadTail()` | `ReverseHeadTail()` | — |
| `OnSpeed(int speed_level)` | `SetSpeed(level)` | 生效的速度档位（1 低 / 2 中 / 3 高） |
| `OnLocked()` | `Locked()` | — |

> `Move()`、`MoveVelocity()`、`Turn()`、`ControlHead()`、`HighLowStance()`、`PosMove()` 这类连续量控制命令
> **没有对应的应答回调**，其结果只看 `WriteHandler` 的发送结果。

### 灯光与感知开关应答

| 回调 | 对应接口 | 参数说明 |
|:--|:--|:--|
| `OnFrontLight(bool on)` | `FrontLight(on)` | 回显下发的开关状态 |
| `OnBackLight(bool on)` | `BackLight(on)` | 同上 |
| `OnAutoModeLight(bool on)` | `AutoModeLight(on)` | 同上 |
| `OnObstacleAvoidance(bool on)` | `ObstacleAvoidance(on)` | 同上 |

### 数据上报配置应答

| 回调 | 对应接口 | 参数说明 |
|:--|:--|:--|
| `OnImuConfig(int freq)` | `SetImuConfig(freq)` | 生效的上报频率 |
| `OnLuxConfig(bool on)` | `SetLuxConfig(on)` | 开关状态 |
| `OnMcConfig(bool on)` | `SetMcConfig(on)` | 开关状态 |
| `OnSpeedReportConfig(bool on, uint32_t frequency)` | `SetSpeedReportConfig(on, freq)` | 开关状态与生效频率 |
| `OnJointStateConfig(bool on)` | `SetJointStateConfig(on)` | 开关状态 |

### 充电桩应答与查询结果

| 回调 | 对应接口 | 协议号 | 说明 |
|:--|:--|:--:|:--|
| `OnDockScanConfig(bool on)` | `SetDockScanConfig(on)` | `1008（target=302）` | 确认扫描开关已经设置；`on` 为当前开关状态 |
| `OnDockConnectionAck(const DockConnectionAck& ack)` | `ConnectDock(config)` / `DisconnectDock()` | `1022` | `ack.accepted` 表示请求是否被接受；失败信息见 `ack.error_code` 和 `ack.error_message`。该回调不是连接终态 |
| `OnDockInfo(const DockInfo& info)` | `GetDockInfo()` | `1023` | 返回充电桩信息以及连接、接触、充电和补光灯状态；状态变化时也可能触发 |

连接或断开请求被接受后，继续等待 `IDataCallback::OnDockConnectionResult()` 获取终态。主动查询后通过 `OnDockInfo()` 读取 `info.dock_connected` 可再次确认连接状态。

### 控制权应答（携带业务结果）

| 回调 | 对应接口 | 说明 |
|:--|:--|:--|
| `OnTakeControlAck(const TakeControlAck& ack)` | `TakeControl()` | `ack.error_code == 0` 才表示获得控制权，失败原因见 `ack.reason` |
| `OnReleaseControlAck(const ReleaseControlAck& ack)` | `ReleaseControl()` | `ack.error_code == 0` 才表示释放成功 |

### 相机应答（携带业务结果）

| 回调 | 对应接口 | 说明 |
|:--|:--|:--|
| `OnUpdateCameraBitrateAck(const CameraBitrateAck& ack)` | `UpdateCameraBitrate(cmd)` | 设备实际生效的码率 |

### 状态切换与任务应答

| 回调 | 对应接口 | 说明 |
|:--|:--|:--|
| `OnSwitchRemote()` | `SwitchRemoteState()` | 机器人已收到切换远程状态的指令 |
| `OnSwitchIdle()` | `SwitchIdleState()` | 机器人已收到切换空闲状态的指令 |
| `OnStartRechargeTask()` | `StartRechargeTask()` | 机器人已进入回充任务 |
| `OnStopRechargeTask()` | `StopRechargeTask()` | 机器人已退出回充任务 |
| `OnStartUnDockTask()` | `StartUnDockTask()` | 机器人已进入离桩任务 |
| `OnStopUnDockTask()` | `StopUnDockTask()` | 机器人已退出离桩任务 |

### 外设电源与 LED 应答（携带业务结果）

| 回调 | 对应接口 | 说明 |
|:--|:--|:--|
| `OnSetPeriphPower(const PowerCtrlAck& ack)` | `SetPeriphPower(cfg)` | 回显下发的通道与开关状态 |
| `OnGetPeriphPower(const PowerCtrlAck& ack)` | `GetPeriphPower(cfg)` | `ack.enable` 为该通道当前实际状态 |
| `OnSetLedAutoMode(const LedAutoModeAck& ack)` | `SetLedAutoMode(mode)` | 生效的模式 |
| `OnGetLedAutoMode(const LedAutoModeAck& ack)` | `GetLedAutoMode()` | 当前模式 |
| `OnSetLedCommand(const LedCommandAck& ack)` | `SetLedCommand(cmd)` | 回显灯分组、灯效、颜色和周期 |

---

## 实现示例

```cpp
#include "robot_sdk/sdk_client.hpp"
using namespace robot_sdk;

class MyDataCallback : public IDataCallback {
 public:
  void OnRobotStateData(const RobotState& data) override {
    // 轻量处理：拷贝需要的数据、更新标志位
    last_motion_status_ = data.motion_status;
  }

  void OnFaultData(const FaultDatas& data) override {
    for (const auto& fault : data) {
      // 只做记录，不做耗时告警发送
      fault_queue_.push(fault);
    }
  }

  void OnControlLost(const ControlLostInfo&) override {
    has_control_ = false;   // 立即停发普通控制命令
  }

  void OnControlAvailable(const ControlAvailableInfo&) override {
    control_available_ = true;  // 通知工作线程去 TakeControl()
  }

 private:
  MotionStatus last_motion_status_ = MotionStatus::MOTION_STATUS_UNKNOWN;
  std::atomic<bool> has_control_{false};
  std::atomic<bool> control_available_{false};
};

class MyControlCallback : public IControlCallback {
 public:
  void OnTakeControlAck(const TakeControlAck& ack) override {
    if (ack.error_code == 0) {
      has_control_ = true;    // 只有 ACK 成功才算真正获得控制权
    } else {
      // ack.reason 里有失败原因
    }
  }

  void OnStandUp() override {
    // 机器人已收到站立命令；动作完成与否需观察 RobotState::motion_status
  }

 private:
  std::atomic<bool> has_control_{false};
};

// 注册
SDKClient client;
client.SetDataCallback(std::make_shared<MyDataCallback>());
client.SetControlCallback(std::make_shared<MyControlCallback>());
```

## 注意事项汇总

- 回调在 SDK I/O 线程执行：**要轻量、禁阻塞、禁同步 SDK 调用、注意线程安全、勿抛异常**。
- 应答 ≠ 动作完成：动作完成情况看 `RobotState`，任务完成情况看 `OnTaskStateData()`。
- 是否属于业务终态取决于具体回调；例如 `OnDockConnectionAck()` 即使携带 `error_code`，也只表示请求是否被接受。
- 回调对象通过 `shared_ptr` 注册，SDK 内部会持有引用；用户可以安全地替换回调，传入 `nullptr` 可取消注册。
- 替换或取消回调不会中断已经开始的调用；SDK 持有该次调用的 `shared_ptr` 快照，旧回调会在本次调用结束后再按引用计数释放。

## 相关文档

- [SDKClient API 文档](sdk_client_api_zh.md) — 客户端接口详细说明
- [数据类型文档](sdk_type_zh.md) — 回调参数涉及的结构体定义
- [控制权说明](sdk_control_ownership_zh.md) — 控制权事件的处理范式
- [充电桩配对接口使用指南](sdk_dock_pairing_zh.md) — 扫描、连接、查询与断开流程
