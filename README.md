# RobotSDK

Welcome to Robot SDK! It provides a comprehensive set of tools, libraries, and documentation designed to help you develop advanced applications for robots.

- **[中文文档](README_zh.md)**

## 🔗 SDK-to-Platform Version Mapping

For easier maintenance, this document no longer uses a large “platform version × SDK version” matrix.  
Instead, it records **recommended version + usable versions + known differences**.

Please prefer the **recommended SDK version**. Only when you need to reuse an existing integration or intentionally fall back should you refer to the **usable SDK versions** and the difference notes.

**Compatibility Levels:**
- **Fully compatible**: The corresponding SDK version has been adapted and validated for the public interfaces on that platform version.
- **Partially compatible**: Core capabilities are available, but some newly added interfaces may be unavailable, behavior may differ, or known protocol incompatibilities may exist.

| Software Platform Version | Recommended SDK Version | Usable SDK Versions | Compatibility Level | Notes |
| --- | --- | --- | --- | --- |
| 0.0.5 ~ 0.1.0-B | 0.0.3 | 0.0.2 ~ 0.0.3 | Fully compatible | Early platform versions |
| 0.1.0-C ~ 0.2.4 | 0.1.1 | 0.0.4 ~ 0.1.1 | Fully compatible | Same compatibility stage; using the latest SDK in this range is recommended |
| 0.3.0 | 0.2.0 | 0.2.0, 0.2.1 | `0.2.0`: fully compatible; `0.2.1`: partially compatible | There are interface differences when using `0.2.1`; see details below |
| 0.3.1 | 0.2.1 | 0.2.1 | Fully compatible | Current latest record |

### ⚠️ Interface Difference Notes

#### Software platform `0.3.0` with SDK `0.2.1`

SDK `0.2.1` can be used for basic control scenarios on software platform `0.3.0`, but this is not a fully compatible combination.

Known differences:
- Core capabilities such as basic control and state reading work normally.
- Some interfaces newly added in SDK `0.2.1` are unavailable on software platform `0.3.0` or have not been fully validated.
- The `UpdateCameraBitrate` interface is not compatible with the protocol definition used by software platform `0.3.0`, so it is not recommended in this combination.
- If you need stable access to the full capability set, prefer SDK `0.2.0`.

**Maintenance Notes:**
- If a new version follows an existing compatibility relationship, prefer extending the platform-version range or usable SDK version range instead of returning to a large matrix.
- If a new version introduces protocol or interface changes, keep one mapping row and add the concrete limitations in the “Interface Difference Notes” section.
- If more fine-grained historical compatibility records are needed later, move the full compatibility matrix into a separate document and keep only this summary table and key differences in the README.


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
- **[Recharge and Undock Task Usage Guide](docs/en/sdk_recharge_task_en.md)**: Detailed instructions for `StartRechargeTask`, `StopRechargeTask`, `StartUnDockTask`, and `StopUnDockTask`.
- **[SDK Data Structures Documentation](docs/en/sdk_type_en.md)**: Detailed descriptions of SDK data structures.
- **[SDK State Transitions](docs/en/sdk_state_en.md)**: Detailed descriptions of SDK internal state transitions.
- **[LED Control Guide](docs/en/sdk_led_control_en.md)**: Customer-facing guidance for LED control modes, interfaces, and integration notes.
- **[Original Protocol Document](docs/protocol/Protocol-1.2.0.pdf)**: If you do not intend to control the machine via the SDK, please refer to this document.

## 📁 Repository Structure

```
.
├── example/          # Demo applications (C++)
├── docs_user/        # Detailed documentation files
├── include/          # C++ header files for the SDK
└── lib/              # Pre-compiled library files (.so) for different architectures
```
