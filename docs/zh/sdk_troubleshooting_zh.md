# Robot SDK 故障排查指南

本文提供一条从“程序能否启动”到“业务任务是否完成”的分层排查路径。请按顺序定位；上游问题未解决时，不要直接从业务接口或任务错误码开始猜测。

## 快速定位

| 故障阶段 | 典型现象 | 主要证据 |
| --- | --- | --- |
| 构建/加载 | 编译失败、程序无法启动、找不到 `.so` | CMake 输出、`uname -m`、`file`、`ldd` |
| 网络 | 无路由、目标地址不可达、频繁断连 | `ip addr`、`ip route`、物理链路状态 |
| 连接/握手 | `Connect()` 超时或状态未到 `CONNECTED` | 返回错误、错误回调、`ConnectionState` |
| 控制权/能力 | 已连接但普通控制无效 | `control_source`、控制权回调、`DeviceType` |
| 命令执行 | 发送成功但姿态或设备状态未变化 | 控制 ACK、`RobotState`、故障数据 |
| 长任务 | 回充/离桩未完成 | `TaskStateInfo`、任务 `error_code`、现场条件 |
| 回调/并发 | 卡住、超时、数据堆积 | 回调耗时、线程与同步调用位置 |

## 1. 检查主机架构和动态库

在 SDK 交付包根目录执行：

```bash
uname -m
find lib -maxdepth 2 -name 'librobot_sdk.so*' -print
file "lib/$(uname -m)/librobot_sdk.so"
```

`uname -m` 的输出必须与库目录一致，例如 `x86_64` 或 `aarch64`。若运行示例时报 `librobot_sdk.so: cannot open shared object file`，继续检查：

```bash
ldd example/build/data
```

- `librobot_sdk.so` 应解析到当前交付包的 `lib/<arch>/`。
- 其他条目出现 `not found` 时，应安装对应运行时依赖。
- 建议使用交付包自带的 `example/CMakeLists.txt` 构建，它会设置正确的运行时搜索路径。

若 CMake 缓存来自另一个 SDK 目录或架构，删除**当前示例的** `example/build/` 后重新配置；不要复用跨机器复制的构建目录。

## 2. 检查地址、路由和传输协议

默认构造的 `SDKClient` 使用 UDP。常用端口对应关系：

| 传输 | 端口 |
| --- | ---: |
| UDP（默认） | `8082` |
| WebSocket | `8081` |

M1 Wi-Fi AP 场景常用目标是 `192.168.234.1:8082`。有线网络和 L2 系列应以现场网络配置为准。

```bash
ip addr
ip route get <robot_ip>
```

重点确认：

1. 主机连接的是预期网卡或机器人 Wi-Fi，而不是通过错误的默认路由发送。
2. 目标 IP 是运控主机地址，不是导航主机地址。
3. 端口与构造 `SDKClient` 时选择的 `TransportProtocol` 一致。
4. VPN、容器网络和主机防火墙没有改写或阻断该路由。

`ping` 只能作为辅助信息：设备可能禁用 ICMP，因此 ping 不通不能单独证明 SDK 服务不可用。

## 3. 区分连接启动、握手完成和断线

同步连接的最小检查：

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

排查时同时保留三类信息：

- `Connect()` 的即时返回值；
- 构造函数错误回调或异步连接回调中的完整 `value()` 与 `message()`；
- `GetConnectionState()` 的状态变化。

只有状态到达 `ConnectionState::CONNECTED` 才表示连接与握手完成。连接成功后建议立即记录：

```cpp
const auto info = client.GetDeviceInfo();
std::cout << "sdk=" << client.Version()
          << ", protocol=" << client.ProtocolVersion()
          << ", system=" << client.SystemVersion()
          << ", device=" << robot_sdk::DeviceTypeName(info.device_type)
          << ", sn=" << info.sn << '\n';
```

这些字段可以快速识别 SDK/系统版本组合错误和机型能力差异。

## 4. 已连接但不能控制

按以下顺序检查：

1. **控制权**：连接成功不等于拥有控制权。查看 `RobotState.control_source`，并记录 `OnControlLost()`、`OnControlAvailable()` 和 `OnTakeControlAck()`。
2. **机型能力**：`Errc::UnsupportedDeviceOperation` 表示握手识别的机型不支持该接口，查 [API 机型能力表](sdk_api_capability_zh.md)。
3. **机器人状态**：急停、故障、机器状态或当前姿态可能阻止命令执行，结合 `RobotState` 和 `OnFaultData()` 判断。
4. **结果层级**：接口返回无错误可能只代表命令已提交发送；继续检查对应控制 ACK 和状态变化。

若 APP 正在连接或曾经接管，请按[控制权说明](sdk_control_ownership_zh.md)实现状态机。不要在未确认重新获得控制权时自动恢复速度命令。

## 5. 函数成功但动作没有完成

为一次命令记录以下时间线：

```text
调用时间 → 函数返回 → 发送回调 → 控制 ACK → RobotState/TaskStateInfo 变化
```

- 没有发送回调：检查连接是否在发送过程中断开。
- 有发送结果、没有 ACK：保留命令类型、时间戳和连接状态，检查机器人服务和协议版本。
- 有 ACK、状态未变化：检查当前姿态切换规则、急停、故障和控制权。
- 长任务进入失败态：使用任务状态中的业务 `error_code`，不要用 SDK 发送错误替代它。

运动姿态规则见[运动状态与命令下发](sdk_state_zh.md)。回充/离桩的终态判断见[任务指南](sdk_recharge_task_zh.md)和[任务错误码](sdk_recharge_errorcode_zh.md)。

## 6. 回充、离桩或标定异常

先确认 SDK 命令确实被接收，再检查现场条件：充电桩供电和连接、二维码识别、标定数据、机器人相对位置以及运动空间。

任务 ACK 只表示任务请求被接收，不表示任务完成。业务应持续处理 `OnTaskStateData()`，并保留：

- `task_type`；
- `task_status`；
- `error_code`；
- 同时段的 `MachineStatus`、故障和充电桩连接状态。

现象与现场原因对照见[回充、退桩与基站标定使用条件和故障说明](sdk_recharge_undock_calibration_conditions_zh.md)。

## 7. 回调卡住、同步调用超时或数据丢失

SDK 回调由 I/O 线程触发。回调中执行文件 I/O、数据库写入、复杂计算、等待锁或同步 SDK 调用，会阻塞后续网络事件。

推荐模式：

1. 回调中仅校验并复制数据。
2. 将数据放入有界、线程安全队列。
3. 由应用工作线程处理耗时逻辑。
4. 对队列积压、丢弃策略和单次处理耗时增加监控。

尤其不要在任何 SDK 回调中调用 `block=true` 或 `timeout_ms>0` 的同步操作。完整线程模型见[回调参考](sdk_callback_zh.md)。

## 8. 自动重连后的状态恢复

自动重连解决的是传输连接恢复，不会替应用恢复全部业务语义。连接重新到达 `CONNECTED` 后：

1. 重新读取并记录设备与系统版本信息。
2. 重新确认控制权，不能沿用断线前的 `has_control`。
3. 按需恢复数据上报配置。
4. 查询或等待机器人当前状态，再决定是否恢复业务。
5. 不要自动重放可能导致运动的历史命令。

频繁进入 `RECONNECTING` 通常应先排查物理链路、供电、IP 变化和路由，而不是单纯缩短重连间隔。

## 问题反馈模板

提交问题时复制并填写以下内容（敏感信息可脱敏）：

```text
SDK 版本：
协议版本：
机器人系统版本：
设备型号 / SN：
主机系统与架构：
传输协议 / 目标 IP / 端口：
是否启用自动重连：
是否有 APP 或其他 SDK 客户端同时连接：
问题发生时间：
预期行为：
实际行为：
函数返回 error_code：
发送回调 / 控制 ACK：
ConnectionState / RobotState / TaskStateInfo：
同时段故障信息：
最小复现步骤：
```

请附完整日志上下文，不要只截取最后一条错误。涉及运动问题时，还应说明机器人初始姿态、急停状态、场地和充电桩条件。

## 相关文档

- [连接配置](sdk_connection_zh.md)
- [SDK 错误码](sdk_error_zh.md)
- [控制权说明](sdk_control_ownership_zh.md)
- [回调参考](sdk_callback_zh.md)
- [API 机型能力表](sdk_api_capability_zh.md)
