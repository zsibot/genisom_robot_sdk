# Robot SDK — Data Types Reference

## Overview

This document describes all data types exposed by the Robot SDK public headers: enums, structs, and their fields.
They are defined in `robot_sdk/sdk_type.hpp` under the `robot_sdk` namespace.

The types fall into three practical categories:

| Category | Types | Purpose |
|:--|:--|:--|
| **Command parameters (input)** | `CameraBitrateCmd`, `LedCommand`, `PowerCtrlCfg`, `PosControlCmd` | Filled in by the caller to tell the robot what to do |
| **Acknowledgments (output)** | `TakeControlAck`, `PowerCtrlAck`, `LedAutoModeAck`, etc. | Returned via `IControlCallback`, reporting how the command went |
| **Reported data (push)** | `RobotState`, `ImuData`, `FaultData`, `TaskStateInfo`, etc. | Pushed continuously via `IDataCallback`, describing the robot's current state |

## Quick Navigation

| Group | Types |
|:--|:--|
| [Basic Types](#basic-types) | `TransportProtocol`, `DeviceType`, `DeviceInfo` |
| [Fault Types](#fault-types) | `FaultCode`, `FaultLevel`, `FaultData` |
| [Sensor & Reported Data](#sensor--reported-data) | `ImuData`, `LuxData`, `MotionData`, `SpeedData`, `JointStateData` |
| [Robot State](#robot-state) | `RobotState`, `Speed`, `BatteryData`, and related enums |
| [Control Ownership](#control-ownership) | `CtrlSource`, `TakeControlAck`, `ReleaseControlAck`, etc. |
| [Camera](#camera) | `CameraBitrateCmd`, etc. |
| [Tasks](#tasks) | `TaskType`, `TaskStatus`, `TaskStateInfo` |
| [Peripheral Power](#peripheral-power) | `PeripheralPower`, `PowerCtrlCfg`, `PowerCtrlAck` |
| [LED](#led) | `LedId`, `LedEffect`, `LedColor`, `LedCommand`, etc. |
| [Position Control](#position-control) | `PosControlCmd` |

---

## Basic Types

### TransportProtocol

```cpp
enum class TransportProtocol { WebSocket, Udp };
```

Transport protocol used by the SDK, selected in the `SDKClient` constructor.

| Value | Description | Default Port |
|:--|:--|:--|
| `Udp` | UDP protocol (default) | 8082 |
| `WebSocket` | WebSocket protocol | 8081 |

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

Robot-model enum. 

| Value | Model | Value | Model |
|:--|:--|:--|:--|
| `M1` | Medium dog, wheeled-foot | `M1_AIR` | Medium dog, wheeled-foot Air |
| `M1F` | Medium dog, point-foot | `M1F_AIR` | Medium dog, point-foot Air |
| `M1_PRO` | Medium dog, wheeled-foot LiDAR | `L2` | Small dog, wheeled sport |
| `M1F_PRO` | Medium dog, point-foot LiDAR | `L2_ULTRA` | Small dog, wheeled surround-view |
| `M1_ULTRA` | Medium dog, wheeled-foot surround-view | `L2F` | Small dog, point-foot sport |
| `M1F_ULTRA` | Medium dog, point-foot surround-view | `L2F_ULTRA` | Small dog, point-foot surround-view |
| `UNKNOWN` | Unrecognized, disconnected, or not reported by an older robot | | |

Use `DeviceTypeName()` to obtain a display name for logs, such as `M1-Pro` or
`L2F-Ultra`.

### DeviceInfo

```cpp
struct DeviceInfo {
  DeviceType device_type = DeviceType::UNKNOWN;
  std::string sn;
};
```

Device information returned by the handshake response, obtained via
`SDKClient::GetDeviceInfo()` (read from a local cache; no network request is
made). Before connecting or after disconnecting, `device_type` is
`DeviceType::UNKNOWN` and `sn` is empty.

```cpp
const auto info = client.GetDeviceInfo();
if (info.device_type == DeviceType::L2) {
    // Small-dog L2 wheeled sport model
}
std::cout << DeviceTypeName(info.device_type) << std::endl;
```

> Per-model API support is listed in the [API capability matrix](sdk_api_capability_en.md).

---

## Fault Types

### FaultCode

```cpp
enum class FaultCode
```

Fault codes that may occur in the robot system. Values start at `10` (except `Unknown`) and increase consecutively.

| Value | Integer | Description |
|:--|:--:|:--|
| `Unknown` | 0 | Unknown fault |
| `ActuatorDisabled` | 10 | Actuator disabled |
| `ActuatorEncoderError` | 11 | Actuator encoder error |
| `ActuatorOffline` | 12 | Actuator offline |
| `ActuatorOverVoltage` | 13 | Actuator overvoltage |
| `ActuatorOverheat` | 14 | Actuator overheating |
| `ActuatorTempWarn` | 15 | Actuator temperature warning |
| `ActuatorTimeout` | 16 | Actuator control timeout |
| `ActuatorUndervolt` | 17 | Actuator undervoltage |
| `PowerControlOverTemp` | 18 | Battery overheat warning |
| `PowerControlPowerEmpty` | 19 | Battery below 10% |
| `PowerControlPowerLow` | 20 | Battery below 20% (above 10%) |
| `PowerControlOffline` | 21 | Power control board MCU connection failed |
| `CANBroken` | 22 | CAN communication error |
| `RobotRemoteKeepAliveFailure` | 23 | Remote control disconnected |
| `SystemClockSanityError` | 24 | System time jump detected |
| `SystemRobotStatusError` | 25 | Robot status abnormal |
| `IMUConnectError` | 26 | IMU connection error |
| `IMUDataNotUpdated` | 27 | IMU data not updating |

---

### FaultLevel

```cpp
enum class FaultLevel
```

Fault severity, from most to least severe: `FatalError` > `Error` > `Warn`.

| Value | Integer | Description |
|:--|:--:|:--|
| `Unknown` | 0 | Unknown |
| `FatalError` | 1 | Fatal; the robot cannot keep working |
| `Error` | 2 | Error; some functions are affected |
| `Warn` | 3 | Warning; attention needed but not blocking |

---

### FaultData

```cpp
struct FaultData {
  FaultCode code;        // Fault code
  FaultLevel level;      // Fault level
  std::string message;   // Fault description text
};
using FaultDatas = std::vector<FaultData>;
```

A single fault record. `OnFaultData()` may carry multiple faults at once (`FaultDatas`)
and is reported only when a fault occurs.

---

## Sensor & Reported Data

The following streams must be enabled through their configuration APIs first
(except `RobotState` and `FaultData`), then the robot pushes them at a fixed frequency
via `IDataCallback`.

### ImuData

```cpp
struct ImuData {
  float acc_x, acc_y, acc_z;             // Accelerometer axes (m/s²)
  float gyro_x, gyro_y, gyro_z;          // Gyroscope axes (rad/s)
  float quat_x, quat_y, quat_z, quat_w;  // Attitude quaternion (x, y, z, w)
};
```

IMU (Inertial Measurement Unit) data. Configure the report rate with `SetImuConfig(freq)`
([0, 100] Hz, 0 disables); received via `OnImuData()`.

---

### LuxData

```cpp
struct LuxData {
  float lux;  // Illuminance (lux)
};
```

Illuminance data. Enabled with `SetLuxConfig(true)`, reported at a fixed **1 Hz** via `OnLuxData()`.

---

### MotionData

```cpp
struct MotionData {
  float quat[4];         // Attitude quaternion [w, x, y, z]
  float v_world[3];      // Velocity in world frame [x, y, z] (m/s)
  float position[3];     // Position in world frame [x, y, z] (m)
  float omega_world[3];  // Angular velocity in world frame [x, y, z] (rad/s)
  float v_body[3];       // Velocity in body frame [x, y, z] (m/s)
  float omega_body[3];   // Angular velocity in body frame [x, y, z] (rad/s)
  uint64_t time_stamp;   // Timestamp (ns)
};
```

Detailed motion-control data (odometry). Enabled with `SetMcConfig(true)`, reported at a fixed
**50 Hz** via `OnMcData()`. Suitable for state estimation, trajectory logging, and other
high-frequency processing.

---

### SpeedData

```cpp
struct SpeedData {
  float x;    // Forward/backward velocity (m/s)
  float y;    // Left/right lateral velocity (m/s)
  float yaw;  // Rotation velocity (rad/s)
};
```

Velocity data. Configure the report rate with `SetSpeedReportConfig(true, frequency)`
([1, 50] Hz); received via `OnSpeedData()`.

---

### JointStateData

```cpp
struct JointStateData {
  std::vector<std::string> names;       // Joint names
  std::vector<double> positions;        // Joint positions (rad)
  std::vector<double> velocities;       // Joint velocities (rad/s)
  std::vector<double> efforts;          // Joint torques (N·m)
};
```

Joint state data. Enabled with `SetJointStateConfig(true)`, reported at a fixed rate via
`OnJointStateData()`. The four arrays correspond element-by-element in `names` order.
See [Joint Naming Rules](#joint-naming-rules).

---

## Robot State

### RobotState

```cpp
struct RobotState
```

A comprehensive snapshot of the robot state, actively reported at **1 Hz** after connecting
(no configuration needed) and received via `OnRobotStateData()`. This is the main entry point
for "what is the robot doing right now".

| Member | Type | Description |
|:--|:--|:--|
| `head_angle` | `double` | Head angle (rad) |
| `front_fill_light` | `FillLightStatus` | Front fill light status |
| `back_fill_light` | `FillLightStatus` | Back fill light status |
| `auto_mode_light` | `bool` | Auto light mode enabled |
| `obstacle_avoidance` | `bool` | Obstacle avoidance enabled |
| `charging_pile_connected` | `bool` | Charging pile connected |
| `speed_level` | `SpeedLevel` | Current speed level |
| `software_emergency_status` | `EmergencyStatus` | Software e-stop status |
| `hardware_emergency_status` | `EmergencyStatus` | Hardware e-stop status |
| `head_direction` | `HeadDirection` | Current head/tail direction |
| `motion_status` | `MotionStatus` | Current motion (posture) state |
| `machine_status` | `MachineStatus` | Machine (task) state |
| `battery` | `BatteryData` | Battery data |
| `speed` | `Speed` | Current velocity |
| `mile_data` | `float` | Cumulative mileage (m) |
| `joint_temps` | `std::unordered_map<std::string, double>` | Joint temperatures (name → °C) |
| `control_source` | `CtrlSource` | Current control source |

> `motion_status` vs `machine_status`: the former describes the **body posture**
> (standing, lying, crawling...), the latter describes **what business the robot is running**
> (idle, remote control, recharging, navigating...). They are independent.

---

### Speed

```cpp
struct Speed {
  double line;         // Forward/backward velocity (m/s)
  double translation;  // Left/right lateral velocity (m/s)
  double angle;        // Rotation angular velocity (rad/s)
};
```

The type of `RobotState::speed`: the robot's current actual velocity.

---

### BatteryData

```cpp
struct BatteryData
```

Dual-battery information. Members ending in `1` / `2` correspond to battery 1 and battery 2.

| Member | Type | Description |
|:--|:--|:--|
| `power1` / `power2` | `float` | Charge level (0–100%) |
| `present1` / `present2` | `bool` | Battery present |
| `voltage1` / `voltage2` | `float` | Voltage (V) |
| `temperature1` / `temperature2` | `float` | Temperature (°C) |
| `current1` / `current2` | `float` | Current (A) |
| `power_supply_status1` / `power_supply_status2` | `PowerSupplyStatus` | Power supply status |

#### PowerSupplyStatus

| Value | Integer | Description |
|:--|:--:|:--|
| `UNKNOWN` | 0 | Unknown |
| `CHARGING` | 1 | Charging |
| `DISCHARGING` | 2 | Discharging |
| `FULL` | 4 | Fully charged |

---

### Joint Naming Rules

`RobotState::joint_temps` and `JointStateData::names` share the same joint naming.
For wheel-foot models:

| Name | Description |
|:--|:--|
| `fl1` – `fl4` | Front-left leg, joints 1–4 |
| `fr1` – `fr4` | Front-right leg, joints 1–4 |
| `bl1` – `bl4` | Back-left leg, joints 1–4 |
| `br1` – `br4` | Back-right leg, joints 1–4 |

Pattern: `f/b` = front/back, `l/r` = left/right; the number is the joint index from hip to foot.

---

### State Enums

#### FillLightStatus — Fill Light Status

| Value | Integer | Description |
|:--|:--:|:--|
| `FILL_LIGHT_STATUS_UNKNOWN` | 0 | Unknown |
| `FILL_LIGHT_STATUS_ON` | 1 | Fill light on |
| `FILL_LIGHT_STATUS_OFF` | 2 | Fill light off |

---

#### SpeedLevel — Speed Level

| Value | Integer | Description |
|:--|:--:|:--|
| `SPEED_LEVEL_UNKNOWN` | 0 | Unknown |
| `SPEED_LEVEL_SLOW` | 1 | Low speed |
| `SPEED_LEVEL_MEDIUM` | 2 | Medium speed |
| `SPEED_LEVEL_HIGH` | 3 | High speed |

The speed level determines how the normalized `[-1.0, 1.0]` arguments of `Move()` map to
actual velocity limits:

| Level | Forward (forward_back) | Lateral (left_right) | Rotation (yaw) |
|:--|:--|:--|:--|
| Low | ±1.0 m/s | ±0.5 m/s | ±1.5 rad/s |
| Medium | ±2.0 m/s | ±0.5 m/s while forward speed < 1 m/s, otherwise locked to 0 | ±1.5 rad/s while forward speed < 1 m/s, otherwise ±1.0 rad/s |
| High | ±3.0 m/s | ±0.5 m/s while forward speed < 1 m/s, otherwise locked to 0 | ±1.5 rad/s while forward speed < 1 m/s; ±1.0 rad/s below 2 m/s; ±0.5 rad/s at or above 2 m/s |

> Rule of thumb: the faster the robot moves forward, the less lateral and rotational motion
> is allowed — a built-in safety policy of the motion controller.

---

#### EmergencyStatus — E-stop Status

| Value | Integer | Description |
|:--|:--:|:--|
| `EMERGENCY_STATUS_UNKNOWN` | 0 | Unknown |
| `EMERGENCY_STATUS_RECOVER` | 1 | E-stop released |
| `EMERGENCY_STATUS_STOP` | 2 | E-stop triggered |

`RobotState` carries one of these for the software e-stop and one for the hardware e-stop.

---

#### HeadDirection — Head/Tail Direction

| Value | Integer | Description |
|:--|:--:|:--|
| `HEAD_DIRECTION_UNKNOWN` | 0 | Unknown |
| `HEAD_DIRECTION_HEAD` | 1 | The head points forward |
| `HEAD_DIRECTION_TAIL` | 2 | The tail points forward (after reversing head/tail) |

---

#### MotionStatus — Motion (Posture) State

| Value | Integer | Description | Related API |
|:--|:--:|:--|:--|
| `MOTION_STATUS_UNKNOWN` | 0 | Unknown | — |
| `MOTION_STATUS_STAND_UP` | 1 | Standing up (in progress) | `StandUp()` |
| `MOTION_STATUS_WALK` | 2 | Walk (ready to move) | `Move()` |
| `MOTION_STATUS_BALANCE_STAND` | 3 | Balance stand | `BalanceStandUp()` |
| `MOTION_STATUS_LIE_DOWN` | 4 | Lying down | `LieDown()` |
| `MOTION_STATUS_CRAWL` | 5 | Crawling | `Crawl()` |
| `MOTION_STATUS_CRAWL_WALK` | 6 | Crawl walking | `CrawlWalk()` |
| `MOTION_STATUS_LOCKED` | 7 | Locked | `Locked()` |
| `MOTION_STATUS_CLIMB` | 8 | Climbing a high platform | `Climb()` |
| `MOTION_STATUS_STAIR` | 9 | Stair-climbing | `Stair()` |
| `MOTION_STATUS_SLIM` | 10 | Slim (narrow passage) | `Slim()` |
| `MOTION_STATUS_GAIT` | 11 | Gait | `Gait()` |
| `MOTION_STATUS_DSB` | 12 | DSB | `DSB()` |
| `MOTION_STATUS_POS_CONTROL` | 13 | Position control | `PosControl()` |
| `MOTION_STATUS_SK_WALK` | 14 | SameKnee walk | `SkWalk()` |
| `MOTION_STATUS_SAND` | 15 | Sand posture | `Sand()` |

> Note: after the robot finishes standing up, the reported state is `MOTION_STATUS_WALK`
> ("standing and ready to walk"), not `MOTION_STATUS_STAND_UP`.

---

#### MachineStatus — Machine (Task) State

| Value | Integer | Description |
|:--|:--:|:--|
| `UNKNOWN` | 0 | Unknown |
| `IDLE` | 1 | Idle |
| `REMOTE` | 2 | Remote control |
| `OTA` | 3 | OTA upgrading |
| `RECHARGE` | 4 | Recharging |
| `MAPPING` | 5 | Mapping |
| `NAVIGATION` | 6 | Navigating |
| `SAFETY` | 7 | Safety protection |
| `SELFTEST` | 8 | Self-test |
| `SOFT_SHUTDOWN` | 9 | Soft shutdown |
| `SILENCE` | 10 | Silence (standby) |
| `FOLLOW` | 11 | Following |
| `TRACK` | 12 | Tracking |
| `UNDOCK` | 13 | Undocking |
| `DOCK_CALIBRATION` | 14 | Dock calibration |
| `ESTOP` | 15 | Emergency stop |
| `FALL` | 16 | Fallen |
| `LOCAL_REMOTE` | 17 | Local remote control |
| `LOW_LEVEL` | 18 | Low-level control |

`SwitchRemoteState()` / `SwitchIdleState()` switch between `REMOTE` and `IDLE`.
`REMOTE` and `LOCAL_REMOTE` are distinct states, reported as 2 and 17,
respectively.

---

#### CtrlSource — Control Source

| Value | Integer | Description |
|:--|:--:|:--|
| `CTRL_SOURCE_UNKNOWN` | 0 | Unknown |
| `CTRL_SOURCE_APP` | 1 | Controlled by the APP |
| `CTRL_SOURCE_SDK` | 2 | Controlled by the SDK |
| `CTRL_SOURCE_OTHER` | 3 | Controlled by another source |

Can be used to passively confirm who currently owns control. See
[Control Ownership](sdk_control_ownership_en.md).

---

## Control Ownership

### ControlLostInfo / ControlAvailableInfo

```cpp
struct ControlLostInfo {};
struct ControlAvailableInfo {};
```

Carriers for control-ownership events; currently empty structs — the event itself
(control lost / control available) is all the information. Received via
`OnControlLost()` / `OnControlAvailable()` respectively.

---

### TakeControlAck / ReleaseControlAck

```cpp
struct TakeControlAck {
  uint32_t error_code;  // 0 = success, non-zero = failure
  std::string reason;   // Failure reason
};
struct ReleaseControlAck { /* same fields */ };
```

Acknowledgment of an ownership request / release. The return values of `TakeControl()` /
`ReleaseControl()` only indicate whether the command was sent — **whether ownership was
actually acquired / released must be determined by `error_code` in this ACK**. See
[Control Ownership](sdk_control_ownership_en.md).

---

## Camera

### CameraBitrateCmd / CameraBitrateAck

```cpp
struct CameraBitrateCmd {
  std::string camera_name;  // "camera_front" / "camera_back"
  uint32_t camera_bps;      // Bitrate (bps), range 50000–100000000
};
struct CameraBitrateAck { /* same fields */ };
```

Camera bitrate configuration and its acknowledgment. The `camera_bps` in the ACK is the
actual value applied by the device.

---

## Tasks

### TaskType — Task Type

| Value | Integer | Description |
|:--|:--:|:--|
| `UNKNOWN` | 0 | Unknown |
| `SCAN_QR` | 1 | Scan QR code |
| `MAPPING` | 2 | Mapping |
| `NAV` | 3 | Navigation |
| `RECHARGING` | 4 | Recharging |
| `UNDOCK` | 5 | Undocking |
| `UWB_FOLLOW` | 6 | UWB follow |
| `VISUAL_TRACK` | 7 | Visual tracking |

---

### TaskStatus — Task Status

| Value | Integer | Description |
|:--|:--:|:--|
| `UNKNOWN` | 0 | Unknown |
| `STARTING` | 1 | Starting |
| `RUNNING` | 2 | Running |
| `SUCCESS` | 3 | Succeeded (final) |
| `FAILURE` | 4 | Failed (final) |
| `STOPPED` | 5 | Stopped (final) |

> The three final states (`SUCCESS` / `FAILURE` / `STOPPED`) mean the task has ended;
> no further updates for that task will arrive afterwards.

---

### TaskStateInfo

```cpp
struct TaskStateInfo {
  TaskType task_type;      // Task type
  TaskStatus task_status;  // Task status
  std::string phase;       // Task phase description (reserved)
  uint32_t error_code;     // 0 = success, non-zero = failure
};
```

Actively reported by the robot when a task's state changes; received via `OnTaskStateData()`.
Mainly used to monitor recharge and undock tasks — see
[Recharge & Undock Task Guide](sdk_recharge_task_en.md).

---

## Peripheral Power

### PeripheralPower

```cpp
enum class PeripheralPower
```

Peripheral power channels.

| Value | Integer | Description |
|:--|:--:|:--|
| `UNKNOWN` | 0 | Unknown |
| `M1_48V` | 1 | M1 48V peripheral power |
| `M1_24V` | 2 | M1 24V peripheral power |
| `M1_12V` | 3 | M1 12V peripheral power |

---

### PowerCtrlCfg / PowerCtrlAck

```cpp
struct PowerCtrlCfg {
  PeripheralPower power;  // Power channel
  bool enable;            // true = power on, false = power off
};
struct PowerCtrlAck { /* same fields */ };
```

Peripheral power control parameters and acknowledgment. For queries, only `power` needs
to be filled in; the `enable` field in the `OnGetPeriphPower()` ACK reports the channel's
actual current state.

---

## LED

### LedId — LED Group

| Value | Integer | Description |
|:--|:--:|:--|
| `UNKNOWN` | 0 | Unknown (invalid; the SDK rejects it) |
| `ALL` | 1 | All LEDs |
| `FRONT` | 2 | Front LEDs |
| `BACK` | 3 | Back LEDs |

---

### LedEffect — LED Effect

| Value | Protocol Value | Description |
|:--|:--|:--|
| `UNKNOWN` | — | Unknown (invalid; the SDK rejects it) |
| `OFF` | `off` | Off |
| `ON` | `on` | Solid on |
| `BREATH` | `breath` | Breathing |
| `BLINK` | `blink` | Blinking |
| `BLINK_TRANSIENT` | `blink_transient` | Transient blink |

---

### LedColor

```cpp
struct LedColor {
  uint8_t r, g, b, a;  // RGBA components, 0–255 each
};
```

LED color; `a` is the brightness/alpha component.

---

### LedCommand / LedCommandAck

```cpp
struct LedCommand {
  LedId id;              // LED group
  LedEffect effect;      // Effect
  LedColor color;        // Color
  uint32_t duration_ms;  // Effect period / blink interval (ms)
};
struct LedCommandAck { /* same fields */ };
```

LED effect command and acknowledgment. See the [LED Control Guide](sdk_led_control_en.md)
for full usage.

---

### LedAutoModeAck

```cpp
struct LedAutoModeAck {
  bool auto_mode;  // true = auto mode, false = manual mode
};
```

ACK for setting/querying the LED auto mode. In auto mode, effects are switched by the robot
according to its own state; in manual mode, they are controlled by `SetLedCommand()`.

---

## Position Control

### PosControlCmd

```cpp
struct PosControlCmd {
  float x, y, z;          // Target position (m)
  float roll, pitch, yaw; // Target orientation (rad)
};
```

Target body pose in position-control mode, used with `PosMove()` (enter the position-control
posture first via `PosControl()`).

---

## Related Documents

- [SDKClient API Reference](sdk_client_api_en.md) — Client interface details
- [Callback Reference](sdk_callback_en.md) — Callback definitions and usage constraints
- [Connection Guide](sdk_connection_en.md) — Connection parameters and states
- [Control Ownership](sdk_control_ownership_en.md) — SDK vs APP ownership rules
- [Recharge & Undock Task Guide](sdk_recharge_task_en.md) — Task state flow and usage tips
