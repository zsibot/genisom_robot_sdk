# Robot SDK — `SDKClient` API Reference

## Overview

`SDKClient` is the core class of the Robot SDK. It manages the connection and sends all commands,
covering motion & posture control, speed control, lights, data report configuration, control ownership,
camera, recharge/undock tasks, peripheral power, and LED effects.

- Namespace: `robot_sdk`; header: `robot_sdk/sdk_client.hpp`
- Per-model API support: see the [API capability matrix](sdk_api_capability_en.md)
- Callbacks (data reports and command ACKs): see the [Callback Reference](sdk_callback_en.md)

## Four Things to Know Before Reading

### 1. Synchronous vs. Asynchronous Modes

Except `Connect`/`Disconnect`, every command API shares two common parameters:

| Parameter | Type | Default | Description |
|:--|:--|:--|:--|
| `timeout_ms` | `int` | `0` | `0`: async mode; `> 0`: sync mode — max time to wait for the send to complete (ms) |
| `handler` | `WriteHandler` | empty | Send-result callback in async mode; unused in sync mode |

- **Async mode** (`timeout_ms = 0`): the function returns immediately; the return value only indicates
  whether the command **entered the send pipeline**. The send result arrives via `handler`.
- **Sync mode** (`timeout_ms > 0`): the function blocks until the send completes or times out;
  the return value is the send result.

> **Never use sync mode inside a callback.** Write completion also depends on the current I/O thread,
> so the call cannot complete and will wait until it times out. See
> [Threading Model](sdk_callback_en.md) in the callback guide.

### 2. Two-Layer Result Semantics: "Sent" ≠ "Done"

- The returned `std::error_code` only indicates whether the command was **sent** (layer 1).
- The **business result** (layer 2) arrives through `IControlCallback` ACKs —
  e.g. whether `TakeControl()` succeeded is determined by `error_code` in `OnTakeControlAck()`.
- Most posture-command ACKs only mean "the robot received the command". Whether the action finished
  should be observed via `RobotState` / `OnTaskStateData()`.

### 3. Error Code Mechanism

All APIs return `std::error_code`: success means `!ec` is true (`ec.value() == 0`);
on failure, `ec.message()` gives a human-readable description.
Full definitions are in the [Error Code Guide](sdk_error_en.md).

Common standard error codes:

| Error Code | Meaning |
|:--|:--|
| `std::errc::success` | Operation succeeded |
| `std::errc::invalid_argument` | Invalid argument |
| `std::errc::not_connected` | Not connected |
| `std::errc::timed_out` | Operation timed out |
| `std::errc::operation_canceled` | Operation canceled |
| `std::errc::operation_in_progress` | Same operation already in progress |

SDK-specific error codes:

| Error Code | Value | Meaning |
|:--|:--:|:--|
| `robot_sdk::Errc::ShakeHandFailed` | 10000 | Handshake failed |
| `robot_sdk::Errc::ProtocolMismatch` | 10001 | Protocol version mismatch (common with older robots) |
| `robot_sdk::Errc::ControlledDenial` | 10002 | Control denied (common with multiple connected clients) |
| `robot_sdk::Errc::ConnectFailed` | 10003 | Connection failed |
| `robot_sdk::Errc::UnsupportedDeviceOperation` | 10004 | Operation not supported by this model |

### 4. Control Ownership Prerequisite

Normal control commands are executed by the robot only while the SDK owns control —
except the software e-stop. See [Control Ownership](sdk_control_ownership_en.md).

## API Overview

| Group | APIs |
|:--|:--|
| [Connection Management](#connection-management) | `Connect`, `Disconnect`, `IsConnected`, `GetConnectionState` |
| [Callback Registration](#callback-registration) | `SetControlCallback`, `SetDataCallback` |
| [Safety](#safety) | `SoftEmergencyStop` |
| [Posture Control](#posture-control) | `StandUp`, `BalanceStandUp`, `LieDown`, `Stair`, `Crawl`, `CrawlWalk`, `Climb`, `Slim`, `Gait`, `DSB`, `PosControl`, `SkWalk`, `Sand`, `ReverseHeadTail`, `Locked` |
| [Motion Control](#motion-control) | `Move`, `PosMove`, `Turn`, `ControlHead`, `HighLowStance`, `SetSpeed` |
| [Lights & Perception Switches](#lights--perception-switches) | `FrontLight`, `BackLight`, `AutoModeLight`, `ObstacleAvoidance` |
| [Data Report Configuration](#data-report-configuration) | `SetImuConfig`, `SetLuxConfig`, `SetMcConfig`, `SetSpeedReportConfig`, `SetJointStateConfig` |
| [Control Ownership](#control-ownership) | `TakeControl`, `ReleaseControl` |
| [Camera](#camera) | `UpdateCameraBitrate` |
| [Tasks & State Switching](#tasks--state-switching) | `StartRechargeTask`, `StopRechargeTask`, `StartUnDockTask`, `StopUnDockTask`, `SwitchRemoteState`, `SwitchIdleState` |
| [Peripheral Power](#peripheral-power) | `SetPeriphPower`, `GetPeriphPower` |
| [LED Effects](#led-effects) | `SetLedAutoMode`, `GetLedAutoMode`, `SetLedCommand` |
| [Version & Device Info](#version--device-info) | `Version`, `ProtocolVersion`, `SystemVersion`, `GetDeviceInfo` |

## Type Definitions

| Type | Definition | Description |
|:--|:--|:--|
| `ConnectHandler` | `std::function<void(const std::error_code&)>` | Connect completion callback |
| `DisConnectHandler` | `std::function<void(const std::error_code&)>` | Disconnect completion callback |
| `ErrorHandler` | `std::function<void(const std::error_code&)>` | SDK internal error callback (passed to the constructor) |
| `WriteHandler` | `std::function<void(const std::error_code&, std::size_t)>` | Command send completion callback |

---

## Construction & Destruction

### Constructor

```cpp
SDKClient(ErrorHandler error_callback = [](const std::error_code&) {},
          ConnectionConfig connection_config = ConnectionConfig(),
          TransportProtocol type = TransportProtocol::Udp)
```

**Parameters:**

| Parameter | Type | Default | Description |
|:--|:--|:--|:--|
| `error_callback` | `ErrorHandler` | empty | SDK internal communication error callback |
| `connection_config` | `ConnectionConfig` | defaults | Connect timeout, auto-reconnect, etc. See the [Connection Guide](sdk_connection_en.md) |
| `type` | `TransportProtocol` | `Udp` | Transport: `Udp` (port 8082) or `WebSocket` (port 8081) |

### Destructor

```cpp
~SDKClient()
```

Stops the I/O threads, disconnects, and releases resources automatically. Calling `Disconnect()`
explicitly is not required, but recommended for deterministic shutdown behavior.

---

## Connection Management

### Connect — Connect to the Robot

```cpp
std::error_code Connect(std::string ip, std::string port,
                        bool block = false,
                        ConnectHandler handler = [](const std::error_code&) {})
```

Initiates a connection and completes the handshake. Connecting has two phases —
"establish the transport" and "handshake negotiation" — and only a successful handshake
counts as connected.

**Parameters:**

| Parameter | Type | Default | Description |
|:--|:--|:--|:--|
| `ip` | `std::string` | — | Robot IP, e.g. `"192.168.234.1"` |
| `port` | `std::string` | — | Port: `"8082"` for UDP (default), `"8081"` for WebSocket |
| `block` | `bool` | `false` | `false`: async; `true`: sync (blocks until connected or failed) |
| `handler` | `ConnectHandler` | empty | Result callback in async mode; unused in sync mode |

**Returns:**

- Async mode: only whether the request was accepted; the result arrives via `handler`
- Sync mode: the final result
- Common failures: `Errc::ShakeHandFailed`, `Errc::ProtocolMismatch`, `Errc::ControlledDenial`,
  `std::errc::operation_in_progress`

> Note: connected ≠ owning control. If the APP is already connected and controlling the robot,
> the SDK can only be an observer. See [Control Ownership](sdk_control_ownership_en.md).

---

### Disconnect — Disconnect from the Robot

```cpp
std::error_code Disconnect(bool block = false,
                           DisConnectHandler handler = [](const std::error_code&) {})
```

Disconnects from the robot (in UDP mode a wave-hand frame is sent first to notify the robot).

**Parameters:** same as `Connect`'s `block` / `handler`.

**Returns:** in async mode, whether the request was accepted; in sync mode, the final result.
Calling it when already disconnected returns an appropriate error code.

---

### IsConnected — Connection Check

```cpp
bool IsConnected() const
```

Returns `true` when the current state is `ConnectionState::CONNECTED` (handshake completed).

### GetConnectionState — Detailed Connection State

```cpp
ConnectionState GetConnectionState() const
```

Returns the `ConnectionState` enum (`DISCONNECTED` / `CONNECTING` / `HANDSHAKING` / `CONNECTED` /
`DISCONNECTING` / `RECONNECTING`). See the [Connection Guide](sdk_connection_en.md).

---

## Callback Registration

### SetControlCallback — Register Command ACK Callbacks

```cpp
void SetControlCallback(std::shared_ptr<IControlCallback> control_callback)
```

Registers callbacks for command acknowledgments ("robot received the command" confirmations
and business results). Full list: [Callback Reference](sdk_callback_en.md).

### SetDataCallback — Register Data Report Callbacks

```cpp
void SetDataCallback(std::shared_ptr<IDataCallback> data_callback)
```

Registers callbacks for data actively reported by the robot (sensors, state, faults,
ownership events, etc.). Full list: [Callback Reference](sdk_callback_en.md).

> Register callbacks before `Connect()` to avoid missing early reports.

---

## Safety

### SoftEmergencyStop — Software E-stop

```cpp
std::error_code SoftEmergencyStop(bool on, int timeout_ms = 0,
                                  WriteHandler handler = [](const std::error_code&, std::size_t) {})
```

Triggers or releases the software e-stop. While triggered, the robot ignores other motion
commands and forces velocity to zero. **The e-stop is not restricted by ownership** —
observers may call it too, so safety logic never has to wait for control.

**Parameters:**

| Parameter | Type | Description |
|:--|:--|:--|
| `on` | `bool` | `true`: trigger e-stop; `false`: release e-stop |

**ACK:** `OnSoftEmergencyStop(bool on)`. **State check:** `RobotState::software_emergency_status`.

---

## Posture Control

The following APIs are all posture-switch commands taking only the common
`timeout_ms` / `handler` parameters (see [Sync vs. Async](#1-synchronous-vs-asynchronous-modes)).
Each has a one-to-one ACK callback (see the [Callback Reference](sdk_callback_en.md)).

```cpp
std::error_code StandUp(int timeout_ms = 0, WriteHandler handler = ...);        // Stand up
std::error_code BalanceStandUp(int timeout_ms = 0, WriteHandler handler = ...); // Balance stand
std::error_code LieDown(int timeout_ms = 0, WriteHandler handler = ...);        // Lie down
std::error_code Stair(int timeout_ms = 0, WriteHandler handler = ...);          // Stair posture
std::error_code Crawl(int timeout_ms = 0, WriteHandler handler = ...);          // Crawl
std::error_code CrawlWalk(int timeout_ms = 0, WriteHandler handler = ...);      // Crawl walk
std::error_code Climb(int timeout_ms = 0, WriteHandler handler = ...);          // Climb platform (general mode)
std::error_code Slim(int timeout_ms = 0, WriteHandler handler = ...);           // Slim / narrow passage (general mode)
std::error_code Gait(int timeout_ms = 0, WriteHandler handler = ...);           // Gait (general mode)
std::error_code DSB(int timeout_ms = 0, WriteHandler handler = ...);            // DSB (general mode)
std::error_code PosControl(int timeout_ms = 0, WriteHandler handler = ...);     // Enter position-control posture (general mode)
std::error_code SkWalk(int timeout_ms = 0, WriteHandler handler = ...);         // SameKnee walk (general mode)
std::error_code Sand(int timeout_ms = 0, WriteHandler handler = ...);           // Sand posture (general mode)
std::error_code ReverseHeadTail(int timeout_ms = 0, WriteHandler handler = ...);// Reverse head/tail
std::error_code Locked(int timeout_ms = 0, WriteHandler handler = ...);         // Lock joints
```

Key notes:

- **StandUp**: while standing up, `RobotState::motion_status` reports `MOTION_STATUS_STAND_UP`;
  once finished, it becomes `MOTION_STATUS_WALK`.
- **Locked**: joints hold their current positions; issuing another posture command
  (stand up / lie down / crawl...) unlocks automatically.
- **Sand**: the wire-protocol action is `action/snow`; the device-reported `snow` state is parsed
  by the SDK as `MOTION_STATUS_SAND`.
- **Model restrictions**: `Slim`, `DSB`, `SkWalk`, and `Sand` are not supported on the L2
  series; `ReverseHeadTail` is unsupported on the L2 series and on `DeviceType::M1_AIR` /
  `DeviceType::M1F_AIR`; `Gait` is unsupported on the L2F point-foot variants
  (`DeviceType::L2F`, `DeviceType::L2F_ULTRA`).
  On unsupported models the SDK does not send the command and returns
  `robot_sdk::Errc::UnsupportedDeviceOperation`. Full matrix: [API capability matrix](sdk_api_capability_en.md).

```cpp
auto ec = client.SkWalk();
if (ec == robot_sdk::Errc::UnsupportedDeviceOperation) {
    // SameKnee walk is not supported on this model
}
```

---

## Motion Control

### Move — Move

```cpp
std::error_code Move(float left_right, float forward_back, float yaw,
                     int timeout_ms = 0,
                     WriteHandler handler = [](const std::error_code&, std::size_t) {})
```

Moves the robot in general mode. All three arguments are normalized ratios; the actual speed
is scaled by the current speed level (`SetSpeed`) — see the
[SpeedLevel mapping table](sdk_type_en.md#speedlevel--speed-level). The latest Move command
stays effective on the robot for 1 second, so send it periodically (10–20 Hz recommended)
for continuous motion.

**Parameters:**

| Parameter | Type | Range | Description |
|:--|:--|:--|:--|
| `left_right` | `float` | [-1.0, 1.0] | Lateral speed ratio; positive = left, negative = right |
| `forward_back` | `float` | [-1.0, 1.0] | Forward speed ratio; positive = forward, negative = backward |
| `yaw` | `float` | [-1.0, 1.0] | Rotation speed ratio; positive = turn left, negative = turn right |

**Returns:** out-of-range arguments yield `std::errc::invalid_argument`;
see [Error Code Mechanism](#3-error-code-mechanism) for others.
**ACK:** none — check the `WriteHandler` send result only.

---

### PosMove — Position-Control Move

```cpp
std::error_code PosMove(PosControlCmd cmd, int timeout_ms = 0,
                        WriteHandler handler = [](const std::error_code&, std::size_t) {})
```

Sets the target body pose in position-control mode; enter the posture first via `PosControl()`.

**Parameters:**

| Parameter | Type | Description |
|:--|:--|:--|
| `cmd` | `PosControlCmd` | Target pose: `x/y/z` (m) and `roll/pitch/yaw` (rad). See [PosControlCmd](sdk_type_en.md#position-control) |

---

### Turn — Body Roll

```cpp
std::error_code Turn(int direction, int timeout_ms = 0,
                     WriteHandler handler = [](const std::error_code&, std::size_t) {})
```

Rolls the body left/right in place (in-place mode only).

**Parameters:**

| Parameter | Type | Description |
|:--|:--|:--|
| `direction` | `int` | `0`: recover; `1`: roll left; `2`: roll right (range [0, 2]) |

**Returns:** out-of-range `direction` yields `std::errc::invalid_argument`.

---

### ControlHead — Head Control

```cpp
std::error_code ControlHead(float left_right, float up_down, int timeout_ms = 0,
                            WriteHandler handler = [](const std::error_code&, std::size_t) {})
```

Controls "lean head left/right" and "look up/down" (in-place mode only).

**Parameters:**

| Parameter | Type | Range | Description |
|:--|:--|:--|:--|
| `left_right` | `float` | [-1.0, 1.0] | Positive = lean left, negative = lean right (rad/s) |
| `up_down` | `float` | [-1.0, 1.0] | Positive = look up, negative = look down (rad/s) |

**Returns:** out-of-range arguments yield `std::errc::invalid_argument`.

---

### HighLowStance — High/Low Stance

```cpp
std::error_code HighLowStance(int stance, int timeout_ms = 0,
                              WriteHandler handler = [](const std::error_code&, std::size_t) {})
```

Adjusts body height stance (in-place mode only).

**Parameters:**

| Parameter | Type | Description |
|:--|:--|:--|
| `stance` | `int` | `0`: recover; `1`: high stance; `2`: low stance (range [0, 2]) |

---

### SetSpeed — Set Speed Level

```cpp
std::error_code SetSpeed(int speed_level, int timeout_ms = 0,
                         WriteHandler handler = [](const std::error_code&, std::size_t) {})
```

Sets the speed level, which scales the actual velocity limits of `Move()` in general mode
(default: low). See the [SpeedLevel mapping table](sdk_type_en.md#speedlevel--speed-level)
for how normalized values map to real speeds at each level.

**Parameters:**

| Parameter | Type | Description |
|:--|:--|:--|
| `speed_level` | `int` | `1`: low; `2`: medium; `3`: high (range [1, 3]) |

**Returns:** out-of-range level yields `std::errc::invalid_argument`.
**ACK:** `OnSpeed(int speed_level)`; the current level is also in `RobotState::speed_level`.

---

## Lights & Perception Switches

```cpp
std::error_code FrontLight(bool on, int timeout_ms = 0, WriteHandler handler = ...);        // Front fill light
std::error_code BackLight(bool on, int timeout_ms = 0, WriteHandler handler = ...);         // Back fill light
std::error_code AutoModeLight(bool on, int timeout_ms = 0, WriteHandler handler = ...);     // Auto light mode
std::error_code ObstacleAvoidance(bool on, int timeout_ms = 0, WriteHandler handler = ...); // Obstacle avoidance
```

**Parameters:**

| Parameter | Type | Description |
|:--|:--|:--|
| `on` | `bool` | `true`: enable; `false`: disable |

Key notes:

- Calling `FrontLight` / `BackLight` manually disables the auto light mode.
- **Model restrictions**: `BackLight` is unsupported on the L2 series and on
  `DeviceType::M1_AIR` / `DeviceType::M1F_AIR`; `ObstacleAvoidance` is unsupported on the L2 series.
  Both return `Errc::UnsupportedDeviceOperation`.
- Current states are available in `RobotState` (`front_fill_light`, `back_fill_light`,
  `auto_mode_light`, `obstacle_avoidance`).

---

## Data Report Configuration

By default the robot only reports `RobotState` (1 Hz) and fault data. Enable more streams on demand:

```cpp
std::error_code SetImuConfig(int freq, int timeout_ms = 0, WriteHandler handler = ...);                 // IMU
std::error_code SetLuxConfig(bool on, int timeout_ms = 0, WriteHandler handler = ...);                  // Illuminance
std::error_code SetMcConfig(bool on, int timeout_ms = 0, WriteHandler handler = ...);                   // Motion data
std::error_code SetSpeedReportConfig(bool on, uint32_t frequency, int timeout_ms = 0, WriteHandler handler = ...); // Speed
std::error_code SetJointStateConfig(bool on, int timeout_ms = 0, WriteHandler handler = ...);           // Joint states
```

| API | Parameters | Data Callback | Report Rate |
|:--|:--|:--|:--|
| `SetImuConfig` | `freq`: [0, 100], 0 disables | `OnImuData` | As configured |
| `SetLuxConfig` | `on`: switch | `OnLuxData` | Fixed 1 Hz |
| `SetMcConfig` | `on`: switch | `OnMcData` | Fixed 50 Hz |
| `SetSpeedReportConfig` | `on` + `frequency`: [1, 50] Hz | `OnSpeedData` | As configured |
| `SetJointStateConfig` | `on`: switch | `OnJointStateData` | Fixed rate |

Each configuration has a matching config ACK (`OnImuConfig`, etc.). Struct definitions:
[Data Types Reference](sdk_type_en.md).

---

## Control Ownership

### TakeControl — Request Ownership

```cpp
std::error_code TakeControl(int timeout_ms = 0,
                            WriteHandler handler = [](const std::error_code&, std::size_t) {})
```

Requests control ownership of the robot. Note:

- The return value only indicates whether the request was sent — **whether ownership was
  acquired is determined by `error_code` in `OnTakeControlAck()`** (`0` = success;
  see `reason` on failure).
- If the APP is still connected to the robot, the request fails — wait for
  `OnControlAvailable()` before requesting.
- Full rules and the recommended handling flow: [Control Ownership](sdk_control_ownership_en.md).

### ReleaseControl — Release Ownership

```cpp
std::error_code ReleaseControl(int timeout_ms = 0,
                               WriteHandler handler = [](const std::error_code&, std::size_t) {})
```

Voluntarily releases the ownership currently held; the result is reported by
`OnReleaseControlAck()`. Call this when exiting or when control is no longer needed,
so ownership becomes available to other clients sooner.

---

## Camera

### UpdateCameraBitrate — Update Camera Bitrate

```cpp
std::error_code UpdateCameraBitrate(CameraBitrateCmd cmd, int timeout_ms = 0,
                                    WriteHandler handler = [](const std::error_code&, std::size_t) {})
```

**Parameters:**

| Parameter | Type | Description |
|:--|:--|:--|
| `cmd` | `CameraBitrateCmd` | `camera_name` (`"camera_front"` / `"camera_back"`) + `camera_bps` (50000–100000000 bps) |

**ACK:** `OnUpdateCameraBitrateAck()`; the `camera_bps` in it is the value actually applied
by the device.

---

## Tasks & State Switching

```cpp
std::error_code StartRechargeTask(int timeout_ms = 0, WriteHandler handler = ...);  // Start recharge
std::error_code StopRechargeTask(int timeout_ms = 0, WriteHandler handler = ...);   // Stop recharge
std::error_code StartUnDockTask(int timeout_ms = 0, WriteHandler handler = ...);    // Start undock
std::error_code StopUnDockTask(int timeout_ms = 0, WriteHandler handler = ...);     // Stop undock
std::error_code SwitchRemoteState(int timeout_ms = 0, WriteHandler handler = ...);  // Switch to remote state
std::error_code SwitchIdleState(int timeout_ms = 0, WriteHandler handler = ...);    // Switch to idle state
```

Key notes:

- For task APIs (recharge/undock), "entered/exited" is confirmed via `OnStartRechargeTask()` etc.;
  **task progress and final results** are reported via `OnTaskStateData()` (`TaskStateInfo`).
  Full state flow and usage tips: [Recharge & Undock Task Guide](sdk_recharge_task_en.md).
- `SwitchRemoteState` / `SwitchIdleState` switch `MachineStatus` (`REMOTE` ↔ `IDLE`);
  their ACKs are `OnSwitchRemote()` / `OnSwitchIdle()`.

---

## Peripheral Power

### SetPeriphPower — Set Peripheral Power

```cpp
std::error_code SetPeriphPower(const PowerCtrlCfg& cfg, int timeout_ms = 0,
                               WriteHandler handler = [](const std::error_code&, std::size_t) {})
```

Switches a peripheral power channel on or off.

**Parameters:**

| Parameter | Type | Description |
|:--|:--|:--|
| `cfg` | `PowerCtrlCfg` | `power`: `M1_12V` / `M1_24V` / `M1_48V`; `enable`: `true` on / `false` off |

**Returns:** invalid `power` yields `std::errc::invalid_argument`.
**ACK:** `OnSetPeriphPower()` echoes the requested channel and state.

### GetPeriphPower — Query Peripheral Power

```cpp
std::error_code GetPeriphPower(const PowerCtrlCfg& cfg, int timeout_ms = 0,
                               WriteHandler handler = [](const std::error_code&, std::size_t) {})
```

Queries the current state of a power channel (only the `power` field of `cfg` is used).
**The query result arrives via `OnGetPeriphPower()`** (`ack.enable` is the actual state);
the function itself does not return the queried value.

---

## LED Effects

```cpp
std::error_code SetLedAutoMode(bool auto_mode, int timeout_ms = 0, WriteHandler handler = ...);  // Set auto/manual mode
std::error_code GetLedAutoMode(int timeout_ms = 0, WriteHandler handler = ...);                  // Query auto/manual mode
std::error_code SetLedCommand(const LedCommand& cmd, int timeout_ms = 0, WriteHandler handler = ...); // Set LED effect
```

Key notes:

- In **auto mode**, effects are switched by the robot according to its state;
  in **manual mode**, effects are controlled by `SetLedCommand()`.
- `SetLedCommand` returns `std::errc::invalid_argument` when `cmd.id` or `cmd.effect` is `UNKNOWN`.
- Query/set results arrive via `OnGetLedAutoMode()` / `OnSetLedAutoMode()` / `OnSetLedCommand()`.
- Effect parameters (group, effect, color, period) and complete examples:
  [LED Control Guide](sdk_led_control_en.md).

```cpp
LedCommand cmd{LedId::ALL, LedEffect::BLINK, {255, 128, 0, 255}, 300};
client.SetLedCommand(cmd);
```

---

## Version & Device Info

### Version — SDK Version

```cpp
const std::string& Version() const
```

Returns the SDK's own version string (e.g. `"1.2.0"`). Available at any time.

### ProtocolVersion — Protocol Version

```cpp
const std::string& ProtocolVersion() const
```

Returns the communication protocol version used by the SDK. Available at any time.
The robot validates this version during the handshake; a mismatch fails the connection
with `Errc::ProtocolMismatch`.

### SystemVersion — Robot System Version

```cpp
const std::string& SystemVersion() const
```

Returns the robot-side system version. **Valid only after a successful handshake**;
empty while not connected.

### GetDeviceInfo — Get Device Info

```cpp
DeviceInfo GetDeviceInfo() const
```

Returns the device information cached from the last successful handshake
(local read, no network request):

```cpp
struct DeviceInfo {
  DeviceType device_type = DeviceType::UNKNOWN;
  std::string sn;
};
```

The SDK converts the protocol model returned by the handshake to a `DeviceType`
enum that uses M1/L2 product names. Before connecting, after
disconnecting, when an older robot does not report its model, or when the model
is unrecognized, `device_type` is `DeviceType::UNKNOWN`; `sn` is empty. See the
complete enum in the [Data Types Reference](sdk_type_en.md#devicetype).

```cpp
auto ec = client.Connect("192.168.234.1", "8082", true);
if (!ec) {
    const auto info = client.GetDeviceInfo();
    std::cout << "device_type=" << DeviceTypeName(info.device_type)
              << ", sn=" << info.sn << std::endl;
}
```

---

## Complete Example

```cpp
#include <iostream>
#include "robot_sdk/sdk_client.hpp"
using namespace robot_sdk;

int main() {
    SDKClient client([](const std::error_code& ec) {
        if (ec) std::cerr << "SDK Error: " << ec.message() << std::endl;
    });

    auto ec = client.Connect("192.168.234.1", "8082", true);  // Synchronous connect
    if (ec) {
        std::cerr << "Connect failed: " << ec.message() << std::endl;
        return -1;
    }

    client.StandUp(2000);          // Sync: wait for the send to complete
    client.Move(0.0f, 0.5f, 0.0f); // Async: half-speed forward
    client.Disconnect(true);
    return 0;
}
```

More examples in the `example/` directory: `control.cpp` (motion), `take_control.cpp` (ownership),
`led.cpp` (LED effects), `recharge.cpp` (recharge task), etc.

## Related Documents

- [Callback Reference](sdk_callback_en.md) — Data reports and command ACKs
- [Data Types Reference](sdk_type_en.md) — Parameter and state struct definitions
- [Connection Guide](sdk_connection_en.md) — Connection parameters, timeouts, auto-reconnect
- [Error Code Guide](sdk_error_en.md) — Error code definitions and checks
- [Control Ownership](sdk_control_ownership_en.md) — SDK vs APP ownership rules
- [API Capability Matrix](sdk_api_capability_en.md) — Per-model API support
- [Recharge & Undock Task Guide](sdk_recharge_task_en.md) — Detailed task API usage
