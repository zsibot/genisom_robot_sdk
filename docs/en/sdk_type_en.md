# Robot SDK — Data Type Documentation

## Overview

This document describes the data types, enumerations, and structure definitions used in Robot SDK.

---

## Namespace

```cpp
namespace robot_sdk
```

---

## Transport Protocol

### TransportProtocol

```cpp
enum class TransportProtocol
```

**Description:**  
Transport protocol types supported by the SDK.

| Enum Value | Description |
|:--|:--|
| `WebSocket` | WebSocket protocol (default recommended) |
| `Udp` | UDP protocol |

---

## Fault-Related

### FaultCode

```cpp
enum class FaultCode
```

**Description:**  
Possible fault codes in the robot system.

| Enum Value | Description |
|:--|:--|
| `Unknown` | Unknown exception |
| `ActuatorDisabled` | Actuator disabled |
| `ActuatorEncoderError` | Actuator encoder error |
| `ActuatorOffline` | Actuator offline |
| `ActuatorOverVoltage` | Actuator overvoltage |
| `ActuatorOverheat` | Actuator overheating |
| `ActuatorTempWarn` | Actuator overtemperature warning |
| `ActuatorTimeout` | Actuator control timeout |
| `ActuatorUndervolt` | Actuator undervoltage |
| `PowerControlOverTemp` | Single battery overtemperature warning |
| `PowerControlPowerEmpty` | Single battery below 10% |
| `PowerControlPowerLow` | Single battery below 20% and above 10% |
| `PowerControlOffline` | Power control board MCU connection failed |
| `CANBroken` | CAN communication error |
| `RobotRemoteKeepAliveFailure` | Remote controller disconnected |
| `SystemClockSanityError` | System time jump detected |
| `SystemRobotStatusError` | Robot status abnormal |
| `IMUConnectError` | IMU connection error |
| `IMUDataNotUpdated` | IMU data not updating |

---

### FaultLevel

```cpp
enum class FaultLevel
```

**Description:**  
Fault severity definition.

| Enum Value | Integer Value | Description |
|:--|:--|:--|
| `Unknown` | 0 | Unknown |
| `FatalError` | 1 | Fatal error |
| `Error` | 2 | General error |
| `Warn` | 3 | Warning |

---

## IMU Data

### ImuData

```cpp
struct ImuData
```

**Description:**  
IMU (Inertial Measurement Unit) data structure.

| Member | Type | Description |
|:--|:--|:--|
| `acc_x`, `acc_y`, `acc_z` | `float` | Accelerometer three-axis data (m/s²) |
| `gyro_x`, `gyro_y`, `gyro_z` | `float` | Gyroscope three-axis data (rad/s) |
| `quat_x`, `quat_y`, `quat_z`, `quat_w` | `float` | Quaternion |

---

## Fault Data

### FaultData

```cpp
struct FaultData
```

**Description:**  
Fault information structure.

| Member | Type | Description |
|:--|:--|:--|
| `code` | `FaultCode` | Fault code |
| `level` | `FaultLevel` | Fault level |
| `message` | `std::string` | Fault message text |

---

## Power-Related

### PowerSupplyStatus

```cpp
enum class PowerSupplyStatus
```

**Description:**  
Battery power supply status enumeration.

| Enum Value | Integer Value | Description |
|:--|:--|:--|
| `UNKNOWN` | 0 | Power supply status unknown |
| `CHARGING` | 1 | Charging |
| `DISCHARGING` | 2 | Discharging |
| `FULL` | 4 | Fully charged |

---

### BatteryData

```cpp
struct BatteryData
```

**Description:**  
Dual-battery data structure.

| Member | Type | Description |
|:--|:--|:--|
| `power1` | `float` | Battery 1 charge percentage (0-100) |
| `power2` | `float` | Battery 2 charge percentage (0-100) |
| `present1` | `bool` | Whether battery 1 is present |
| `present2` | `bool` | Whether battery 2 is present |
| `voltage1` | `float` | Battery 1 voltage (V) |
| `voltage2` | `float` | Battery 2 voltage (V) |
| `temperature1` | `float` | Battery 1 temperature (°C) |
| `temperature2` | `float` | Battery 2 temperature (°C) |
| `current1` | `float` | Battery 1 current (A) |
| `current2` | `float` | Battery 2 current (A) |
| `power_supply_status1` | `PowerSupplyStatus` | Battery 1 power supply status |
| `power_supply_status2` | `PowerSupplyStatus` | Battery 2 power supply status |

---

## Sensor Data

### LuxData

```cpp
struct LuxData
```

**Description:**  
Illuminance sensor data.

| Member | Type | Description |
|:--|:--|:--|
| `lux` | `float` | Illuminance value |

---

## Robot State Enumerations

### FillLightStatus

```cpp
enum class FillLightStatus
```

**Description:**  
Fill light status enumeration.

| Enum Value | Integer Value | Description |
|:--|:--|:--|
| `FILL_LIGHT_STATUS_UNKNOWN` | 0 | Unknown state |
| `FILL_LIGHT_STATUS_ON` | 1 | Fill light on |
| `FILL_LIGHT_STATUS_OFF` | 2 | Fill light off |

---

### SpeedLevel

```cpp
enum class SpeedLevel
```

**Description:**  
Speed level enumeration. The selected level affects the speed limits of the
`Move` command.

| Enum Value | Integer Value | Description |
|:--|:--|:--|
| `SPEED_LEVEL_UNKNOWN` | 0 | Unknown |
| `SPEED_LEVEL_SLOW` | 1 | Low speed |
| `SPEED_LEVEL_MEDIUM` | 2 | Medium speed |
| `SPEED_LEVEL_HIGH` | 3 | High speed |

**Speed limits:**

| Level | vx (forward/backward) | vy (left/right) | vyaw (rotation) |
|:--|:--|:--|:--|
| Low | -0.5 to +0.5 m/s | -1.0 to +1.0 m/s | -2.0 to +2.0 rad/s |
| Medium | 0 m/s | -2.0 to +2.0 m/s | -1.5 to +1.5 rad/s |
| High | 0 m/s | -3.0 to +3.0 m/s | -1.0 to +1.0 rad/s |

---

### EmergencyStatus

```cpp
enum class EmergencyStatus
```

**Description:** Emergency-stop status enumeration.

| Enum Value | Integer Value | Description |
|:--|:--|:--|
| `EMERGENCY_STATUS_UNKNOWN` | 0 | Unknown |
| `EMERGENCY_STATUS_RECOVER` | 1 | Emergency stop released |
| `EMERGENCY_STATUS_STOP` | 2 | Emergency stop activated |

---

### HeadDirection

```cpp
enum class HeadDirection
```

**Description:** Robot head/tail direction enumeration.

| Enum Value | Integer Value | Description |
|:--|:--|:--|
| `HEAD_DIRECTION_UNKNOWN` | 0 | Unknown |
| `HEAD_DIRECTION_HEAD` | 1 | Head faces forward |
| `HEAD_DIRECTION_TAIL` | 2 | Tail faces forward |

---

### MotionStatus

```cpp
enum class MotionStatus
```

**Description:** Robot motion status enumeration.

| Enum Value | Integer Value | Description |
|:--|:--|:--|
| `MOTION_STATUS_UNKNOWN` | 0 | Unknown |
| `MOTION_STATUS_STAND_UP` | 1 | Standing up |
| `MOTION_STATUS_WALK` | 2 | Walking |
| `MOTION_STATUS_BALANCE_STAND` | 3 | Balance standing |
| `MOTION_STATUS_LIE_DOWN` | 4 | Lying down |
| `MOTION_STATUS_CRAWL` | 5 | Crawling posture |
| `MOTION_STATUS_CRAWL_WALK` | 6 | Crawl walking |
| `MOTION_STATUS_LOCKED` | 7 | Locked |
| `MOTION_STATUS_CLIMB` | 8 | Climbing a high platform |
| `MOTION_STATUS_STAIR` | 9 | Stair-climbing mode |
| `MOTION_STATUS_SLIM` | 10 | Narrow-passage mode |
| `MOTION_STATUS_GAIT` | 11 | Gait mode |
| `MOTION_STATUS_DSB` | 12 | DSB mode |
| `MOTION_STATUS_POS_CONTROL` | 13 | Position-control mode |
| `MOTION_STATUS_SK_WALK` | 14 | Same-knee walking mode |
| `MOTION_STATUS_SAND` | 15 | Sand posture |

---

## Control-Related

### CtrlSource

```cpp
enum class CtrlSource
```

**Description:**  
Robot control source enumeration.

| Enum Value | Integer Value | Description |
|:--|:--|:--|
| `CTRL_SOURCE_UNKNOWN` | 0 | Unknown |
| `CTRL_SOURCE_APP` | 1 | App control |
| `CTRL_SOURCE_SDK` | 2 | SDK control |
| `CTRL_SOURCE_OTHER` | 3 | Other control source |

---

### ControlLostInfo

```cpp
struct ControlLostInfo
```

**Description:**  
Control ownership lost information.

---

### ControlAvailableInfo

```cpp
struct ControlAvailableInfo
```

**Description:** Control ownership availability information.

---

### TakeControlAck

```cpp
struct TakeControlAck
```

**Description:**  
Acknowledgment information for a take-control command.

| Member | Type | Description |
|:--|:--|:--|
| `error_code` | `uint32_t` | Error code (`0` means success; non-zero means failure) |
| `reason` | `std::string` | Failure reason description |

---

### ReleaseControlAck

```cpp
struct ReleaseControlAck
```

**Description:**  
Acknowledgment information for a release-control command.

| Member | Type | Description |
|:--|:--|:--|
| `error_code` | `uint32_t` | Error code (`0` means success; non-zero means failure) |
| `reason` | `std::string` | Failure reason description |

---

### CameraBitrateCmd

```cpp
struct CameraBitrateCmd
```

**Description:**  
Camera bitrate configuration parameters. This maps to wire protocol `type=1019`, `data.target=805`, with fields carried under `data.params`.

| Member | Type | Description |
|:--|:--|:--|
| `camera_name` | `std::string` | Camera name: `"camera_front"` for the front camera or `"camera_back"` for the rear camera |
| `camera_bps` | `uint32_t` | Camera bitrate in bit/s; valid range: 50,000–100,000,000 |

---

### CameraBitrateAck

```cpp
struct CameraBitrateAck
```

**Description:** Camera bitrate configuration acknowledgment. This maps to wire protocol `type=1019`, `data.target=805`; `camera_bps` is the actual value returned by the device.

| Member | Type | Description |
|:--|:--|:--|
| `camera_name` | `std::string` | Camera name: `"camera_front"` for the front camera or `"camera_back"` for the rear camera |
| `camera_bps` | `uint32_t` | Camera bitrate in bit/s; valid range: 50,000–100,000,000 |

---

### PhotoDeviceId

```cpp
enum class PhotoDeviceId : uint32_t
```

**Description:**  
Photo device ID.

| Enum Value | Integer Value | Description |
|:--|:--|:--|
| `FRONT` | 0 | Front camera |
| `BACK` | 1 | Rear camera |

---

### TakePhotoCmd

```cpp
struct TakePhotoCmd
```

**Description:**  
Take-photo command parameters.

| Member | Type | Description |
|:--|:--|:--|
| `task_id` | `uint32_t` | Task ID supplied by the caller and used to match the acknowledgment |
| `device_id` | `uint32_t` | Device ID: `0` for the front camera, `1` for the rear camera |

---

### TakePhotoAck

```cpp
struct TakePhotoAck
```

**Description:**  
Take-photo command acknowledgment information.

| Member | Type | Description |
|:--|:--|:--|
| `task_id` | `uint32_t` | Task ID matching `task_id` in the request |
| `device_id` | `uint32_t` | Device ID: `0` for the front camera, `1` for the rear camera |
| `error_code` | `uint32_t` | Error code; `0` means success and non-zero means failure |
| `reason` | `std::string` | Failure reason description |

---

### Speed

```cpp
struct Speed
```

**Description:** Current robot speed information.

| Member | Type | Description |
|:--|:--|:--|
| `line` | `double` | Forward/backward speed (m/s) |
| `translation` | `double` | Left/right translational speed (m/s) |
| `angle` | `double` | Turning angular speed (rad/s) |

---

### Joint Temperature Reference

```cpp
std::unordered_map<std::string, double>
```

**Description:** Map of robot joint names to joint temperatures.

The wheel-leg joint names are listed below.

| Name | Description |
|:--|:--|
| `fl1`, `fl2`, `fl3`, `fl4` | Front-left leg joints 1–4 |
| `fr1`, `fr2`, `fr3`, `fr4` | Front-right leg joints 1–4 |
| `bl1`, `bl2`, `bl3`, `bl4` | Rear-left leg joints 1–4 |
| `br1`, `br2`, `br3`, `br4` | Rear-right leg joints 1–4 |

---

### RobotState

```cpp
struct RobotState
```

**Description:** Robot state information structure.

| Member | Type | Description |
|:--|:--|:--|
| `head_angle` | `double` | Head angle (rad) |
| `front_fill_light` | `FillLightStatus` | Front fill light status |
| `back_fill_light` | `FillLightStatus` | Rear fill light status |
| `auto_mode_light` | `bool` | Whether automatic lighting mode is enabled |
| `obstacle_avoidance` | `bool` | Whether obstacle avoidance is enabled |
| `charging_pile_connected` | `bool` | Whether the charging pile is connected |
| `speed_level` | `SpeedLevel` | Current speed level |
| `software_emergency_status` | `EmergencyStatus` | Software emergency-stop status |
| `hardware_emergency_status` | `EmergencyStatus` | Hardware emergency-stop status |
| `head_direction` | `HeadDirection` | Current head/tail direction |
| `motion_status` | `MotionStatus` | Current motion status |
| `machine_status` | `MachineStatus` | Machine running status |
| `battery` | `BatteryData` | Battery information |
| `speed` | `Speed` | Current speed |
| `mile_data` | `float` | Cumulative mileage data (m) |
| `joint_temps` | `std::unordered_map<std::string, double>` | Joint temperature map `<joint name, joint temperature>` in °C |
| `control_source` | `CtrlSource` | Control source |

---

### JointStateData

```cpp
struct JointStateData
```

**Description:** Joint state data structure.

| Member | Type | Description |
|:--|:--|:--|
| `names` | `std::vector<std::string>` | Joint names |
| `positions` | `std::vector<double>` | Joint positions (rad) |
| `velocities` | `std::vector<double>` | Joint velocities (rad/s) |
| `efforts` | `std::vector<double>` | Joint efforts (N·m) |

---

### PeripheralPower

```cpp
enum class PeripheralPower
```

**Description:**  
Peripheral power channel.

| Enum Value | Integer Value | Description |
|:--|:--|:--|
| `UNKNOWN` | 0 | Unknown |
| `M1_48V` | 1 | M1 48V peripheral power |
| `M1_24V` | 2 | M1 24V peripheral power |
| `M1_12V` | 3 | M1 12V peripheral power |

---

### PowerCtrlCfg

```cpp
struct PowerCtrlCfg
```

**Description:**  
Peripheral power control configuration.

| Member | Type | Description |
|:--|:--|:--|
| `power` | `PeripheralPower` | Peripheral power channel |
| `enable` | `bool` | Switch state |

---

### PowerCtrlAck

```cpp
struct PowerCtrlAck
```

**Description:**  
Peripheral power control acknowledgment structure.

| Member | Type | Description |
|:--|:--|:--|
| `power` | `PeripheralPower` | Peripheral power channel |
| `enable` | `bool` | Switch state |

---

## LED

### LedId

```cpp
enum class LedId
```

| Enum Value | Description |
|:--|:--|
| `ALL` | All LEDs |
| `FRONT` | Front LEDs |
| `BACK` | Back LEDs |

### LedEffect

```cpp
enum class LedEffect
```

| Enum Value | Protocol Value | Description |
|:--|:--|:--|
| `OFF` | `off` | Off |
| `ON` | `on` | Solid on |
| `BREATH` | `breath` | Breathing |
| `BLINK` | `blink` | Blinking |
| `BLINK_TRANSIENT` | `blink_transient` | Transient blinking |

### LedCommand

```cpp
struct LedCommand
```

| Member | Type | Description |
|:--|:--|:--|
| `id` | `LedId` | LED group |
| `effect` | `LedEffect` | LED effect |
| `color` | `LedColor` | RGBA color |
| `duration_ms` | `uint32_t` | Effect period / blink interval in milliseconds |

`LedCommandAck` has the same fields as `LedCommand`; `LedAutoModeAck::auto_mode` indicates whether LED auto mode is enabled.

---

## Task-Related

### TaskType

```cpp
enum class TaskType
```

**Description:**  
Task type enumeration.

| Enum Value | Integer Value | Description |
|:--|:--|:--|
| `UNKNOWN` | 0 | Unknown |
| `SCAN_QR` | 1 | Scan QR code |
| `MAPPING` | 2 | Mapping |
| `NAV` | 3 | Navigation |
| `RECHARGING` | 4 | Recharging |
| `UNDOCK` | 5 | Undocking |
| `UWB_FOLLOW` | 6 | UWB follow |
| `VISUAL_TRACK` | 7 | Visual tracking |

---

### TaskStatus

```cpp
enum class TaskStatus
```

**Description:**  
Task status enumeration.

| Enum Value | Integer Value | Description |
|:--|:--|:--|
| `UNKNOWN` | 0 | Unknown |
| `STARTING` | 1 | Starting |
| `RUNNING` | 2 | Running |
| `SUCCESS` | 3 | Success (terminal state) |
| `FAILURE` | 4 | Failure (terminal state) |
| `STOPPED` | 5 | Stopped (terminal state) |

---

### TaskStateInfo

```cpp
struct TaskStateInfo
```

**Description:**  
Task state information structure.

| Member | Type | Description |
|:--|:--|:--|
| `task_type` | `TaskType` | Task type |
| `task_status` | `TaskStatus` | Task status |
| `phase` | `std::string` | Current task phase description |
| `error_code` | `uint32_t` | Error code (`0` means success, non-zero means failure) |

---

### MotionData

```cpp
struct MotionData
```

**Description:**  
Detailed motion control data.

| Member | Type | Description |
|:--|:--|:--|
| `quat[4]` | `float` | Quaternion `[w, x, y, z]` |
| `v_world[3]` | `float` | Velocity in world coordinates `[x, y, z]` (m/s) |
| `position[3]` | `float` | Position in world coordinates `[x, y, z]` (m) |
| `omega_world[3]` | `float` | Angular velocity in world coordinates `[x, y, z]` (rad/s) |
| `v_body[3]` | `float` | Velocity in body coordinates `[x, y, z]` (m/s) |
| `omega_body[3]` | `float` | Angular velocity in body coordinates `[x, y, z]` (rad/s) |
| `time_stamp` | `uint64_t` | Timestamp (ns) |

### SpeedData

```cpp
struct SpeedData
```

**Description:**  
Speed report information.

| Member | Type | Description |
|:--|:--|:--|
| `x` | `float` | Forward/backward velocity (m/s) |
| `y` | `float` | Left/right lateral velocity (m/s) |
| `yaw` | `float` | Rotational angular velocity (rad/s) |

---

## Related Documentation

- [SDKClient API Documentation](sdk_client_api_en.md) - Detailed client interface description
- [Connection Configuration Documentation](sdk_connection_en.md) - Connection parameters and status description
- [Callback Interface Documentation](sdk_callback_en.md) - Callback interface definitions
- [Recharge and Undock Task Usage Guide](sdk_recharge_task_en.md) - State transitions and usage recommendations for recharge and undock tasks
- [State Definition Documentation](sdk_state_en.md) - Detailed explanation of connection state and motion state
