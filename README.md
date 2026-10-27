# Robot SDK User Guide

Robot SDK is a C++17 client library for M1- and L2-series robots. It connects applications to a robot, streams state and sensor data, and provides model-aware motion, device, and task control.

[中文](README_zh.md) · [Documentation Center](docs/en/index_en.md) · [API Reference](docs/en/sdk_client_api_en.md) · [Changelog](CHANGELOG.md)

## Before You Start

### Requirements

- Ubuntu 22.04 (currently validated environment)
- x86_64 or aarch64, with a matching library under `lib/<arch>/`
- CMake 3.8+, a C++17 compiler, and Boost 1.74
- Wired or Wi-Fi access to the robot network

### Safety

The `control`, `sync`, `async`, and `recharge` examples can move the robot or change device state. Clear the area, make sure an emergency stop is available, and validate connectivity with the read-only `quick_start` example first.

### Integration Checklist

1. Identify the robot family and system version, then choose an SDK from the [compatibility table](#sdk-and-robot-system-compatibility).
2. Run `uname -m` and verify that its output matches a directory under `lib/`.
3. Confirm the robot IP, transport, and port. UDP is the default; an M1 Wi-Fi AP commonly uses `192.168.234.1:8082`.
4. If the app is connected at the same time, read [Control Ownership](docs/en/sdk_control_ownership_en.md). A successful connection does not imply control ownership.

## First Connection in 10 Minutes

Run these commands from the extracted SDK root.

### 1. Check the Package

```bash
uname -m
test -f "lib/$(uname -m)/librobot_sdk.so" && echo "SDK library found"
```

If `SDK library found` is not printed, obtain a package containing the library for the host architecture.

### 2. Join the Robot Network

- M1/L2 over Wi-Fi: connect the host to the robot Wi-Fi AP; the common target address is `192.168.234.1`.
- Wired: place the host and motion-control computer on the same LAN and use the robot address assigned at the site.
- Default ports: UDP `8082`; WebSocket `8081` when explicitly selected.

See [Network Architecture](docs/en/sdk_network_en.md) for the topology. You can first check whether the host has a route to the target:

```bash
ip route get 192.168.234.1
```

### 3. Build the Examples

```bash
cd example
mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j"$(nproc)"
```

The example project locates `include/` and `lib/<arch>/` from the current SDK package. You do not need to copy headers or set `LD_LIBRARY_PATH` manually.
After the build, run the following example commands from `example/build/`.

### 4. Run the Read-Oriented Example First

```bash
./quick_start 192.168.234.1 8082
```

The basic path is working when the program:

- prints the SDK and protocol versions;
- reports `[OK] Connected`;
- then reports `[OK] First robot state received`; and
- finishes with `[OK] Disconnected cleanly` and no connection timeout or shared-library loading error.

The example disconnects and exits after receiving the first robot state. It warns if no state arrives within five seconds. If it does not connect, go to [Troubleshooting](#troubleshooting).

### 5. Try Interactive Control

After confirming site safety and understanding control ownership, run:

```bash
./control 192.168.234.1 8082
```

Press `h` for keyboard help, `o` for status, and `q` to quit. If the SDK is an observer, wait until ownership becomes available and press `t` to request it. `OnTakeControlAck()` provides the final request result.

## Integrating the SDK into a CMake Project

Pass the extracted SDK location to your project and link it as an imported library:

```cmake
cmake_minimum_required(VERSION 3.8)
project(my_robot_app LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(Threads REQUIRED)

set(ROBOT_SDK_ROOT "/absolute/path/to/robot_sdk" CACHE PATH "Robot SDK root")
add_library(robot_sdk SHARED IMPORTED)
set_target_properties(robot_sdk PROPERTIES
    IMPORTED_LOCATION
        "${ROBOT_SDK_ROOT}/lib/${CMAKE_SYSTEM_PROCESSOR}/librobot_sdk.so"
    INTERFACE_INCLUDE_DIRECTORIES
        "${ROBOT_SDK_ROOT}/include"
)

add_executable(my_robot_app main.cpp)
target_link_libraries(my_robot_app PRIVATE robot_sdk Threads::Threads)
set_target_properties(my_robot_app PROPERTIES
    BUILD_RPATH "${ROBOT_SDK_ROOT}/lib/${CMAKE_SYSTEM_PROCESSOR}"
)
```

Run the following commands from your own project root. You can override the SDK location while configuring:

```bash
cmake -S . -B build -DROBOT_SDK_ROOT=/opt/robot_sdk
cmake --build build -j"$(nproc)"
```

Use `example/quick_start.cpp` as the minimal application skeleton. When deploying, ship the matching `librobot_sdk.so` and make it discoverable through an install RPATH or the system library path.

## How to Interpret an SDK Call

A control operation commonly has three result layers:

1. **Function return value:** whether local validation passed and the command was submitted for sending.
2. **Send callback/control ACK:** whether transport completed and the robot received or handled the command.
3. **State or task callback:** whether the robot actually reached the target state, or the task completed or failed.

A successful function return therefore does not mean that the action has completed. Long-running operations such as recharge and undock must be evaluated through `OnTaskStateData()` and its error code. SDK callbacks run on an I/O thread: keep them lightweight and never call a synchronous SDK operation from a callback.

## Choosing an Example

| Example | Purpose | May change robot state |
| --- | --- | :---: |
| `quick_start` | Minimal connection, version/device details, and first-state check | No |
| `data` | Connect and subscribe to state, fault, and sensor data | No (changes reporting configuration) |
| `control` | Interactive keyboard control | Yes |
| `sync` | Synchronous connection and commands | Yes |
| `async` | Asynchronous connection, sends, and callbacks | Yes |
| `manual_reconnect` | Application-managed reconnection | No |
| `auto_reconnect` | SDK-managed reconnection | No |
| `take_control` | App/SDK ownership transitions | Possibly |
| `camera_bitrate` | Camera bitrate control | Yes |
| `recharge` | Recharge and undock tasks | Yes |
| `led` | Status LED mode and effects | Yes |
| `dock_pairing` | Dock scan, connect, query, and disconnect | Yes |

The common command form is:

```text
./<name> <robot_ip> <port> [example-specific arguments]
```

Some examples require extra arguments or keyboard input. Run one without arguments to print its usage.

## Documentation Map

| Goal | Document |
| --- | --- |
| Configure connection, timeout, or reconnect | [Connection Guide](docs/en/sdk_connection_en.md) |
| Look up all client methods | [SDKClient API](docs/en/sdk_client_api_en.md) |
| Look up structs, enums, and units | [Data Types](docs/en/sdk_type_en.md) |
| Understand data callbacks and command ACKs | [Callback Reference](docs/en/sdk_callback_en.md) |
| Check model-specific API support | [API Capability Matrix](docs/en/sdk_api_capability_en.md) |
| Handle SDK errors | [Error Codes](docs/en/sdk_error_en.md) |
| Handle app/SDK preemption | [Control Ownership](docs/en/sdk_control_ownership_en.md) |
| Understand posture changes and confirmation | [Motion States and Command Delivery](docs/en/sdk_state_en.md) |
| Build recharge or undock flows | [Task Guide](docs/en/sdk_recharge_task_en.md) · [Task Error Codes](docs/en/sdk_recharge_errorcode_en.md) |
| Pair a charging dock | [Dock Pairing Guide](docs/en/sdk_dock_pairing_en.md) |
| Control status LEDs | [LED Control Guide](docs/en/sdk_led_control_en.md) |
| Diagnose an integration | [Troubleshooting Guide](docs/en/sdk_troubleshooting_en.md) |

For all documents organized by role and development phase, open the [Documentation Center](docs/en/index_en.md).

## Troubleshooting

| Symptom | Check first |
| --- | --- |
| `librobot_sdk.so` cannot be loaded | Does `uname -m` match `lib/<arch>/`, and was the binary built with the example CMake project? |
| Connection times out | Does the IP/port match the transport? Check the host route, Wi-Fi/cable, and robot system state. |
| Connected, but control has no effect | Is the SDK an observer? Was `OnControlLost()` received? Does the current robot state allow the command? |
| `UnsupportedDeviceOperation` | The device model does not support that method; check the capability matrix. |
| Function succeeds, but task does not finish | The return covers submit/send only; inspect the ACK, task state, and `error_code`. |
| Auto-reconnect keeps cycling | Fix physical-link or address instability first, then check the reconnect interval and state notifications. |
| A callback stalls or a synchronous call times out | Do not perform slow work or synchronous SDK calls inside callbacks. |

See the [Troubleshooting Guide](docs/en/sdk_troubleshooting_en.md) for a diagnostic sequence and an issue-report template.

## SDK and Robot System Compatibility

“Fully compatible” means the SDK has been adapted and validated against the public interfaces of that system version. It does not mean all robot models have identical hardware capabilities. New projects should use the recommended SDK.

### M1 Series

| System version | Recommended SDK | Other compatible SDKs | Compatibility/notes |
| --- | --- | --- | --- |
| `v0.0.5`–`v0.1.0-B` | `v0.0.3` | `v0.0.2` | Fully compatible |
| `v0.1.0-C`–`v0.2.4` | `v0.1.1` | `v0.0.4`–`v0.1.0` | Fully compatible; use the latest SDK in this stage |
| `v0.3.0` | `v0.2.0` | `v0.2.1` | `v0.2.0` fully compatible; `v0.2.1` partially compatible |
| `v0.3.1` | `v0.2.2` | `v0.2.1` | Fully compatible; use `v0.2.2` for new projects |

### L2 Series

| System version | Recommended SDK | Other compatible SDKs | Compatibility/notes |
| --- | --- | --- | --- |
| `v0.0.9` | `v0.2.2` | — | Fully compatible |

### Known Version Difference

With M1 system `v0.3.0` and SDK `v0.2.1`, basic control and state reading work, but some newer methods are unavailable or not fully validated. `UpdateCameraBitrate()` is protocol-incompatible in this combination and must not be used. Use SDK `v0.2.0`, or upgrade the M1 system to `v0.3.1`, when all validated interfaces are required.

The SDK validates methods against the `DeviceType` obtained during the handshake. Unsupported operations return `Errc::UnsupportedDeviceOperation`; see the [API Capability Matrix](docs/en/sdk_api_capability_en.md).

## Integration Recommendations

- Log `Version()`, `ProtocolVersion()`, `SystemVersion()`, and `GetDeviceInfo()` at application startup.
- Maintain explicit “connected and owns control” state before sending ordinary control commands.
- For asynchronous tasks, record the function result, send result, ACK, and final state instead of only one result layer.
- Copy callback data into an application-owned queue and perform file, database, or network work on application threads.
- When auto-reconnect is enabled in production, re-confirm control ownership and restore business state after reconnection.

## Package Layout

```text
.
├── README.md          # This guide
├── README_zh.md       # 中文用户指南
├── CHANGELOG.md       # Release history
├── docs/              # Chinese, English, and protocol documents
├── include/           # Public C++ headers
├── lib/<arch>/        # Prebuilt shared library
└── example/           # Example source and standalone build project
```
