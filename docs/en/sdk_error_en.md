# Robot SDK — Error Code Documentation

## Overview

Robot SDK interfaces return `std::error_code`. In addition to standard C++ error codes, the SDK defines `robot_sdk::Errc` for handshake, connection-control, and device-capability errors.

The definitions are available from:

```cpp
#include "robot_sdk/sdk_error.hpp"
```

`sdk_client.hpp` already includes this header, so applications using `SDKClient` normally do not need to include it again.

---

## SDK Error Codes

```cpp
enum class Errc {
  Success = 0,
  ShakeHandFailed = 10000,
  ProtocolMismatch = 10001,
  ControlledDenial = 10002,
  ConnectFailed = 10003,
  UnsupportedDeviceOperation = 10004,
  UnknownError = 19999
};
```

| Enum Value | Integer | Error Message | Description |
|:--|--:|:--|:--|
| `Errc::Success` | 0 | `Success` | Operation succeeded |
| `Errc::ShakeHandFailed` | 10000 | `Robot ShakeHand failed` | Protocol handshake did not complete after connecting |
| `Errc::ProtocolMismatch` | 10001 | `Robot Communication protocol version mismatch` | SDK and robot protocol versions do not match |
| `Errc::ControlledDenial` | 10002 | `Robot Controlled denial of service` | The robot is controlled by another client |
| `Errc::ConnectFailed` | 10003 | `Robot Connect failed` | The SDK failed to connect to the robot |
| `Errc::UnsupportedDeviceOperation` | 10004 | `Operation is not supported by the current device type` | The current `DeviceType` does not support the requested interface |
| `Errc::UnknownError` | 19999 | `Robot Unknown error` | Unclassified internal SDK error |

Compare errors using the enum instead of comparing raw integer values in application code.

---

## Checking Errors

### Checking for success

```cpp
std::error_code ec = client.Connect("192.168.234.1", "8082", true);
if (!ec) {
    // Success
}
```

### Comparing with an SDK error

`Errc` is registered as a `std::error_code` enum and can be compared directly:

```cpp
auto ec = client.SkWalk();
if (ec == robot_sdk::Errc::UnsupportedDeviceOperation) {
    // Same-knee posture is unsupported by this device type.
}
```

### Reading error details

```cpp
if (ec) {
    std::cerr << "error=" << ec.value()
              << ", category=" << ec.category().name()
              << ", message=" << ec.message() << std::endl;
}
```

The category name for SDK-specific errors is `Robot`.

---

## Standard Error-Condition Compatibility

`Errc::UnsupportedDeviceOperation` maps to the standard
`std::errc::operation_not_supported` condition. Both comparisons work:

```cpp
if (ec == robot_sdk::Errc::UnsupportedDeviceOperation) {
    // Recommended when preserving the specific SDK meaning.
}

if (ec == std::errc::operation_not_supported) {
    // Useful in generic code that handles standard error conditions.
}
```

Other SDK-specific errors currently have no standard error-condition mapping
and should be checked using the corresponding `Errc` value.

---

## Error-Code Construction

### `make_error_code`

```cpp
std::error_code make_error_code(Errc e);
```

Converts an `Errc` value to `std::error_code`:

```cpp
std::error_code ec =
    robot_sdk::make_error_code(robot_sdk::Errc::ConnectFailed);
```

Application code normally only needs to inspect errors returned by SDK
interfaces and does not need to construct them.

### `robot_category`

```cpp
const std::error_category& robot_category();
```

Returns the SDK error category:

```cpp
if (ec.category() == robot_sdk::robot_category()) {
    // SDK-specific error
}
```

---

## Device Adaptation Examples

All L2-series variants do not support the same-knee interface. The SDK
checks the device type before sending:

```cpp
auto ec = client.SkWalk();
if (ec == robot_sdk::Errc::UnsupportedDeviceOperation) {
    std::cerr << "Same-knee posture is unsupported by this device"
              << std::endl;
    return;
}
if (ec) {
    std::cerr << "SkWalk failed: " << ec.message() << std::endl;
}
```

The error is returned directly from the interface, and no command is sent to
the robot.

The `DeviceType::M1_AIR` and `DeviceType::M1F_AIR` medium-dog Air variants and
all L2-series variants do not support reversing the head and tail:

```cpp
auto ec = client.ReverseHeadTail();
if (ec == robot_sdk::Errc::UnsupportedDeviceOperation) {
    std::cerr << "Head-tail reversal is unsupported by this device"
              << std::endl;
}
```
