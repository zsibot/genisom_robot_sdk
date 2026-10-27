# Charging Dock Pairing API Guide

This guide explains how to scan for, connect to, query, and disconnect from a charging dock through the Robot SDK.

## API Overview

| API or callback | Type | Protocol ID | Purpose |
| --- | --- | --- | --- |
| `SetDockScanConfig(bool enabled, int timeout_ms, WriteHandler handler)` | Active call | `1008 (target=302)` | Starts or stops charging dock scanning. Set `enabled` to `true` to start scanning and `false` to stop it. |
| `IControlCallback::OnDockScanConfig(bool enabled)` | Result callback | `1008 (target=302)` | Confirms that the scan setting has been applied. |
| `IDataCallback::OnDockScanResult(const DockScanResult& result)` | Passive push | `1018` | Receives the charging docks discovered by the robot. It may be called multiple times while scanning. |
| `ConnectDock(const DockConnectConfig& config, int timeout_ms, WriteHandler handler)` | Active call | `1022` | Connects to the specified charging dock. The dock is currently selected through `config.address`; the structure allows future extension. |
| `IControlCallback::OnDockConnectionAck(const DockConnectionAck& ack)` | Result callback | `1022` | Reports whether the robot accepted a connect or disconnect request. It does not mean that the operation has completed. |
| `IDataCallback::OnDockConnectionResult(const DockConnectionResult& result)` | Passive push | `1024` | Reports the final result of a connect or disconnect operation. |
| `GetDockInfo(int timeout_ms, WriteHandler handler)` | Active call | `1023` | Queries charging dock information and its connection, contact, charging, and fill-light states. |
| `IControlCallback::OnDockInfo(const DockInfo& info)` | Query result/status callback | `1023` | Returns charging dock information. It is called after an active query and may also be called when the dock state changes. |
| `DisconnectDock(int timeout_ms, WriteHandler handler)` | Active call | `1022` | Disconnects the currently connected charging dock. |

The return value of an active API call and its `WriteHandler` only indicate whether the request was sent successfully. They do not indicate that the charging dock operation succeeded.

## Recommended Workflow

### 1. Register the callbacks

Register both control and data callbacks before calling the charging dock APIs:

```cpp
client->SetControlCallback(control_callback);
client->SetDataCallback(data_callback);
```

The control callback receives request acknowledgments and dock information. The data callback receives scan results and final connection results.

### 2. Scan for charging docks

1. Call `SetDockScanConfig(true)` to start scanning.
2. Wait for `OnDockScanConfig(true)` to confirm that scanning has been enabled.
3. Wait for `OnDockScanResult()` and select a connectable dock from `result.devices`.
4. Save the selected device's `address`.
5. Call `SetDockScanConfig(false)` to stop scanning.

Scan results may be pushed repeatedly. The application should update its device list by dock address instead of simply appending every result.

### 3. Connect to a charging dock

```cpp
robot_sdk::DockConnectConfig config;
config.address = selected_address;

client->ConnectDock(config, 3000,
                    [](const std::error_code& ec, std::size_t) {
    // ec indicates whether an error occurred while sending the request.
});
```

Handle the connection results in this order:

1. In `OnDockConnectionAck()`, check `ack.accepted` to determine whether the robot accepted the request.
2. In `OnDockConnectionResult()`, check `result.success` to determine whether the connection finally succeeded.
3. After a successful connection, call `GetDockInfo()` and confirm `info.dock_connected` in `OnDockInfo()`.

If `accepted` or `success` is `false`, use the corresponding `error_code` to diagnose the failure. The request acknowledgment also provides `error_message`.

### 4. Query charging dock information

```cpp
client->GetDockInfo(3000,
                    [](const std::error_code& ec, std::size_t) {
    // Wait for OnDockInfo().
});
```

Common fields received through `OnDockInfo()` are listed below:

| Field | Description |
| --- | --- |
| `ble_address` | Charging dock address |
| `name` | Charging dock name |
| `dock_connected` | Whether the dock is connected |
| `has_contact` | Whether the robot is physically in contact with the dock |
| `charging_active` | Whether charging is active |
| `light_switch_status` | Whether the fill light is on |
| `fill_light_brightness` | Fill-light brightness |
| `dock_sn` | Charging dock serial number |
| `dock_version` | Charging dock version |
| `auto_fill_light_enabled` | Whether automatic fill light is enabled |
| `error_code` / `error_message` | Error information |

### 5. Disconnect from the charging dock

1. Call `DisconnectDock()`.
2. Wait for `OnDockConnectionAck()` and check whether the disconnect request was accepted.
3. Wait for `OnDockConnectionResult()` and check the final result.
4. Optionally call `GetDockInfo()` again and confirm that `dock_connected` is `false`.

## Minimal Callback Example

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

## Keyboard-Driven Test Example

The project includes the `dock_pairing` example for interactive testing:

```bash
cmake --build build --target dock_pairing -j4
./build/dock_pairing <robot_ip> <port> [dock_address]
```

| Key | Action |
| --- | --- |
| `1` | Start scanning |
| `2` | Stop scanning |
| `3` | Show the latest scan results |
| `4` | Select a charging dock by index |
| `5` | Connect to the selected charging dock |
| `6` | Query charging dock information |
| `7` | Disconnect from the charging dock |
| `h` | Show help |
| `q` | Quit |

For a complete pairing test, use the keys in this order: `1 → 3 → 4 → 2 → 5 → 6`.

## Notes

- `OnDockConnectionAck()` only indicates that the request was accepted. Use `OnDockConnectionResult()` as the final result.
- SDK callbacks run on the I/O thread. Keep callback processing lightweight and dispatch UI updates or time-consuming work to an application-owned thread.
- Start only one connect or disconnect operation at a time. Wait for its final result before starting the next operation.
- Stop scanning before starting a connection operation.
