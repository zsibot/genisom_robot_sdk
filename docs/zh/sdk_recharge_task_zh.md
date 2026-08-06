# Robot SDK — 充电与离桩任务使用说明

## 概述

本文档说明 `SDKClient` 中与充电/离桩相关的 4 个任务控制接口的使用方法：

- `StartRechargeTask()`：启动充电任务
- `StopRechargeTask()`：停止充电任务
- `StartUnDockTask()`：启动脱离充电桩任务
- `StopUnDockTask()`：停止脱离充电桩任务

除接口本身外，本文档还说明：

- 同步/异步调用方式
- 控制回调与任务状态回调的区别
- 充电任务、离桩任务的状态流转
- `MachineStatus` 在任务过程中的辅助判断方法

---

## 命名空间

```cpp
namespace robot_sdk
```

---

## 接口声明

```cpp
std::error_code StartRechargeTask(
    int timeout_ms = 0,
    WriteHandler handler = [](const std::error_code&, std::size_t) {});

std::error_code StopRechargeTask(
    int timeout_ms = 0,
    WriteHandler handler = [](const std::error_code&, std::size_t) {});

std::error_code StartUnDockTask(
    int timeout_ms = 0,
    WriteHandler handler = [](const std::error_code&, std::size_t) {});

std::error_code StopUnDockTask(
    int timeout_ms = 0,
    WriteHandler handler = [](const std::error_code&, std::size_t) {});
```

这些接口声明位于：

- `/home/wenxi/code/robot_bak_sdk/include/robot_sdk/sdk_client.hpp`

---

## 参数与返回值

### 参数说明

| 参数名 | 类型 | 默认值 | 说明 |
|:--|:--|:--|:--|
| `timeout_ms` | `int` | `0` | `0` 表示异步模式；`> 0` 表示同步模式，单位毫秒 |
| `handler` | `WriteHandler` | 空回调 | 异步模式下用于接收命令发送结果；同步模式下不使用 |

### 返回值说明

返回值类型为：

```cpp
std::error_code
```

含义如下：

- 返回成功：表示命令已成功发送，或在同步模式下已成功收到协议层应答
- 返回失败：表示发送失败、超时、连接异常，或其他 SDK 层错误

**注意：**  
函数返回成功，仅表示“命令发送/协议层确认”成功，**不表示任务已经执行完成**。

---

## 同步模式与异步模式

### 异步模式

当 `timeout_ms == 0` 时，接口按异步方式发送命令：

- 函数立即返回
- 实际发送结果通过 `handler` 回调返回

示例：

```cpp
client.StartRechargeTask(0, [](const std::error_code& ec, std::size_t) {
    if (ec) {
        std::cout << "启动充电任务命令发送失败: " << ec.message() << std::endl;
    } else {
        std::cout << "启动充电任务命令已发送" << std::endl;
    }
});
```

### 同步模式

当 `timeout_ms > 0` 时，接口按同步方式发送命令：

- 函数阻塞等待发送/应答结果
- 超时或失败时直接通过返回值体现
- 此时 `handler` 不使用

示例：

```cpp
std::error_code ec = client.StartRechargeTask(1000);
if (ec) {
    std::cout << "启动充电任务失败: " << ec.message() << std::endl;
}
```

---

## 回调与状态判断

这 4 个接口建议结合 3 个层次的反馈来判断执行情况。

| 层次 | 反馈方式 | 用途 |
|:--|:--|:--|
| 命令发送层 | 返回值 / `WriteHandler` | 判断命令是否成功发送 |
| 控制确认层 | `OnStartRechargeTask()` 等 | 判断机器人是否确认收到对应开始/停止任务命令 |
| 任务执行层 | `OnTaskStateData(const TaskStateInfo&)` | 判断任务是否开始、运行、成功、失败或停止 |

### 相关控制回调

位于 `/home/wenxi/code/robot_bak_sdk/include/robot_sdk/sdk_callback.hpp`：

```cpp
virtual void OnStartRechargeTask() {}
virtual void OnStopRechargeTask() {}
virtual void OnStartUnDockTask() {}
virtual void OnStopUnDockTask() {}
```

建议理解为：

- `OnStartRechargeTask()`：机器人已确认开始充电任务命令
- `OnStopRechargeTask()`：机器人已确认停止充电任务命令
- `OnStartUnDockTask()`：机器人已确认开始脱离充电桩任务命令
- `OnStopUnDockTask()`：机器人已确认停止脱离充电桩任务命令

### 相关任务状态回调

`IDataCallback` 中的任务状态回调：

- `OnTaskStateData(const TaskStateInfo& info)`

其中 `TaskStateInfo` 定义如下：

```cpp
struct TaskStateInfo {
  TaskType task_type;
  TaskStatus task_status;
  std::string phase;
  uint32_t error_code;
};
```

相关任务类型：

- `TaskType::RECHARGING`
- `TaskType::UNDOCK`

相关任务状态：

- `TaskStatus::STARTING`
- `TaskStatus::RUNNING`
- `TaskStatus::SUCCESS`
- `TaskStatus::FAILURE`
- `TaskStatus::STOPPED`

---

## MachineStatus 辅助判断

除任务状态回调外，还可以结合 `RobotState.machine_status` 判断机器人当前处于哪种机器状态。

相关状态包括：

- `MachineStatus::RECHARGE`
- `MachineStatus::UNDOCK`

推荐理解：

- 充电任务过程中，`MachineStatus` 应处于 `RECHARGE`
- 离桩任务过程中，`MachineStatus` 应处于 `UNDOCK`

`MachineStatus` 适合作为辅助判断，不建议替代 `OnTaskStateData()` 作为唯一依据。

---

## StartRechargeTask / StopRechargeTask

### StartRechargeTask - 启动充电任务

**作用：**  
请求机器人启动充电任务。

**典型使用场景：**

- 机器人需要进入回充/充电流程
- 上位机希望让机器人进入充电任务态

### 充电任务状态流转

根据机器人当前行为约束，调用 `StartRechargeTask()` 后，任务状态只会出现：

- `STARTING`
- `RUNNING`
- `FAILURE`

即：

- 启动后可能进入 `STARTING -> RUNNING`
- 如果启动或执行失败，则进入 `FAILURE`
- **不会自然进入 `SUCCESS`**

如果收到：

- `task_type == TaskType::RECHARGING`
- `task_status == TaskStatus::FAILURE`

则应直接查看：

- `error_code`

### 充电过程中的 MachineStatus

在充电任务过程中：

- `MachineStatus == RECHARGE`

### StopRechargeTask - 停止充电任务

**作用：**  
请求机器人停止当前充电任务。

调用 `StopRechargeTask()` 后，充电任务状态会变为：

- `STOPPED`

因此，停止充电任务的推荐判断条件为：

1. `StopRechargeTask()` 返回成功，或异步发送成功
2. 收到 `OnStopRechargeTask()`
3. 收到 `OnTaskStateData()`，并且：
   - `task_type == TaskType::RECHARGING`
   - `task_status == TaskStatus::STOPPED`

### 充电任务使用建议

建议按如下顺序判断：

1. 命令是否成功发送
2. 是否收到 `OnStartRechargeTask()`
3. 是否收到 `TaskType::RECHARGING` 的状态更新
4. 若失败，则查看 `error_code`
5. 若需要结束充电任务，则调用 `StopRechargeTask()`，并等待 `STOPPED`

---

## StartUnDockTask / StopUnDockTask

### StartUnDockTask - 启动脱离充电桩任务

**作用：**  
请求机器人启动脱离充电桩任务。

**典型使用场景：**

- 机器人当前处于与充电桩相关的状态，需要执行离桩动作
- 上位机希望触发一次离桩流程

### 离桩任务状态流转

根据机器人当前行为约束，调用 `StartUnDockTask()` 后，任务状态只会出现：

- `STARTING`
- `RUNNING`
- `SUCCESS`
- `FAILURE`

也就是说：

- 正常情况下可能进入 `STARTING -> RUNNING -> SUCCESS`
- 如果执行失败，则进入 `FAILURE`

如果收到：

- `task_type == TaskType::UNDOCK`
- `task_status == TaskStatus::FAILURE`

则应查看：

- `error_code`

### 离桩过程中的 MachineStatus

在离桩任务过程中：

- `MachineStatus == UNDOCK`

### StopUnDockTask - 停止脱离充电桩任务

**作用：**  
请求机器人停止当前脱离充电桩任务。

调用 `StopUnDockTask()` 后，离桩任务状态会变为：

- `STOPPED`

因此，停止离桩任务的推荐判断条件为：

1. `StopUnDockTask()` 返回成功，或异步发送成功
2. 收到 `OnStopUnDockTask()`
3. 收到 `OnTaskStateData()`，并且：
   - `task_type == TaskType::UNDOCK`
   - `task_status == TaskStatus::STOPPED`

### 离桩任务使用建议

建议按如下顺序判断：

1. 命令是否成功发送
2. 是否收到 `OnStartUnDockTask()`
3. 是否收到 `TaskType::UNDOCK` 的状态更新
4. 若最终为 `SUCCESS`，表示离桩任务完成
5. 若最终为 `FAILURE`，查看 `error_code`
6. 若需要中途停止，则调用 `StopUnDockTask()`，并等待 `STOPPED`

---

## 推荐判定逻辑

### 1. 启动充电任务成功进入执行态

推荐至少满足以下条件中的 2 项：

- `StartRechargeTask()` 返回成功 / 发送成功
- 收到 `OnStartRechargeTask()`
- 收到 `TaskType::RECHARGING` 的 `STARTING` 或 `RUNNING`
- `MachineStatus == RECHARGE`

### 2. 充电任务失败

满足以下条件：

- `task_type == TaskType::RECHARGING`
- `task_status == TaskStatus::FAILURE`

然后读取：

- `error_code`

### 3. 停止充电任务成功

满足以下条件：

- 收到 `OnStopRechargeTask()`
- `task_type == TaskType::RECHARGING`
- `task_status == TaskStatus::STOPPED`

### 4. 启动离桩任务成功进入执行态

推荐至少满足以下条件中的 2 项：

- `StartUnDockTask()` 返回成功 / 发送成功
- 收到 `OnStartUnDockTask()`
- 收到 `TaskType::UNDOCK` 的 `STARTING` 或 `RUNNING`
- `MachineStatus == UNDOCK`

### 5. 离桩任务成功完成

满足以下条件：

- `task_type == TaskType::UNDOCK`
- `task_status == TaskStatus::SUCCESS`

### 6. 离桩任务失败

满足以下条件：

- `task_type == TaskType::UNDOCK`
- `task_status == TaskStatus::FAILURE`

然后读取：

- `error_code`

### 7. 停止离桩任务成功

满足以下条件：

- 收到 `OnStopUnDockTask()`
- `task_type == TaskType::UNDOCK`
- `task_status == TaskStatus::STOPPED`

---

## 最小示例

下面示例演示：

- 注册控制回调
- 监听任务状态
- 启动/停止充电任务
- 启动/停止离桩任务

```cpp
#include <iostream>
#include "robot_sdk/sdk_client.hpp"

using namespace robot_sdk;

class ControlCallback : public IControlCallback {
 public:
  void OnStartRechargeTask() override {
    std::cout << "[CTRL] 已确认开始充电任务" << std::endl;
  }

  void OnStopRechargeTask() override {
    std::cout << "[CTRL] 已确认停止充电任务" << std::endl;
  }

  void OnStartUnDockTask() override {
    std::cout << "[CTRL] 已确认开始离桩任务" << std::endl;
  }

  void OnStopUnDockTask() override {
    std::cout << "[CTRL] 已确认停止离桩任务" << std::endl;
  }
};

class DataCallback : public IDataCallback {
 public:
  void OnTaskStateData(const TaskStateInfo& info) override {
    if (info.task_type == TaskType::RECHARGING) {
      std::cout << "[TASK][RECHARGING] status="
                << static_cast<int>(info.task_status)
                << ", error_code=" << info.error_code << std::endl;
    }

    if (info.task_type == TaskType::UNDOCK) {
      std::cout << "[TASK][UNDOCK] status="
                << static_cast<int>(info.task_status)
                << ", error_code=" << info.error_code << std::endl;
    }
  }

  void OnRobotStateData(const RobotState& state) override {
    if (state.machine_status == MachineStatus::RECHARGE) {
      std::cout << "[STATE] 当前处于 RECHARGE" << std::endl;
    } else if (state.machine_status == MachineStatus::UNDOCK) {
      std::cout << "[STATE] 当前处于 UNDOCK" << std::endl;
    }
  }
};

int main() {
  SDKClient client;

  auto control_cb = std::make_shared<ControlCallback>();
  auto data_cb = std::make_shared<DataCallback>();

  client.SetControlCallback(control_cb);
  client.SetDataCallback(data_cb);

  // 连接机器人后再调用以下接口

  std::error_code ec = client.StartRechargeTask(1000);
  if (ec) {
    std::cout << "StartRechargeTask 失败: " << ec.message() << std::endl;
  }

  ec = client.StopRechargeTask(1000);
  if (ec) {
    std::cout << "StopRechargeTask 失败: " << ec.message() << std::endl;
  }

  ec = client.StartUnDockTask(1000);
  if (ec) {
    std::cout << "StartUnDockTask 失败: " << ec.message() << std::endl;
  }

  ec = client.StopUnDockTask(1000);
  if (ec) {
    std::cout << "StopUnDockTask 失败: " << ec.message() << std::endl;
  }

  return 0;
}
```

仓库中也可以参考现有示例：

- `/home/wenxi/code/robot_bak_sdk/example/recharge.cpp`

---

## 注意事项

### 1. ACK 不等于任务完成

收到以下控制回调时：

- `OnStartRechargeTask()`
- `OnStopRechargeTask()`
- `OnStartUnDockTask()`
- `OnStopUnDockTask()`

仅表示机器人已确认对应控制命令，**不能等价理解为整个任务已完成**。

最终应以 `OnTaskStateData()` 为准。

### 2. 失败时优先查看 `error_code`

当任务状态为 `FAILURE` 时，应优先读取：

- `TaskStateInfo.error_code`

用于定位任务失败原因。

### 3. `MachineStatus` 只作为辅助判断

推荐组合使用：

- 控制回调
- 任务状态回调
- `MachineStatus`

不要仅依赖某一个信号判断整个任务流程。

### 4. 充电任务不会自然上报 `SUCCESS`

根据当前机器人行为，`StartRechargeTask()` 后任务状态只会出现：

- `STARTING`
- `RUNNING`
- `FAILURE`

若需要结束该任务，应调用：

- `StopRechargeTask()`

随后等待：

- `STOPPED`

### 5. 离桩任务可能自然结束为 `SUCCESS`

根据当前机器人行为，`StartUnDockTask()` 后，任务可能自然到达：

- `SUCCESS`

也可能进入：

- `FAILURE`

若中途主动停止，则会进入：

- `STOPPED`

---

## 相关文档

- [SDKClient API 文档](sdk_client_api_zh.md) - 客户端接口详细说明
- [Callback 回调接口](sdk_callback_zh.md) - 控制回调与数据回调说明
- [类型定义文档](sdk_type_zh.md) - `TaskType`、`TaskStatus`、`TaskStateInfo` 等类型说明
- [连接配置文档](sdk_connection_zh.md) - 连接与基本使用方式说明
