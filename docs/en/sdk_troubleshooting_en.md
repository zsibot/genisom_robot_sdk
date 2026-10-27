# Robot SDK Troubleshooting Guide

This guide provides a layered path from “can the program start?” to “did the business task complete?” Diagnose in order; do not guess at business methods or task errors while an upstream layer is still failing.

## Quick Triage

| Stage | Typical symptom | Primary evidence |
| --- | --- | --- |
| Build/load | Compile failure, process will not start, `.so` missing | CMake output, `uname -m`, `file`, `ldd` |
| Network | No route, target unreachable, repeated disconnects | `ip addr`, `ip route`, physical-link state |
| Connect/handshake | `Connect()` times out or never reaches `CONNECTED` | Return error, error callback, `ConnectionState` |
| Ownership/capability | Connected but ordinary controls have no effect | `control_source`, ownership callbacks, `DeviceType` |
| Command execution | Send succeeds but posture/device state does not change | Control ACK, `RobotState`, faults |
| Long-running task | Recharge/undock does not complete | `TaskStateInfo`, task `error_code`, site conditions |
| Callback/concurrency | Stalls, timeouts, or data backlog | Callback duration, threads, location of sync calls |

## 1. Check Host Architecture and Shared Library

From the SDK package root, run:

```bash
uname -m
find lib -maxdepth 2 -name 'librobot_sdk.so*' -print
file "lib/$(uname -m)/librobot_sdk.so"
```

The `uname -m` output must match the library directory, such as `x86_64` or `aarch64`. If an example reports `librobot_sdk.so: cannot open shared object file`, continue with:

```bash
ldd example/build/data
```

- `librobot_sdk.so` should resolve to `lib/<arch>/` in this package.
- Install the corresponding runtime dependency if another entry says `not found`.
- Prefer the package's `example/CMakeLists.txt`, which configures the runtime search path.

If the CMake cache came from another SDK path or architecture, remove **this example's** `example/build/` directory and configure it again. Do not reuse a build directory copied from another machine.

## 2. Check Address, Route, and Transport

The default `SDKClient` uses UDP. The standard port mapping is:

| Transport | Port |
| --- | ---: |
| UDP (default) | `8082` |
| WebSocket | `8081` |

An M1 Wi-Fi AP commonly uses `192.168.234.1:8082`. Use the site-specific network configuration for wired connections and L2-series robots.

```bash
ip addr
ip route get <robot_ip>
```

Confirm that:

1. The host uses the expected interface or robot Wi-Fi, not an unintended default route.
2. The target is the motion-control computer, not the navigation computer.
3. The port matches the `TransportProtocol` selected when constructing `SDKClient`.
4. A VPN, container network, or host firewall is not rewriting or blocking the path.

Treat `ping` as supporting evidence only. A device may disable ICMP, so a failed ping alone does not prove that the SDK service is unavailable.

## 3. Separate Connect Start, Handshake Completion, and Disconnect

A minimal synchronous check:

```cpp
robot_sdk::SDKClient client([](const std::error_code& ec) {
  std::cerr << "SDK error: " << ec.value() << ", " << ec.message() << '\n';
});

const auto ec = client.Connect(robot_ip, port, true);
if (ec) {
  std::cerr << "Connect failed: " << ec.value() << ", " << ec.message() << '\n';
  return;
}

std::cout << "state=" << static_cast<int>(client.GetConnectionState()) << '\n';
```

Keep all three types of evidence:

- the immediate `Connect()` return value;
- full `value()` and `message()` values from the constructor error callback or async connect callback; and
- transitions reported by `GetConnectionState()`.

The connection and handshake are complete only at `ConnectionState::CONNECTED`. Log these values immediately afterward:

```cpp
const auto info = client.GetDeviceInfo();
std::cout << "sdk=" << client.Version()
          << ", protocol=" << client.ProtocolVersion()
          << ", system=" << client.SystemVersion()
          << ", device=" << robot_sdk::DeviceTypeName(info.device_type)
          << ", sn=" << info.sn << '\n';
```

They quickly expose SDK/system compatibility mistakes and device-capability differences.

## 4. Connected but Unable to Control

Check in this order:

1. **Ownership:** a successful connection does not imply control ownership. Inspect `RobotState.control_source` and log `OnControlLost()`, `OnControlAvailable()`, and `OnTakeControlAck()`.
2. **Device capability:** `Errc::UnsupportedDeviceOperation` means the model identified during the handshake does not support the method. Consult the [API Capability Matrix](sdk_api_capability_en.md).
3. **Robot state:** emergency stop, faults, machine state, or current posture can prevent execution. Inspect `RobotState` and `OnFaultData()`.
4. **Result layer:** no immediate error may only mean that the command was submitted for sending. Continue with the corresponding control ACK and state transition.

If an app is connected or has taken over, implement the state machine in [Control Ownership](sdk_control_ownership_en.md). Never resume velocity commands until ownership has been confirmed again.

## 5. Function Succeeds but the Action Does Not Complete

Record this timeline for one command:

```text
call time → function return → send callback → control ACK → RobotState/TaskStateInfo change
```

- No send callback: check whether the connection dropped during sending.
- Send result but no ACK: keep the command type, timestamp, and connection state; check the robot service and protocol version.
- ACK but no state change: check posture-transition rules, emergency stop, faults, and ownership.
- Long-running task enters failure: use the business `error_code` from task state, not an SDK send error in its place.

See [Motion States and Command Delivery](sdk_state_en.md) for posture rules. See the [Recharge and Undock Task Guide](sdk_recharge_task_en.md) and [Task Error Codes](sdk_recharge_errorcode_en.md) for terminal-state evaluation.

## 6. Recharge, Undock, or Calibration Failure

First confirm that the SDK command was received. Then check field conditions: dock power and connection, QR-code recognition, calibration data, relative robot position, and clear motion space.

A task ACK means that the request was received, not that the task completed. Continue to process `OnTaskStateData()` and retain:

- `task_type`;
- `task_status`;
- `error_code`;
- `MachineStatus`, faults, and dock connection state from the same period.

See [Recharge, Undock, and Dock Calibration Conditions and Troubleshooting](sdk_recharge_undock_calibration_conditions_en.md) for symptom-to-cause guidance.

## 7. Callback Stall, Sync Timeout, or Data Loss

SDK callbacks run on an I/O thread. File I/O, database writes, complex computation, lock waits, or synchronous SDK calls inside a callback block subsequent network events.

Recommended pattern:

1. Validate and copy data only in the callback.
2. Put it into a bounded, thread-safe queue.
3. Process slow work on application worker threads.
4. Monitor queue depth, drop policy, and per-item processing duration.

In particular, never call a `block=true` or `timeout_ms>0` SDK operation from an SDK callback. See the [Callback Reference](sdk_callback_en.md) for the complete threading model.

## 8. Restoring State After Auto-Reconnect

Auto-reconnect restores transport connectivity; it does not restore all application semantics. After the state returns to `CONNECTED`:

1. Read and log device and system version information again.
2. Re-confirm ownership; do not retain the pre-disconnect `has_control` value.
3. Restore data-reporting configuration as needed.
4. Query or wait for current robot state before restoring business behavior.
5. Never replay historical commands that may cause motion automatically.

Repeated `RECONNECTING` should trigger checks of the physical link, power, IP changes, and routing before reducing the reconnect interval.

## Issue Report Template

Copy and complete the following information (redact sensitive values if necessary):

```text
SDK version:
Protocol version:
Robot system version:
Device model / serial number:
Host OS and architecture:
Transport / target IP / port:
Auto-reconnect enabled:
App or another SDK client connected:
Incident time:
Expected behavior:
Actual behavior:
Function return error_code:
Send callback / control ACK:
ConnectionState / RobotState / TaskStateInfo:
Faults from the same period:
Minimal reproduction steps:
```

Attach complete surrounding logs, not only the final error. For motion issues, also include the initial posture, emergency-stop state, site conditions, and dock conditions.

## Related Documents

- [Connection Guide](sdk_connection_en.md)
- [SDK Error Codes](sdk_error_en.md)
- [Control Ownership](sdk_control_ownership_en.md)
- [Callback Reference](sdk_callback_en.md)
- [API Capability Matrix](sdk_api_capability_en.md)
