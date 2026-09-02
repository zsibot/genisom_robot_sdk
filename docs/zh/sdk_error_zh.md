# Robot SDK — 错误码文档

## 概述

Robot SDK 的接口使用 `std::error_code` 返回执行结果。除 C++ 标准错误码外，SDK 通过 `robot_sdk::Errc` 提供握手、连接控制和机型能力相关的专用错误码。

相关定义位于：

```cpp
#include "robot_sdk/sdk_error.hpp"
```

`sdk_client.hpp` 已包含该头文件，因此使用 `SDKClient` 时通常不需要再次包含。

---

## SDK 错误码

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

| 枚举值 | 整数值 | 错误消息 | 说明 |
|:--|--:|:--|:--|
| `Errc::Success` | 0 | `Success` | 操作成功 |
| `Errc::ShakeHandFailed` | 10000 | `Robot ShakeHand failed` | 连接后未能完成协议握手 |
| `Errc::ProtocolMismatch` | 10001 | `Robot Communication protocol version mismatch` | SDK 与机器人协议版本不匹配 |
| `Errc::ControlledDenial` | 10002 | `Robot Controlled denial of service` | 机器人已被其他终端控制 |
| `Errc::ConnectFailed` | 10003 | `Robot Connect failed` | SDK 连接机器人失败 |
| `Errc::UnsupportedDeviceOperation` | 10004 | `Operation is not supported by the current device type` | 当前 `DeviceType` 不支持所调用的接口 |
| `Errc::UnknownError` | 19999 | `Robot Unknown error` | 无法归类的 SDK 内部错误 |

建议通过枚举比较错误，不要在业务代码中直接比较整数值。

---

## 判断错误

### 判断操作是否成功

```cpp
std::error_code ec = client.Connect("192.168.234.1", "8082", true);
if (!ec) {
    // 成功
}
```

### 与 SDK 错误码比较

`Errc` 已注册为 `std::error_code` 枚举，可直接比较：

```cpp
auto ec = client.SkWalk();
if (ec == robot_sdk::Errc::UnsupportedDeviceOperation) {
    // 当前机型不支持同膝接口
}
```

### 获取错误信息

```cpp
if (ec) {
    std::cerr << "error=" << ec.value()
              << ", category=" << ec.category().name()
              << ", message=" << ec.message() << std::endl;
}
```

SDK 专用错误码的错误类别名称为 `Robot`。

---

## 标准错误条件兼容

`Errc::UnsupportedDeviceOperation` 映射到标准错误条件 `std::errc::operation_not_supported`，以下两种写法都有效：

```cpp
if (ec == robot_sdk::Errc::UnsupportedDeviceOperation) {
    // 推荐：保留明确的 SDK 语义
}

if (ec == std::errc::operation_not_supported) {
    // 适合只处理标准错误条件的通用代码
}
```

其他 SDK 专用错误码目前没有映射到标准错误条件，应使用对应的 `Errc` 枚举判断。

---

## 错误码构造函数

### `make_error_code`

```cpp
std::error_code make_error_code(Errc e);
```

将 `Errc` 转换为 `std::error_code`：

```cpp
std::error_code ec =
    robot_sdk::make_error_code(robot_sdk::Errc::ConnectFailed);
```

通常业务代码只需判断 SDK 接口返回的错误，不需要主动构造。

### `robot_category`

```cpp
const std::error_category& robot_category();
```

返回 SDK 专用错误类别：

```cpp
if (ec.category() == robot_sdk::robot_category()) {
    // SDK 专用错误
}
```

---

## 机型适配示例

所有 L2 系列机型均不支持同膝接口。SDK 会在发送命令前完成判断：

```cpp
auto ec = client.SkWalk();
if (ec == robot_sdk::Errc::UnsupportedDeviceOperation) {
    std::cerr << "当前机型不支持同膝姿态" << std::endl;
    return;
}
if (ec) {
    std::cerr << "SkWalk failed: " << ec.message() << std::endl;
}
```

该错误在接口调用时直接返回，不会向机器人发送命令。

中狗 Air 版 `DeviceType::M1_AIR`、`DeviceType::M1F_AIR` 以及所有 L2 系列机型不支持头尾调换：

```cpp
auto ec = client.ReverseHeadTail();
if (ec == robot_sdk::Errc::UnsupportedDeviceOperation) {
    std::cerr << "当前机型不支持头尾调换" << std::endl;
}
```
