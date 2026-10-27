# Robot SDK 用户指南

Robot SDK 是面向 M1、L2 系列机器人的 C++17 客户端库。它帮助应用程序连接机器人、接收状态和传感器数据，并按机型能力执行运动、设备和任务控制。

[English](README.md) · [文档中心](docs/zh/index_zh.md) · [API 参考](docs/zh/sdk_client_api_zh.md) · [版本记录](CHANGELOG.md)

## 开始前

### 环境要求

- Ubuntu 22.04（当前验证环境）
- x86_64 或 aarch64，且交付包的 `lib/<arch>/` 中包含对应动态库
- CMake 3.8+、支持 C++17 的编译器、Boost 1.74
- 可访问机器人的有线网络或 Wi-Fi AP

### 安全提醒

`control`、`sync`、`async`、`recharge` 等示例可能让机器人运动或改变设备状态。运行前请确保机器人周围无人员和障碍物、急停可用，并先用只读的 `quick_start` 示例验证连接。

### 接入前检查

1. 确认机器人系列和系统版本，并在[版本兼容表](#sdk-与机器人系统版本兼容性)中选择匹配的 SDK。
2. 执行 `uname -m`，确认输出与 `lib/<arch>/` 目录一致。
3. 确认机器人 IP、传输协议和端口。默认传输为 UDP；M1 Wi-Fi AP 常用 `192.168.234.1:8082`。
4. 若 APP 同时连接，请先阅读[控制权说明](docs/zh/sdk_control_ownership_zh.md)。连接成功不等于拥有控制权。

## 10 分钟完成首次连接

以下命令均在解压后的 SDK 根目录执行。

### 1. 检查交付包

```bash
uname -m
test -f "lib/$(uname -m)/librobot_sdk.so" && echo "SDK library found"
```

若未看到 `SDK library found`，说明当前交付包不包含本机架构的库，请更换匹配的交付包。

### 2. 连接机器人网络

- M1/L2 无线连接：主机接入机器人 Wi-Fi AP，常用目标地址为 `192.168.234.1`。
- 有线连接：主机和机器人运控主机需位于同一局域网，使用现场分配的机器人地址。
- 默认端口：UDP `8082`；显式选择 WebSocket 时使用 `8081`。

完整拓扑见[网络架构](docs/zh/sdk_network_zh.md)。可以先检查主机是否存在到目标 IP 的路由：

```bash
ip route get 192.168.234.1
```

### 3. 编译示例

```bash
cd example
mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j"$(nproc)"
```

示例工程会自动从当前 SDK 目录读取 `include/` 和 `lib/<arch>/`，无需复制头文件或手动设置 `LD_LIBRARY_PATH`。
完成后，下面的示例运行命令均在 `example/build/` 目录执行。

### 4. 先运行只读示例

```bash
./quick_start 192.168.234.1 8082
```

看到以下信息表示基础链路正常：

- 输出 SDK 与协议版本；
- 出现 `[OK] Connected`；
- 随后出现 `[OK] First robot state received`；
- 最后出现 `[OK] Disconnected cleanly`，且没有连接超时或动态库加载错误。

示例收到第一条机器人状态后会自动断开并退出。若连接成功但 5 秒内没有状态数据，程序会给出警告；若连接失败，请直接查看[故障排查](#故障排查)。

### 5. 再尝试交互控制

确认现场安全且已理解控制权机制后运行：

```bash
./control 192.168.234.1 8082
```

程序启动后输入 `h` 查看按键，输入 `o` 查看状态，输入 `q` 退出。若 SDK 是观察方，需要等待控制权可用后输入 `t` 申请控制权；最终结果以 `OnTakeControlAck()` 为准。

## 接入自己的 CMake 工程

将 SDK 解压路径传给你的工程，并以导入库方式链接：

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

以下命令在你自己的工程根目录执行；配置时也可以覆盖 SDK 路径：

```bash
cmake -S . -B build -DROBOT_SDK_ROOT=/opt/robot_sdk
cmake --build build -j"$(nproc)"
```

`example/quick_start.cpp` 是推荐的最小代码骨架。部署应用时需同时提供匹配架构的 `librobot_sdk.so`，并通过安装 RPATH 或系统动态库路径保证运行时能够找到它。

## 正确理解一次 SDK 调用

一个控制动作通常经过三层结果：

1. **函数返回值**：命令是否满足本地条件并成功提交发送。
2. **发送回调/控制 ACK**：网络发送是否完成、机器人是否接收并处理命令。
3. **状态或任务回调**：机器人是否真正进入目标状态、任务是否完成或失败。

因此，“函数返回成功”不等于“动作已经完成”。回充、离桩等长任务必须以 `OnTaskStateData()` 的状态和错误码为准。回调由 SDK I/O 线程触发，回调内只做轻量处理；不要在回调内调用同步模式接口。

## 示例怎么选

| 示例 | 用途 | 是否可能改变机器人状态 |
| --- | --- | :---: |
| `quick_start` | 最小连接、版本/设备信息和首条状态验证 | 否 |
| `data` | 连接并订阅状态、故障和传感器数据 | 否（会调整数据上报配置） |
| `control` | 键盘交互式综合控制 | 是 |
| `sync` | 同步连接和同步命令模式 | 是 |
| `async` | 异步连接、命令发送与回调 | 是 |
| `manual_reconnect` | 应用层手动重连 | 否 |
| `auto_reconnect` | SDK 自动重连 | 否 |
| `take_control` | APP/SDK 控制权切换 | 可能 |
| `camera_bitrate` | 相机码率调整 | 是 |
| `recharge` | 回充与离桩任务 | 是 |
| `led` | 状态 LED 模式和灯效 | 是 |
| `dock_pairing` | 充电桩扫描、连接、查询与断开 | 是 |

所有程序的基本参数形式均为：

```text
./<name> <robot_ip> <port> [example-specific arguments]
```

个别示例需要附加参数或键盘输入，直接不带参数运行即可查看用法。

## 文档地图

| 想完成的任务 | 文档 |
| --- | --- |
| 查连接参数、超时和重连 | [连接配置](docs/zh/sdk_connection_zh.md) |
| 查全部客户端接口 | [SDKClient API](docs/zh/sdk_client_api_zh.md) |
| 查结构体、枚举和单位 | [数据类型](docs/zh/sdk_type_zh.md) |
| 理解数据回调和命令 ACK | [回调参考](docs/zh/sdk_callback_zh.md) |
| 判断某机型是否支持接口 | [API 机型能力表](docs/zh/sdk_api_capability_zh.md) |
| 处理错误码 | [SDK 错误码](docs/zh/sdk_error_zh.md) |
| 处理 APP/SDK 抢占 | [控制权说明](docs/zh/sdk_control_ownership_zh.md) |
| 理解姿态切换和命令确认 | [运动状态与命令下发](docs/zh/sdk_state_zh.md) |
| 开发回充或离桩任务 | [任务指南](docs/zh/sdk_recharge_task_zh.md) · [任务错误码](docs/zh/sdk_recharge_errorcode_zh.md) |
| 配对充电桩 | [充电桩配对指南](docs/zh/sdk_dock_pairing_zh.md) |
| 控制状态 LED | [LED 控制指南](docs/zh/sdk_led_control_zh.md) |
| 现场问题定位 | [故障排查指南](docs/zh/sdk_troubleshooting_zh.md) |

按角色和开发阶段浏览全部文档，请进入[中文文档中心](docs/zh/index_zh.md)。

## 故障排查

| 现象 | 优先检查 |
| --- | --- |
| 程序启动时报 `librobot_sdk.so` 找不到 | `uname -m` 是否与 `lib/<arch>/` 一致；是否通过示例 CMake 构建 |
| 连接超时 | IP/端口是否匹配传输协议；主机路由、Wi-Fi/网线、机器人系统状态是否正常 |
| 已连接但控制命令无效 | SDK 是否为观察方；是否收到 `OnControlLost()`；机器人当前状态是否允许该命令 |
| 返回 `UnsupportedDeviceOperation` | 当前机型不支持该接口，查 API 机型能力表 |
| 函数返回成功但任务未完成 | 返回值仅代表提交/发送结果，继续检查 ACK、任务状态与 `error_code` |
| 开启自动重连后仍反复掉线 | 先排查物理链路和地址稳定性，再检查重连间隔与连接状态回调 |
| 回调卡住或同步调用超时 | 回调中不要执行耗时操作，也不要调用同步模式 SDK 接口 |

更完整的诊断顺序、信息采集模板和常见错误见[故障排查指南](docs/zh/sdk_troubleshooting_zh.md)。

## SDK 与机器人系统版本兼容性

“完全兼容”表示对应 SDK 已适配并验证该系统版本的公开接口；它不表示所有机型拥有相同硬件能力。新项目应优先使用推荐版本。

### M1 系列

| 系统版本 | 推荐 SDK | 其他兼容 SDK | 兼容级别/说明 |
| --- | --- | --- | --- |
| `v0.0.5` ~ `v0.1.0-B` | `v0.0.3` | `v0.0.2` | 完全兼容 |
| `v0.1.0-C` ~ `v0.2.4` | `v0.1.1` | `v0.0.4` ~ `v0.1.0` | 完全兼容，建议使用该阶段最新版本 |
| `v0.3.0` | `v0.2.0` | `v0.2.1` | `v0.2.0` 完全兼容；`v0.2.1` 部分兼容 |
| `v0.3.1` | `v0.2.2` | `v0.2.1` | 完全兼容，新项目推荐 `v0.2.2` |

### L2 系列

| 系统版本 | 推荐 SDK | 其他兼容 SDK | 兼容级别/说明 |
| --- | --- | --- | --- |
| `v0.0.9` | `v0.2.2` | — | 完全兼容 |

### 已知版本差异

M1 系统 `v0.3.0` 配合 SDK `v0.2.1` 时，基础控制和状态读取可用，但部分新增接口不可用或未完整验证；`UpdateCameraBitrate()` 的协议定义不兼容，请勿在该组合下调用。需要全部已验证接口时，请使用 SDK `v0.2.0`，或将 M1 系统升级至 `v0.3.1`。

SDK 会根据握手获得的 `DeviceType` 做能力校验。不支持的操作会返回 `Errc::UnsupportedDeviceOperation`，完整范围见[API 机型能力表](docs/zh/sdk_api_capability_zh.md)。

## 集成建议

- 应用启动时记录 `Version()`、`ProtocolVersion()`、`SystemVersion()` 和 `GetDeviceInfo()`，便于现场定位版本与机型问题。
- 在发送普通控制命令前维护明确的“已连接且拥有控制权”状态。
- 对异步任务同时记录函数返回值、发送结果、ACK 和最终状态，不要只记录一层结果。
- 将回调数据快速复制到应用自己的队列，由业务线程完成文件、数据库或网络操作。
- 生产环境启用自动重连时，仍需处理控制权重新确认和业务状态恢复。

## 安装包结构

```text
.
├── README.md          # English user guide
├── README_zh.md       # 本文档
├── CHANGELOG.md       # 版本记录
├── docs/              # 中文、英文和协议文档
├── include/           # C++ 公开头文件
├── lib/<arch>/        # 预编译动态库
└── example/           # 示例源码与独立构建脚本
```
