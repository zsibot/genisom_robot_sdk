# Robot SDK Documentation Center

This page indexes the Robot SDK user documentation. If this is your first integration, complete “First Connection in 10 Minutes” in the package-root `README.md` first.

## Recommended Reading Paths

### First Integration

1. [Network Architecture](sdk_network_en.md): confirm the connection method, IP, and port.
2. [Connection Guide](sdk_connection_en.md): understand sync/async connection, timeout, and auto-reconnect.
3. Run `example/quick_start.cpp`: validate versions, connectivity, and the first state callback.
4. [Control Ownership](sdk_control_ownership_en.md): understand app/SDK arbitration before sending motion or task commands.
5. [Callback Reference](sdk_callback_en.md): distinguish send results, command ACKs, and state reports.
6. [Troubleshooting](sdk_troubleshooting_en.md): diagnose unexpected connection or control behavior by layer.

### Feature Development

| Scenario | Read first | Then consult |
| --- | --- | --- |
| State monitoring and sensor collection | [Callback Reference](sdk_callback_en.md) | [Data Types](sdk_type_en.md) |
| Posture and motion control | [Motion States and Command Delivery](sdk_state_en.md) | [SDKClient API](sdk_client_api_en.md) |
| Multiple controllers | [Control Ownership](sdk_control_ownership_en.md) | `example/take_control.cpp` |
| Recharge and undock | [Task Guide](sdk_recharge_task_en.md) | [Task Error Codes](sdk_recharge_errorcode_en.md) · [Site Conditions and Troubleshooting](sdk_recharge_undock_calibration_conditions_en.md) |
| Charging dock pairing | [Dock Pairing Guide](sdk_dock_pairing_en.md) | `example/dock_pairing.cpp` |
| LED effects | [LED Control Guide](sdk_led_control_en.md) | `example/led.cpp` |
| Multiple robot models | [API Capability Matrix](sdk_api_capability_en.md) | [Error Codes](sdk_error_en.md) |

## Reference Documents

| Document | What it answers |
| --- | --- |
| [SDKClient API](sdk_client_api_en.md) | Method signatures, parameters, results, constraints, and examples |
| [Data Types](sdk_type_en.md) | Enums, structs, field meanings, and units |
| [Callback Reference](sdk_callback_en.md) | Trigger semantics for `IDataCallback` and `IControlCallback` |
| [Connection Guide](sdk_connection_en.md) | `ConnectionConfig`, states, and reconnect policy |
| [SDK Error Codes](sdk_error_en.md) | Local SDK errors and standard error-condition mappings |
| [API Capability Matrix](sdk_api_capability_en.md) | Availability of each public method on M1/L2 variants |
| [Network Architecture](sdk_network_en.md) | M1 network components, access methods, and ports |
| [Motion States and Command Delivery](sdk_state_en.md) | Posture state machine and command confirmation |
| [Control Ownership](sdk_control_ownership_en.md) | Controller, observer, preemption, and recovery flows |
| [LED Control Guide](sdk_led_control_en.md) | Auto/manual modes and effect parameters |
| [Recharge and Undock Task Guide](sdk_recharge_task_en.md) | Starting, stopping, and evaluating long-running tasks |
| [Recharge and Undock Error Codes](sdk_recharge_errorcode_en.md) | Business failure reasons reported through `OnTaskStateData()` |
| [Dock Pairing Guide](sdk_dock_pairing_en.md) | Scanning, connecting, querying, and disconnecting a dock |
| [Recharge, Undock, and Calibration Field Guide](sdk_recharge_undock_calibration_conditions_en.md) | Site requirements, normal flows, and symptom-based actions |
| [Troubleshooting Guide](sdk_troubleshooting_en.md) | A diagnostic sequence from build and network to business state |

The raw protocol reference is currently provided in Chinese at `docs/zh/protocol/Protocol-1.3.0.md`. Use it only when implementing a client without this SDK.

## Three Rules That Apply Everywhere

1. **Check compatibility before calling a method.** Log the system version and device model after the handshake, then consult the capability matrix.
2. **Separate send success from business success.** Use ACKs and state for controls; use task state and business error codes for long-running tasks.
3. **Keep callbacks lightweight.** Copy and validate data in callbacks, move slow work to application threads, and never call synchronous SDK methods from a callback.

## Example Index

| File | Focus |
| --- | --- |
| `example/quick_start.cpp` | Minimal read-only connection and state validation |
| `example/data.cpp` | State, fault, and sensor subscriptions |
| `example/control.cpp` | Comprehensive interactive control and device information |
| `example/sync.cpp` | Synchronous mode |
| `example/async.cpp` | Asynchronous mode |
| `example/manual_reconnect.cpp` | Manual reconnection |
| `example/auto_reconnect.cpp` | Automatic reconnection |
| `example/take_control.cpp` | Ownership events and requests |
| `example/camera_bitrate.cpp` | Camera bitrate |
| `example/recharge.cpp` | Recharge and undock |
| `example/led.cpp` | Status LEDs |
| `example/dock_pairing.cpp` | Charging dock pairing |

## Before Reporting an Issue

Include the SDK, protocol, and robot system versions; device model and serial number; host architecture; transport and target address; full error text; and the smallest call sequence that reproduces the problem. Copy the template in the [Troubleshooting Guide](sdk_troubleshooting_en.md#issue-report-template).
