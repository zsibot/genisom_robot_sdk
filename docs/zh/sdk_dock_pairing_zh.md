# 充电桩配对接口使用指南

本文介绍如何通过 Robot SDK 扫描、连接、查询及断开充电桩。

## 接口一览

| 接口或回调 | 类型 | 协议号 | 作用 |
| --- | --- | --- | --- |
| `SetDockScanConfig(bool enabled, int timeout_ms, WriteHandler handler)` | 主动调用 | `1008（target=302）` | 开启或关闭充电桩扫描。`enabled=true` 开启扫描，`false` 关闭扫描。 |
| `IControlCallback::OnDockScanConfig(bool enabled)` | 结果回调 | `1008（target=302）` | 通知扫描开关已经设置完成。 |
| `IDataCallback::OnDockScanResult(const DockScanResult& result)` | 被动推送 | `1018` | 接收机器人扫描到的充电桩列表。扫描期间可能多次触发。 |
| `ConnectDock(const DockConnectConfig& config, int timeout_ms, WriteHandler handler)` | 主动调用 | `1022` | 连接指定充电桩。当前通过 `config.address` 指定充电桩地址，结构体便于后续扩展。 |
| `IControlCallback::OnDockConnectionAck(const DockConnectionAck& ack)` | 结果回调 | `1022` | 通知连接或断开请求是否被机器人接受。该回调不代表操作已经完成。 |
| `IDataCallback::OnDockConnectionResult(const DockConnectionResult& result)` | 被动推送 | `1024` | 通知连接或断开操作的最终结果。 |
| `GetDockInfo(int timeout_ms, WriteHandler handler)` | 主动调用 | `1023` | 查询当前充电桩信息及连接、接触、充电、补光灯等状态。 |
| `IControlCallback::OnDockInfo(const DockInfo& info)` | 查询结果/状态回调 | `1023` | 返回充电桩信息。主动查询后会触发；充电桩状态变化时也可能收到该回调。 |
| `DisconnectDock(int timeout_ms, WriteHandler handler)` | 主动调用 | `1022` | 断开当前连接的充电桩。 |

其中，主动调用的返回值以及 `WriteHandler` 只表示请求是否成功发出，不表示充电桩操作已经成功。

## 推荐使用流程

### 1. 注册回调

在调用接口前，注册控制回调和数据回调：

```cpp
client->SetControlCallback(control_callback);
client->SetDataCallback(data_callback);
```

控制回调用于接收请求确认和信息查询结果，数据回调用于接收扫描结果以及连接终态。

### 2. 扫描充电桩

1. 调用 `SetDockScanConfig(true)` 开启扫描。
2. 收到 `OnDockScanConfig(true)`，表示扫描开关设置完成。
3. 等待 `OnDockScanResult()`，从 `result.devices` 中选择可连接的充电桩。
4. 保存所选设备的 `address`。
5. 调用 `SetDockScanConfig(false)` 关闭扫描。

扫描结果可能连续推送多次，建议应用层按充电桩地址更新设备列表，而不是简单追加。

### 3. 连接充电桩

```cpp
robot_sdk::DockConnectConfig config;
config.address = selected_address;

client->ConnectDock(config, 3000,
                    [](const std::error_code& ec, std::size_t) {
    // ec 表示请求发送阶段是否出错
});
```

连接过程依次关注以下结果：

1. `OnDockConnectionAck()`：检查 `ack.accepted`，确认机器人是否接受连接请求。
2. `OnDockConnectionResult()`：检查 `result.success`，确认连接是否最终成功。
3. 连接成功后调用 `GetDockInfo()`，并在 `OnDockInfo()` 中确认 `info.dock_connected`。

如果 `accepted` 或 `success` 为 `false`，可通过对应结果中的 `error_code` 定位原因；请求确认还会提供 `error_message`。

### 4. 查询充电桩信息

```cpp
client->GetDockInfo(3000,
                    [](const std::error_code& ec, std::size_t) {
    // 等待 OnDockInfo()
});
```

`OnDockInfo()` 中常用字段如下：

| 字段 | 含义 |
| --- | --- |
| `ble_address` | 充电桩地址 |
| `name` | 充电桩名称 |
| `dock_connected` | 是否已连接 |
| `has_contact` | 机器人是否与充电桩接触 |
| `charging_active` | 是否正在充电 |
| `light_switch_status` | 补光灯是否开启 |
| `fill_light_brightness` | 补光灯亮度 |
| `dock_sn` | 充电桩序列号 |
| `dock_version` | 充电桩版本 |
| `auto_fill_light_enabled` | 是否启用自动补光 |
| `error_code` / `error_message` | 错误信息 |

### 5. 断开充电桩

1. 调用 `DisconnectDock()`。
2. 收到 `OnDockConnectionAck()`，确认断开请求是否被接受。
3. 收到 `OnDockConnectionResult()`，确认断开操作的最终结果。
4. 可再次调用 `GetDockInfo()`，确认 `dock_connected` 已变为 `false`。

## 最小回调示例

```cpp
class ControlCallback final : public robot_sdk::IControlCallback {
 public:
  void OnDockScanConfig(bool enabled) override {
    std::cout << "dock scan: " << (enabled ? "enabled" : "disabled") << '\n';
  }

  void OnDockConnectionAck(
      const robot_sdk::DockConnectionAck& ack) override {
    std::cout << "request accepted: " << ack.accepted << '\n';
  }

  void OnDockInfo(const robot_sdk::DockInfo& info) override {
    std::cout << "dock connected: " << info.dock_connected << '\n';
  }
};

class DataCallback final : public robot_sdk::IDataCallback {
 public:
  void OnDockScanResult(
      const robot_sdk::DockScanResult& result) override {
    for (const auto& device : result.devices) {
      std::cout << device.address << " " << device.name << '\n';
    }
  }

  void OnDockConnectionResult(
      const robot_sdk::DockConnectionResult& result) override {
    std::cout << "operation success: " << result.success << '\n';
  }
};
```

## 按键测试示例

项目提供了 `dock_pairing` 示例，可通过按键完成完整测试：

```bash
cmake --build build --target dock_pairing -j4
./build/dock_pairing <机器人IP> <端口> [充电桩地址]
```

| 按键 | 操作 |
| --- | --- |
| `1` | 开启扫描 |
| `2` | 关闭扫描 |
| `3` | 显示最近一次扫描结果 |
| `4` | 按序号选择充电桩 |
| `5` | 连接选中的充电桩 |
| `6` | 查询充电桩信息 |
| `7` | 断开充电桩 |
| `h` | 显示帮助 |
| `q` | 退出程序 |

推荐按 `1 → 3 → 4 → 2 → 5 → 6` 的顺序完成配对测试。

## 使用注意事项

- `OnDockConnectionAck()` 只代表请求被接受，最终结果以 `OnDockConnectionResult()` 为准。
- 回调运行在 SDK 的 I/O 线程中，不要在回调里执行耗时操作；需要更新界面或业务状态时，应投递到应用自己的线程。
- 同一时间只发起一个连接或断开操作，收到终态结果后再执行下一次操作。
- 扫描结束或开始连接前，建议主动关闭扫描。
