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
Fill light status.

### SpeedLevel

```cpp
enum class SpeedLevel
```

**Description:**  
Robot speed level.

### CtrlSource

```cpp
enum class CtrlSource
```

**Description:**  
Robot control source.

### MachineStatus

```cpp
enum class MachineStatus
```

**Description:**  
Robot machine running status.

---

## Robot State Data

### Speed

```cpp
struct Speed
```

**Description:**  
Current robot speed information.

### RobotState

```cpp
struct RobotState
```

**Description:**  
Robot state information structure.

| Member | Type | Description |
|:--|:--|:--|
| `fill_light_status` | `FillLightStatus` | Fill light status |
| `speed_level` | `SpeedLevel` | Speed level |
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

**Description:**  
Joint state data structure.

| Member | Type | Description |
|:--|:--|:--|
| `names` | `std::vector<std::string>` | Joint names |
| `positions` | `std::vector<double>` | Joint positions (rad) |
| `velocities` | `std::vector<double>` | Joint velocities (rad/s) |
| `efforts` | `std::vector<double>` | Joint efforts (N·m) |

---

## Camera-Related

### PhotoDeviceId

```cpp
enum class PhotoDeviceId : uint32_t
```

**Description:**  
Photo device ID.

| Enum Value | Integer Value | Description |
|:--|:--|:--|
| `FRONT` | 0 | Front camera |
| `BACK` | 1 | Back camera |

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
| `device_id` | `uint32_t` | Device ID, `0`: front camera, `1`: back camera |

---

### TakePhotoAck

```cpp
struct TakePhotoAck
```

**Description:**  
Take-photo command acknowledgment information.

| Member | Type | Description |
|:--|:--|:--|
| `task_id` | `uint32_t` | Task ID matching the request |
| `device_id` | `uint32_t` | Device ID, `0`: front camera, `1`: back camera |
| `error_code` | `uint32_t` | Error code, `0` means success and non-zero means failure |
| `reason` | `std::string` | Failure reason description |

---

## Peripheral Power

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
