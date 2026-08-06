# SDK Callback Interface Documentation

This document defines the data callback and control command callback interfaces in the Robot SDK.  
Namespace: `robot_sdk`

---

## Interface Overview

- [`IDataCallback`](#idatacallback): Data reporting callback interface  
- [`IControlCallback`](#icontrolcallback): Control command callback interface (non-blocking mode)

---

## IDataCallback

Users can implement this interface as needed to receive data from the robot system.

> ⚠️ Callback functions must be **lightweight** and cannot perform time-consuming operations.  
> Inside the callback, only fast operations such as data copying and validation are recommended.  
> If time-consuming tasks are required, such as database writes, file I/O, complex calculations, or network transmission, please **copy the data to an independent thread or thread pool** for processing.

### Definition

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

  virtual ~IDataCallback() = default;
};
```

---

### Callback List

#### 🧭 `OnImuData`
- **Description**: IMU data callback, reported at the configured frequency after enabling IMU reporting.  
- **Parameter**: `const ImuData& data` — IMU data structure  
- **Call Frequency**: Configurable  

#### 💡 `OnLuxData`
- **Description**: Illuminance data callback, reported at a fixed frequency of 1 Hz after configuration.  
- **Parameter**: `const LuxData& data` — illuminance data structure  

#### 🤖 `OnMcData`
- **Description**: Motion control data callback, reported at a fixed frequency of 50 Hz after configuration.  
- **Parameter**: `const MotionData& data` — motion data structure  

#### 🚀 `OnSpeedData`
- **Description**: Robot speed data callback.  
- **Parameter**: `const SpeedData& data` — speed data structure  

#### 🦾 `OnJointStateData`
- **Description**: Joint state data callback, reported at the configured frequency after enabling it.  
- **Parameter**: `const JointStateData& data` — joint state data structure  

#### 🤖 `OnRobotStateData`
- **Description**: Robot state data callback, actively reported at 1 Hz.  
- **Parameter**: `const RobotState& data` — robot state structure  

#### ⚠️ `OnFaultData`
- **Description**: Fault information callback, actively reported when a fault occurs.  
- **Parameter**: `const FaultDatas& data` — collection of fault data  

#### 🔓 `OnControlLost`
- **Description**: Callback triggered when control ownership is lost.  
- **Parameter**: `const ControlLostInfo& info` — control ownership loss information  

#### 🔒 `OnControlAvailable`
- **Description**: Callback triggered when control ownership becomes available.  
- **Parameter**: `const ControlAvailableInfo& info` — control availability information  

#### 📋 `OnTaskStateData`
- **Description**: Task state callback, reported when the task state changes. It is mainly used for recharge, undock, and similar task workflows.  
- **Parameter**: `const TaskStateInfo& info` — task state information  

---

## IControlCallback

This interface is used to receive acknowledgments that the robot has received control commands in non-blocking mode.

### Definition

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
  virtual void OnDSB() {}
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
  virtual void OnTakeControlAck(const TakeControlAck& ack) {}
  virtual void OnReleaseControlAck(const ReleaseControlAck& ack) {}
  virtual void OnUpdateCameraBitrateAck(const CameraBitrateAck& ack) {}
  virtual void OnTakePhotoAck(const TakePhotoAck& ack) {}
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

---

### Callback List

| Callback Function | Description | Parameter |
|-----------|------|------|
| `OnSoftEmergencyStop(bool on)` | Acknowledgment that the soft emergency stop command was received | `on = true`: enable emergency stop; `on = false`: disable emergency stop |
| `OnStandUp()` | Acknowledgment that the stand up command was received | — |
| `OnBalanceStandUp()` | Acknowledgment that the balance stand up command was received | — |
| `OnLieDown()` | Acknowledgment that the lie down command was received | — |
| `OnStair()` | Acknowledgment that the stair mode command was received | — |
| `OnCrawl()` | Acknowledgment that the crawl mode command was received | — |
| `OnCrawlWalk()` | Acknowledgment that the crawl walk mode command was received | — |
| `OnClimb()` | Acknowledgment that the climb mode command was received | — |
| `OnSlim()` | Acknowledgment that the slim mode command was received | — |
| `OnGait()` | Acknowledgment that the gait mode command was received | — |
| `OnDSB()` | Acknowledgment that the DSB mode command was received | — |
| `OnPosControl()` | Acknowledgment that the position control mode command was received | — |
| `OnSkWalk()` | Acknowledgment that the SkWalk mode command was received | — |
| `OnSand()` | Acknowledgment that the sand posture command was received | — |
| `OnReverseHeadTail()` | Acknowledgment that the head-tail reverse command was received | — |
| `OnSpeed(int speed_level)` | Acknowledgment that the speed level switch command was received | Speed level |
| `OnLocked()` | Acknowledgment that the lock command was received | — |
| `OnFrontLight(bool on)` | Acknowledgment that the front fill light command was received | `true`: on; `false`: off |
| `OnBackLight(bool on)` | Acknowledgment that the rear fill light command was received | `true`: on; `false`: off |
| `OnAutoModeLight(bool on)` | Acknowledgment that the automatic fill light mode command was received | `true`: enable; `false`: disable |
| `OnObstacleAvoidance(bool on)` | Acknowledgment that the obstacle avoidance command was received | `true`: enable; `false`: disable |
| `OnLuxConfig(bool on)` | Acknowledgment that the illuminance reporting configuration command was received | `true`: enable; `false`: disable |
| `OnImuConfig(int freq)` | Acknowledgment that the IMU reporting configuration command was received | Frequency value |
| `OnMcConfig(bool on)` | Acknowledgment that the motion data reporting configuration command was received | `true`: enable; `false`: disable |
| `OnSpeedReportConfig(bool on, uint32_t frequency)` | Acknowledgment that the speed reporting configuration command was received | `true`: enable; `false`: disable; `frequency`: report frequency |
| `OnJointStateConfig(bool on)` | Acknowledgment that the joint state reporting configuration command was received | `true`: enable; `false`: disable |
| `OnTakeControlAck(const TakeControlAck& ack)` | Acknowledgment for the take-control command | Control acknowledgment information |
| `OnReleaseControlAck(const ReleaseControlAck& ack)` | Acknowledgment for the release-control command | Control acknowledgment information |
| `OnUpdateCameraBitrateAck(const CameraBitrateAck& ack)` | Acknowledgment for the camera bitrate update command | Camera bitrate acknowledgment information |
| `OnTakePhotoAck(const TakePhotoAck& ack)` | Acknowledgment for the take-photo command | Acknowledgment information including task ID, device ID, error code, and failure reason |
| `OnSwitchRemote()` | Notification that the robot has switched to remote control state | — |
| `OnSwitchIdle()` | Notification that the robot has switched to idle state | — |
| `OnStartRechargeTask()` | Notification that the robot has entered recharge mode | — |
| `OnStopRechargeTask()` | Notification that the robot has exited recharge mode | — |
| `OnStartUnDockTask()` | Notification that the robot has entered undock mode | — |
| `OnStopUnDockTask()` | Notification that the robot has exited undock mode | — |
| `OnSetPeriphPower(const PowerCtrlAck& ack)` | Acknowledgment for the peripheral power setting command | The `ack` contains the power channel and the target switch state |
| `OnGetPeriphPower(const PowerCtrlAck& ack)` | Acknowledgment for the peripheral power query command | The `ack` contains the current power channel state |
| `OnSetLedAutoMode(const LedAutoModeAck& ack)` | Acknowledgment for setting LED auto/manual mode | `ack.auto_mode` indicates the current mode |
| `OnGetLedAutoMode(const LedAutoModeAck& ack)` | Acknowledgment for querying LED auto/manual mode | `ack.auto_mode` indicates the current mode |
| `OnSetLedCommand(const LedCommandAck& ack)` | Acknowledgment for setting the LED effect | The `ack` contains LED group, effect, color, and period |

---

## Implementation Example

```cpp
#include "robot_sdk/sdk_callback.hpp"

class MyDataCallback : public robot_sdk::IDataCallback {
 public:
  void OnImuData(const ImuData& data) override {
    // Lightweight processing, for example caching
  }
};

class MyControlCallback : public robot_sdk::IControlCallback {
 public:
  void OnStandUp() override {
    // Handle stand up command acknowledgment
  }

  void OnSand() override {
    // Handle sand posture command acknowledgment
  }
};
```

---

## Notes

- Do not perform time-consuming operations inside callbacks.
- If longer processing is required, copy the data and hand it off to an independent thread.
