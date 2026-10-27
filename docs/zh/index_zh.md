# Robot SDK 中文文档中心

这里汇总 Robot SDK 的用户文档。第一次使用请先完成交付包根目录 `README_zh.md` 中的“10 分钟完成首次连接”。

## 推荐阅读路径

### 第一次接入

1. [网络架构](sdk_network_zh.md)：确认接入方式、IP 和端口。
2. [连接配置](sdk_connection_zh.md)：理解同步/异步连接、超时和自动重连。
3. 运行 `example/quick_start.cpp`：先验证版本、连接和首条状态回调。
4. [控制权说明](sdk_control_ownership_zh.md)：在发送运动或任务命令前理解 APP/SDK 仲裁。
5. [回调参考](sdk_callback_zh.md)：区分发送结果、命令 ACK 和状态上报。
6. [故障排查](sdk_troubleshooting_zh.md)：连接或控制行为不符合预期时按层定位。

### 开发业务功能

| 场景 | 先读 | 再查 |
| --- | --- | --- |
| 状态监控、传感器采集 | [回调参考](sdk_callback_zh.md) | [数据类型](sdk_type_zh.md) |
| 姿态与移动控制 | [运动状态与命令下发](sdk_state_zh.md) | [SDKClient API](sdk_client_api_zh.md) |
| 多客户端控制 | [控制权说明](sdk_control_ownership_zh.md) | `example/take_control.cpp` |
| 回充、离桩 | [任务指南](sdk_recharge_task_zh.md) | [任务错误码](sdk_recharge_errorcode_zh.md) · [现场条件与故障](sdk_recharge_undock_calibration_conditions_zh.md) |
| 充电桩配对 | [充电桩配对指南](sdk_dock_pairing_zh.md) | `example/dock_pairing.cpp` |
| LED 灯效 | [LED 控制指南](sdk_led_control_zh.md) | `example/led.cpp` |
| 适配不同机型 | [API 机型能力表](sdk_api_capability_zh.md) | [错误码](sdk_error_zh.md) |

## 参考文档

| 文档 | 解决的问题 |
| --- | --- |
| [SDKClient API](sdk_client_api_zh.md) | 方法签名、参数、返回值、约束与示例 |
| [数据类型](sdk_type_zh.md) | 枚举、结构体、字段含义和单位 |
| [回调参考](sdk_callback_zh.md) | `IDataCallback` 与 `IControlCallback` 的触发语义 |
| [连接配置](sdk_connection_zh.md) | `ConnectionConfig`、状态和重连策略 |
| [SDK 错误码](sdk_error_zh.md) | SDK 本地错误的判断和标准错误条件映射 |
| [API 机型能力表](sdk_api_capability_zh.md) | 每个公开接口在 M1/L2 子型号上的可用性 |
| [网络架构](sdk_network_zh.md) | M1 网络组成、接入方式与端口 |
| [运动状态与命令下发](sdk_state_zh.md) | 姿态状态机和命令确认关系 |
| [控制权说明](sdk_control_ownership_zh.md) | 控制方、观察方、抢占与恢复流程 |
| [LED 控制指南](sdk_led_control_zh.md) | 自动/手动模式和灯效参数 |
| [回充与离桩任务指南](sdk_recharge_task_zh.md) | 长任务的启动、停止和终态判断 |
| [回充与离桩任务错误码](sdk_recharge_errorcode_zh.md) | `OnTaskStateData()` 中的业务失败原因 |
| [充电桩配对指南](sdk_dock_pairing_zh.md) | 扫描、连接、查询和断开充电桩 |
| [回充、退桩与标定现场指南](sdk_recharge_undock_calibration_conditions_zh.md) | 现场条件、正常流程和异常处理 |
| [充电接口版本差异](sdk_charging_api_diff_zh.md) | 历史充电相关接口的版本差异 |
| [故障排查指南](sdk_troubleshooting_zh.md) | 从构建、网络、握手到业务状态的诊断顺序 |
| [原始通信协议](protocol/Protocol-1.3.0.md) | 不通过本 SDK 直连机器人时使用的报文参考 |

## 三条通用规则

1. **先确认兼容性，再调用接口。** 握手成功后记录系统版本和设备型号，并检查机型能力表。
2. **区分发送成功和业务成功。** 控制命令看 ACK 和状态；长任务看任务状态及业务错误码。
3. **保持回调轻量。** 回调内只复制和校验数据，耗时处理交给应用线程；不要在回调内调用同步 SDK 接口。

## 示例索引

| 文件 | 重点 |
| --- | --- |
| `example/quick_start.cpp` | 最小只读连接与状态验证 |
| `example/data.cpp` | 状态、故障和传感器订阅 |
| `example/control.cpp` | 综合交互控制与设备信息 |
| `example/sync.cpp` | 同步模式 |
| `example/async.cpp` | 异步模式 |
| `example/manual_reconnect.cpp` | 手动重连 |
| `example/auto_reconnect.cpp` | 自动重连 |
| `example/take_control.cpp` | 控制权事件和申请 |
| `example/camera_bitrate.cpp` | 相机码率 |
| `example/recharge.cpp` | 回充与离桩 |
| `example/led.cpp` | 状态 LED |
| `example/dock_pairing.cpp` | 充电桩配对 |

## 提交问题前

请提供 SDK、协议和机器人系统版本，设备型号和 SN，主机架构，传输协议与目标地址，完整错误信息，以及能够复现问题的最小调用序列。可直接复制[故障排查指南中的信息模板](sdk_troubleshooting_zh.md#问题反馈模板)。
