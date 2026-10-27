# Robot SDK — Callback Reference

## Overview

Communication between the SDK and the robot is bidirectional: your code calls APIs to **send commands**,
and the robot **pushes results and data** back through callbacks. The SDK provides two callback interfaces
with distinct responsibilities:

| Interface | Responsibility | Typical Content |
|:--|:--|:--|
| `IDataCallback` | Receives robot **pushes** and asynchronous final results | Sensor data, robot state, faults, ownership events, task state, dock scan results, and final connection results |
| `IControlCallback` | Receives command **acknowledgments (ACKs)** and query results | Confirmation that "the robot received your command", plus business or query results |

Both are abstract base classes with default empty implementations — override only the callbacks you care about,
then register them via `SDKClient::SetDataCallback()` / `SDKClient::SetControlCallback()`
(passed as `std::shared_ptr`; the SDK holds the reference internally).

## Two Callback Semantics: Notifications vs. ACKs

The key to understanding callbacks is distinguishing two kinds of messages:

- **Notifications and asynchronous final results (IDataCallback)**: data, events, or final results
  of long-running operations pushed by the robot, rather than command-receipt ACKs.
  For example, `OnRobotStateData()` keeps arriving at 1 Hz, while
  `OnDockConnectionResult()` arrives when a dock connect or disconnect operation finishes.
- **Acknowledgments (IControlCallback)**: the robot's receipt for a specific control command,
  meaning "command received". Some query results, such as `OnDockInfo()`, also use this interface.

> **Important:** most ACKs only mean the robot **received** the command — not that the action **finished**.
> When `OnStandUp()` arrives, the robot may still be in the process of standing up.
> To know whether an action completed, watch `RobotState::motion_status`;
> for task completion, watch `OnTaskStateData()`.
>
> A few ACKs carry real business results, such as `OnTakeControlAck()` (ownership request result)
> and `OnGetPeriphPower()` (the queried power state).

## Threading Model and Constraints (Must Read)

**All callbacks run on the SDK's internal I/O thread.** Please follow these rules:

1. **Keep callbacks lightweight**: only do quick work such as copying data or setting flags.
   For time-consuming work (database writes, file I/O, heavy computation, network requests),
   copy the data and hand it off to your own thread or thread pool.
2. **Never call blocking (synchronous) SDK APIs from within a callback**
   (forms with `timeout_ms > 0`, `Connect(..., true)`, `Disconnect(true)`):
   a synchronous call needs to wait for subsequent events on the I/O thread,
   but the I/O thread is busy running your callback, so the call cannot complete and waits until timeout.
   If you must trigger SDK operations from a callback, post them to another thread or use the async form.
3. **Beware of data races**: the callback thread runs concurrently with your main thread;
   protect shared data with locks or use atomics.
4. **Do not throw exceptions** out of a callback. The SDK catches `std::exception` and unknown exceptions,
   logs an error, and drops the current callback event to protect its internal I/O thread; your application
   remains responsible for recovery, alerting, and data compensation.

```cpp
// Recommended pattern: copy data / set flags in the callback, do business logic in your own thread
void OnImuData(const ImuData& data) override {
  std::lock_guard<std::mutex> lock(mtx_);
  latest_imu_ = data;          // quick copy
  imu_updated_ = true;         // set flag
}
```

---

## IDataCallback — Data Report Callbacks

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

### Sensor Data (must be enabled first)

| Callback | Description | Report Rate | Enable API |
|:--|:--|:--|:--|
| `OnImuData(const ImuData&)` | IMU data (accelerometer, gyroscope, quaternion) | Configured [0, 100] Hz | `SetImuConfig(freq)` |
| `OnLuxData(const LuxData&)` | Illuminance | Fixed 1 Hz | `SetLuxConfig(true)` |
| `OnMcData(const MotionData&)` | Motion data (odometry) | Fixed 50 Hz | `SetMcConfig(true)` |
| `OnSpeedData(const SpeedData&)` | Velocity data | Configured [1, 50] Hz | `SetSpeedReportConfig(true, freq)` |
| `OnJointStateData(const JointStateData&)` | Joint position/velocity/torque | Fixed rate | `SetJointStateConfig(true)` |

### State & Events (actively reported, no configuration needed)

| Callback | Description | Trigger |
|:--|:--|:--|
| `OnRobotStateData(const RobotState&)` | Comprehensive robot state snapshot | Continuously at 1 Hz |
| `OnFaultData(const FaultDatas&)` | Fault information (may contain multiple faults) | When a fault occurs |
| `OnTaskStateData(const TaskStateInfo&)` | Task state (recharge, undock, etc.) | When task state changes |

When a recharge or undock task fails, the error reason is reported in
`TaskStateInfo.error_code`; see [Recharge and Undock Task Error Codes](sdk_recharge_errorcode_en.md).
`TaskStateInfo.phase` provides finer recharge/undock progress and may be empty or
contain a value introduced by a later version. Do not use it instead of
`task_status` for terminal-state decisions. See
[Recharge & Undock Task Guide](sdk_recharge_task_en.md#phase--task-subphase).

### Charging Dock Scan and Final Connection Results

| Callback | Protocol ID | Trigger | Key Result |
|:--|:--:|:--|:--|
| `OnDockScanResult(const DockScanResult& result)` | `1018` | Passively pushed after scanning is enabled through `SetDockScanConfig(true)`; may be called repeatedly while scanning | `result.devices` contains the current device list; `device.address` identifies a dock and `device.connectable` indicates whether it can be connected |
| `OnDockConnectionResult(const DockConnectionResult& result)` | `1024` | When the asynchronous operation started by `ConnectDock()` or `DisconnectDock()` finishes | `result.success` is the final result; `result.error_code` provides the failure reason |

`OnDockConnectionResult()` is the final state of a connect or disconnect operation. The earlier `OnDockConnectionAck()` only reports whether the request was accepted. Start only one connect or disconnect operation at a time.

See the [Charging Dock Pairing API Guide](sdk_dock_pairing_en.md) for the complete workflow.

### Control Ownership Events

| Callback | Description | Trigger |
|:--|:--|:--|
| `OnControlLost(const ControlLostInfo&)` | Ownership was taken by another client (e.g. the APP) | When ownership is taken away |
| `OnControlAvailable(const ControlAvailableInfo&)` | Ownership became requestable | Usually when the controller disconnects |

> Handling ownership events correctly is the key to state-machine programming.
> See [Control Ownership](sdk_control_ownership_en.md).

---

## IControlCallback — Command Acknowledgments

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

### Motion & Posture Command ACKs

| Callback | Corresponding API | Parameter |
|:--|:--|:--|
| `OnSoftEmergencyStop(bool on)` | `SoftEmergencyStop(on)` | `true` = e-stop triggered; `false` = released |
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
| `OnSpeed(int speed_level)` | `SetSpeed(level)` | Applied speed level (1 low / 2 medium / 3 high) |
| `OnLocked()` | `Locked()` | — |

> Continuous-control commands such as `Move()`, `MoveVelocity()`, `Turn()`, `ControlHead()`, `HighLowStance()`,
> and `PosMove()` have **no ACK callbacks**; their result is only the `WriteHandler` send result.

### Light & Perception Switch ACKs

| Callback | Corresponding API | Parameter |
|:--|:--|:--|
| `OnFrontLight(bool on)` | `FrontLight(on)` | Echoes the requested switch state |
| `OnBackLight(bool on)` | `BackLight(on)` | Same as above |
| `OnAutoModeLight(bool on)` | `AutoModeLight(on)` | Same as above |
| `OnObstacleAvoidance(bool on)` | `ObstacleAvoidance(on)` | Same as above |

### Data Report Configuration ACKs

| Callback | Corresponding API | Parameter |
|:--|:--|:--|
| `OnImuConfig(int freq)` | `SetImuConfig(freq)` | Applied report rate |
| `OnLuxConfig(bool on)` | `SetLuxConfig(on)` | Switch state |
| `OnMcConfig(bool on)` | `SetMcConfig(on)` | Switch state |
| `OnSpeedReportConfig(bool on, uint32_t frequency)` | `SetSpeedReportConfig(on, freq)` | Switch state and applied rate |
| `OnJointStateConfig(bool on)` | `SetJointStateConfig(on)` | Switch state |

### Charging Dock ACKs and Query Results

| Callback | Corresponding API | Protocol ID | Description |
|:--|:--|:--:|:--|
| `OnDockScanConfig(bool on)` | `SetDockScanConfig(on)` | `1008 (target=302)` | Confirms that the scan setting has been applied; `on` is the current setting |
| `OnDockConnectionAck(const DockConnectionAck& ack)` | `ConnectDock(config)` / `DisconnectDock()` | `1022` | `ack.accepted` indicates whether the request was accepted; failures are described by `ack.error_code` and `ack.error_message`. This is not the final connection result |
| `OnDockInfo(const DockInfo& info)` | `GetDockInfo()` | `1023` | Returns charging dock information and its connection, contact, charging, and fill-light states; it may also be triggered by state changes |

After a connect or disconnect request is accepted, continue waiting for `IDataCallback::OnDockConnectionResult()` to obtain the final result. After an active query, read `info.dock_connected` in `OnDockInfo()` to confirm the connection state.

### Control Ownership ACKs (carry business results)

| Callback | Corresponding API | Description |
|:--|:--|:--|
| `OnTakeControlAck(const TakeControlAck& ack)` | `TakeControl()` | Ownership is acquired only when `ack.error_code == 0`; failure reason in `ack.reason` |
| `OnReleaseControlAck(const ReleaseControlAck& ack)` | `ReleaseControl()` | Release succeeded only when `ack.error_code == 0` |

### Camera ACKs (carry business results)

| Callback | Corresponding API | Description |
|:--|:--|:--|
| `OnUpdateCameraBitrateAck(const CameraBitrateAck& ack)` | `UpdateCameraBitrate(cmd)` | The bitrate actually applied by the device |

### State Switch & Task ACKs

| Callback | Corresponding API | Description |
|:--|:--|:--|
| `OnSwitchRemote()` | `SwitchRemoteState()` | The robot received the switch-to-remote instruction |
| `OnSwitchIdle()` | `SwitchIdleState()` | The robot received the switch-to-idle instruction |
| `OnStartRechargeTask()` | `StartRechargeTask()` | The robot entered the recharge task |
| `OnStopRechargeTask()` | `StopRechargeTask()` | The robot exited the recharge task |
| `OnStartUnDockTask()` | `StartUnDockTask()` | The robot entered the undock task |
| `OnStopUnDockTask()` | `StopUnDockTask()` | The robot exited the undock task |

### Peripheral Power & LED ACKs (carry business results)

| Callback | Corresponding API | Description |
|:--|:--|:--|
| `OnSetPeriphPower(const PowerCtrlAck& ack)` | `SetPeriphPower(cfg)` | Echoes the requested channel and switch state |
| `OnGetPeriphPower(const PowerCtrlAck& ack)` | `GetPeriphPower(cfg)` | `ack.enable` is the channel's actual current state |
| `OnSetLedAutoMode(const LedAutoModeAck& ack)` | `SetLedAutoMode(mode)` | The applied mode |
| `OnGetLedAutoMode(const LedAutoModeAck& ack)` | `GetLedAutoMode()` | The current mode |
| `OnSetLedCommand(const LedCommandAck& ack)` | `SetLedCommand(cmd)` | Echoes group, effect, color, and period |

---

## Implementation Example

```cpp
#include "robot_sdk/sdk_client.hpp"
using namespace robot_sdk;

class MyDataCallback : public IDataCallback {
 public:
  void OnRobotStateData(const RobotState& data) override {
    // Lightweight: copy what you need, update flags
    last_motion_status_ = data.motion_status;
  }

  void OnFaultData(const FaultDatas& data) override {
    for (const auto& fault : data) {
      // Just record it; don't do slow alerting work here
      fault_queue_.push(fault);
    }
  }

  void OnControlLost(const ControlLostInfo&) override {
    has_control_ = false;   // Stop sending normal control commands immediately
  }

  void OnControlAvailable(const ControlAvailableInfo&) override {
    control_available_ = true;  // Let the worker thread call TakeControl()
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
      has_control_ = true;    // Ownership is real only when the ACK says success
    } else {
      // ack.reason explains why it failed
    }
  }

  void OnStandUp() override {
    // The robot received the stand-up command; completion is tracked via
    // RobotState::motion_status
  }

 private:
  std::atomic<bool> has_control_{false};
};

// Registration
SDKClient client;
client.SetDataCallback(std::make_shared<MyDataCallback>());
client.SetControlCallback(std::make_shared<MyControlCallback>());
```

## Notes

- Callbacks run on the SDK I/O thread: **be lightweight, never block, never call synchronous SDK APIs,
  mind thread safety, and don't throw**.
- An ACK does not mean the action completed: check `RobotState` for motion states and
  `OnTaskStateData()` for task results.
- Whether an ACK is final depends on the callback. For example, `OnDockConnectionAck()` carries an `error_code` but only reports whether the request was accepted.
- Callback objects are registered via `shared_ptr`, and the SDK keeps its own
  reference. Callbacks can be safely replaced; pass `nullptr` to unregister one.
- Replacing or unregistering a callback does not interrupt an invocation that
  has already started. The SDK holds a `shared_ptr` snapshot until that invocation finishes.

## Related Documents

- [SDKClient API Reference](sdk_client_api_en.md) — Client interface details
- [Data Types Reference](sdk_type_en.md) — Struct definitions used by callback parameters
- [Control Ownership](sdk_control_ownership_en.md) — The ownership event handling pattern
- [Charging Dock Pairing API Guide](sdk_dock_pairing_en.md) — Scan, connect, query, and disconnect workflow
