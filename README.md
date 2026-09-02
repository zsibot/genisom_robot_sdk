# RobotSDK

Welcome to Robot SDK! It provides a comprehensive set of tools, libraries, and documentation designed to help you develop advanced applications for robots.

- **[中文文档](README_zh.md)**

## 🔗 SDK-to-Robot System Version Mapping

System versions are maintained separately for each robot family. Before selecting an SDK version, confirm the robot family and its system version. The recommended version should be used for new integrations, while existing integrations may continue to use another compatible version listed below.

**Compatibility Levels:**

- **Fully compatible**: The corresponding SDK version has been adapted and validated for the public interfaces on that system version.
- **Partially compatible**: Core capabilities are available, but some interfaces are unavailable, have not been fully validated, or have known protocol differences.

### Medium Dog M1 Series

| System Version | Recommended SDK Version | Other Compatible SDK Versions | Compatibility Level | Notes |
| --- | --- | --- | --- | --- |
| `v0.0.5` ~ `v0.1.0-B` | `v0.0.3` | `v0.0.2` | Fully compatible | Early system versions |
| `v0.1.0-C` ~ `v0.2.4` | `v0.1.1` | `v0.0.4` ~ `v0.1.0` | Fully compatible | Use the latest SDK in this compatibility stage |
| `v0.3.0` | `v0.2.0` | `v0.2.1` | SDK `v0.2.0`: fully compatible; SDK `v0.2.1`: partially compatible | See the known SDK `v0.2.1` differences below |
| `v0.3.1` | `v0.2.2` | `v0.2.1` | Fully compatible | Both SDK `v0.2.1` and `v0.2.2` are supported; use `v0.2.2` for new integrations |

### Small Dog L2 Series

| System Version | Recommended SDK Version | Other Compatible SDK Versions | Compatibility Level | Notes |
| --- | --- | --- | --- | --- |
| `v0.0.9` | `v0.2.2` | — | Fully compatible | L2-series robots require SDK `v0.2.2` |

### ⚠️ Interface Difference Notes

#### Version Compatibility: M1 System `v0.3.0` + SDK `v0.2.1`

This combination supports basic control scenarios, but it is not fully compatible:

- Core capabilities such as basic control and state reading work normally.
- Some interfaces introduced in SDK `v0.2.1` are unavailable or have not been fully validated on M1 system `v0.3.0`.
- `UpdateCameraBitrate()` is incompatible with the protocol definition used by M1 system `v0.3.0`; do not call it with this combination.
- To use the complete set of validated interfaces, use SDK `v0.2.0` or upgrade the M1 system to `v0.3.1`.

#### Device Capabilities: M1 Series vs. L2 Series

SDK `v0.2.2` supports both the M1 and L2 series. However, “fully compatible” means that the SDK and the corresponding system version work correctly together; it does not mean that different robot models provide identical hardware or motion capabilities. The SDK validates capabilities using the device model obtained during the handshake and returns `Errc::UnsupportedDeviceOperation` when an operation is unavailable on that model.

For model-specific API availability, see the [SDK API Device Compatibility Guide](docs/en/sdk_api_capability_en.md).


## 🌟 Features

This SDK provides high-level control interfaces that allow you to flexibly interact with robots.

## 🚀 Quick Start

Follow the steps below to set up your development environment and run the included demo programs.

### System Requirements

- **Operating System**: Ubuntu 22.04
- **Build Tools**: CMake 3.8+, GCC 11.4
- **Dependencies**: Boost 1.74
- **Programming Language**: C++

### Installation and Setup

1. **Network Configuration**: Supports both wired and wireless connections to the dongle's network. See **[Network Architecture](docs/en/sdk_network_en.md)** for details.

2. **SDK Libraries**: C++ libraries (`.so` files) are included in the `lib/` directory and organized by architecture (`x86_64`, `aarch64`).

### Running Demo Programs

The `example/` directory contains C++ example code.

**C++ Demo:**

```bash
# Enter the C++ demo directory
cd example/

# Create build directory
mkdir build && cd build

# Configure and compile the project
cmake ..
make -j6

# Run the demo program
./data ${ip} 8082
```

## 📚 Documentation

For more in-depth information, please refer to the official documentation:

- **[API Reference](docs/en/sdk_client_api_en.md)**: Detailed descriptions of SDK functions.
- **[API Capability Matrix](docs/en/sdk_api_capability_en.md)**: Availability of every public API on each device type.
- **[SDK Error Code Documentation](docs/en/sdk_error_en.md)**: SDK-specific errors, comparison patterns, and standard error-condition mappings.
- **[Recharge and Undock Task Usage Guide](docs/en/sdk_recharge_task_en.md)**: Detailed instructions for `StartRechargeTask`, `StopRechargeTask`, `StartUnDockTask`, and `StopUnDockTask`.
- **[SDK Data Structures Documentation](docs/en/sdk_type_en.md)**: Detailed descriptions of SDK data structures.
- **[SDK State Transitions](docs/en/sdk_state_en.md)**: Detailed descriptions of SDK internal state transitions.
- **[SDK and APP Control Ownership](docs/en/sdk_control_ownership_en.md)**: Ownership priority, transition flows, permissions, and related callbacks.
- **[LED Control Guide](docs/en/sdk_led_control_en.md)**: Customer-facing guidance for LED control modes, interfaces, and integration notes.
- **[Original Protocol Document](docs/protocol/Protocol-1.3.0.pdf)**: If you do not intend to control the robot via the SDK, please refer to this document.

## 📁 Package Structure

```
.
├── example/          # Demo applications (C++)
├── docs/             # Detailed documentation files
├── include/          # C++ header files for the SDK
└── lib/              # Pre-compiled library files (.so) for different architectures
```
