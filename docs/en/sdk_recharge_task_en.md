# Robot SDK — Recharge and Undock Task Usage Guide

## Overview

This document explains how to use the following four task control interfaces in `SDKClient`:

- `StartRechargeTask()`: Start the recharge task
- `StopRechargeTask()`: Stop the recharge task
- `StartUnDockTask()`: Start the undock task
- `StopUnDockTask()`: Stop the undock task

In addition to the interfaces themselves, this document also covers:

- Synchronous and asynchronous invocation modes
- The difference between control callbacks and task state callbacks
- State transitions for recharge and undock tasks
- How to use `MachineStatus` as an auxiliary signal during task execution

---

## Namespace

```cpp
namespace robot_sdk
```

---

## Interface Declarations

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

These interfaces are declared in:

- `/home/wenxi/code/robot_bak_sdk/include/robot_sdk/sdk_client.hpp`

---

## Parameters and Return Value

### Parameters

| Parameter | Type | Default | Description |
|:--|:--|:--|:--|
| `timeout_ms` | `int` | `0` | `0` means asynchronous mode; `> 0` means synchronous mode, in milliseconds |
| `handler` | `WriteHandler` | Empty callback | Used to receive the command sending result in asynchronous mode; not used in synchronous mode |

### Return Value

The return type is:

```cpp
std::error_code
```

Meaning:

- Success: the command was sent successfully, or in synchronous mode the protocol-level acknowledgment was received successfully
- Failure: sending failed, timed out, the connection was unavailable, or another SDK-level error occurred

**Note:**  
A successful return value only means the command sending or protocol-level acknowledgment succeeded. It **does not mean the task itself has completed**.

---

## Synchronous and Asynchronous Modes

### Asynchronous Mode

When `timeout_ms == 0`, the interface works in asynchronous mode:

- The function returns immediately
- The actual sending result is reported through `handler`

Example:

```cpp
client.StartRechargeTask(0, [](const std::error_code& ec, std::size_t) {
    if (ec) {
        std::cout << "Failed to send StartRechargeTask command: "
                  << ec.message() << std::endl;
    } else {
        std::cout << "StartRechargeTask command sent" << std::endl;
    }
});
```

### Synchronous Mode

When `timeout_ms > 0`, the interface works in synchronous mode:

- The function blocks until the send/ack result is available
- Timeout or failure is reported directly through the return value
- `handler` is not used in this mode

Example:

```cpp
std::error_code ec = client.StartRechargeTask(1000);
if (ec) {
    std::cout << "StartRechargeTask failed: " << ec.message() << std::endl;
}
```

---

## Callbacks and State Evaluation

For these four interfaces, it is recommended to evaluate execution using three levels of feedback.

| Level | Feedback | Purpose |
|:--|:--|:--|
| Command sending layer | Return value / `WriteHandler` | Determines whether the command was sent successfully |
| Control acknowledgment layer | `OnStartRechargeTask()` and related callbacks | Determines whether the robot acknowledged the corresponding start/stop task command |
| Task execution layer | `OnTaskStateData(const TaskStateInfo&)` | Determines whether the task started, is running, succeeded, failed, or was stopped |

### Related Control Callbacks

Declared in `/home/wenxi/code/robot_bak_sdk/include/robot_sdk/sdk_callback.hpp`:

```cpp
virtual void OnStartRechargeTask() {}
virtual void OnStopRechargeTask() {}
virtual void OnStartUnDockTask() {}
virtual void OnStopUnDockTask() {}
```

Recommended interpretation:

- `OnStartRechargeTask()`: the robot has acknowledged the start recharge task command
- `OnStopRechargeTask()`: the robot has acknowledged the stop recharge task command
- `OnStartUnDockTask()`: the robot has acknowledged the start undock task command
- `OnStopUnDockTask()`: the robot has acknowledged the stop undock task command

### Related Task State Callback

Task state callback in `IDataCallback`:

- `OnTaskStateData(const TaskStateInfo& info)`

`TaskStateInfo` is defined as follows:

```cpp
struct TaskStateInfo {
  TaskType task_type;
  TaskStatus task_status;
  std::string phase;
  uint32_t error_code;
};
```

Relevant task types:

- `TaskType::RECHARGING`
- `TaskType::UNDOCK`

Relevant task states:

- `TaskStatus::STARTING`
- `TaskStatus::RUNNING`
- `TaskStatus::SUCCESS`
- `TaskStatus::FAILURE`
- `TaskStatus::STOPPED`

---

## Using MachineStatus as an Auxiliary Signal

In addition to task state callbacks, you can also use `RobotState.machine_status` as an auxiliary signal to identify the current machine state.

Relevant states include:

- `MachineStatus::RECHARGE`
- `MachineStatus::UNDOCK`

Recommended interpretation:

- During the recharge task, `MachineStatus` should be `RECHARGE`
- During the undock task, `MachineStatus` should be `UNDOCK`

`MachineStatus` is best used as an auxiliary signal and should not replace `OnTaskStateData()` as the only source of truth.

---

## StartRechargeTask / StopRechargeTask

### StartRechargeTask - Start Recharge Task

**Description:**  
Requests the robot to start the recharge task.

**Typical usage scenarios:**

- The robot needs to enter a recharge workflow
- The host application wants the robot to enter the recharge task state

### Recharge Task State Transitions

According to the current robot behavior constraints, after `StartRechargeTask()` is called, the recharge task will only report:

- `STARTING`
- `RUNNING`
- `FAILURE`

In other words:

- The task may enter `STARTING -> RUNNING`
- If startup or execution fails, it enters `FAILURE`
- It **does not naturally reach `SUCCESS`**

If you receive:

- `task_type == TaskType::RECHARGING`
- `task_status == TaskStatus::FAILURE`

then you should inspect:

- `error_code`

### MachineStatus During Recharge

During the recharge task:

- `MachineStatus == RECHARGE`

### StopRechargeTask - Stop Recharge Task

**Description:**  
Requests the robot to stop the current recharge task.

After `StopRechargeTask()` is called, the recharge task state becomes:

- `STOPPED`

Therefore, the recommended criteria for confirming recharge task stop are:

1. `StopRechargeTask()` returns success, or the asynchronous send succeeds
2. `OnStopRechargeTask()` is received
3. `OnTaskStateData()` is received, and:
   - `task_type == TaskType::RECHARGING`
   - `task_status == TaskStatus::STOPPED`

### Usage Recommendations for the Recharge Task

Recommended evaluation order:

1. Check whether the command was sent successfully
2. Check whether `OnStartRechargeTask()` was received
3. Check whether task updates for `TaskType::RECHARGING` were received
4. If the task failed, inspect `error_code`
5. If the recharge task needs to be ended, call `StopRechargeTask()` and wait for `STOPPED`

---

## StartUnDockTask / StopUnDockTask

### StartUnDockTask - Start Undock Task

**Description:**  
Requests the robot to start the undock task.

**Typical usage scenarios:**

- The robot is in a dock-related state and needs to execute the undocking procedure
- The host application wants to trigger an undock workflow

### Undock Task State Transitions

According to the current robot behavior constraints, after `StartUnDockTask()` is called, the undock task will only report:

- `STARTING`
- `RUNNING`
- `SUCCESS`
- `FAILURE`

That means:

- Under normal conditions, the task may reach `STARTING -> RUNNING -> SUCCESS`
- If execution fails, it enters `FAILURE`

If you receive:

- `task_type == TaskType::UNDOCK`
- `task_status == TaskStatus::FAILURE`

then you should inspect:

- `error_code`

### MachineStatus During Undock

During the undock task:

- `MachineStatus == UNDOCK`

### StopUnDockTask - Stop Undock Task

**Description:**  
Requests the robot to stop the current undock task.

After `StopUnDockTask()` is called, the undock task state becomes:

- `STOPPED`

Therefore, the recommended criteria for confirming undock task stop are:

1. `StopUnDockTask()` returns success, or the asynchronous send succeeds
2. `OnStopUnDockTask()` is received
3. `OnTaskStateData()` is received, and:
   - `task_type == TaskType::UNDOCK`
   - `task_status == TaskStatus::STOPPED`

### Usage Recommendations for the Undock Task

Recommended evaluation order:

1. Check whether the command was sent successfully
2. Check whether `OnStartUnDockTask()` was received
3. Check whether task updates for `TaskType::UNDOCK` were received
4. If the final state is `SUCCESS`, the undock task completed successfully
5. If the final state is `FAILURE`, inspect `error_code`
6. If the task needs to be stopped midway, call `StopUnDockTask()` and wait for `STOPPED`

---

## Recommended Evaluation Logic

### 1. Confirm that the recharge task has successfully entered execution

It is recommended that at least two of the following conditions are met:

- `StartRechargeTask()` returned success / was sent successfully
- `OnStartRechargeTask()` was received
- `TaskType::RECHARGING` reported `STARTING` or `RUNNING`
- `MachineStatus == RECHARGE`

### 2. Recharge task failure

The following conditions are met:

- `task_type == TaskType::RECHARGING`
- `task_status == TaskStatus::FAILURE`

Then inspect:

- `error_code`

### 3. Recharge task successfully stopped

The following conditions are met:

- `OnStopRechargeTask()` was received
- `task_type == TaskType::RECHARGING`
- `task_status == TaskStatus::STOPPED`

### 4. Confirm that the undock task has successfully entered execution

It is recommended that at least two of the following conditions are met:

- `StartUnDockTask()` returned success / was sent successfully
- `OnStartUnDockTask()` was received
- `TaskType::UNDOCK` reported `STARTING` or `RUNNING`
- `MachineStatus == UNDOCK`

### 5. Undock task completed successfully

The following conditions are met:

- `task_type == TaskType::UNDOCK`
- `task_status == TaskStatus::SUCCESS`

### 6. Undock task failure

The following conditions are met:

- `task_type == TaskType::UNDOCK`
- `task_status == TaskStatus::FAILURE`

Then inspect:

- `error_code`

### 7. Undock task successfully stopped

The following conditions are met:

- `OnStopUnDockTask()` was received
- `task_type == TaskType::UNDOCK`
- `task_status == TaskStatus::STOPPED`

---

## Minimal Example

The following example shows how to:

- Register control callbacks
- Listen for task states
- Start/stop the recharge task
- Start/stop the undock task

```cpp
#include <iostream>
#include "robot_sdk/sdk_client.hpp"

using namespace robot_sdk;

class ControlCallback : public IControlCallback {
 public:
  void OnStartRechargeTask() override {
    std::cout << "[CTRL] Start recharge task acknowledged" << std::endl;
  }

  void OnStopRechargeTask() override {
    std::cout << "[CTRL] Stop recharge task acknowledged" << std::endl;
  }

  void OnStartUnDockTask() override {
    std::cout << "[CTRL] Start undock task acknowledged" << std::endl;
  }

  void OnStopUnDockTask() override {
    std::cout << "[CTRL] Stop undock task acknowledged" << std::endl;
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
      std::cout << "[STATE] MachineStatus = RECHARGE" << std::endl;
    } else if (state.machine_status == MachineStatus::UNDOCK) {
      std::cout << "[STATE] MachineStatus = UNDOCK" << std::endl;
    }
  }
};

int main() {
  SDKClient client;

  auto control_cb = std::make_shared<ControlCallback>();
  auto data_cb = std::make_shared<DataCallback>();

  client.SetControlCallback(control_cb);
  client.SetDataCallback(data_cb);

  // Call the following interfaces after connecting to the robot.

  std::error_code ec = client.StartRechargeTask(1000);
  if (ec) {
    std::cout << "StartRechargeTask failed: " << ec.message() << std::endl;
  }

  ec = client.StopRechargeTask(1000);
  if (ec) {
    std::cout << "StopRechargeTask failed: " << ec.message() << std::endl;
  }

  ec = client.StartUnDockTask(1000);
  if (ec) {
    std::cout << "StartUnDockTask failed: " << ec.message() << std::endl;
  }

  ec = client.StopUnDockTask(1000);
  if (ec) {
    std::cout << "StopUnDockTask failed: " << ec.message() << std::endl;
  }

  return 0;
}
```

You can also refer to the existing example in the repository:

- `/home/wenxi/code/robot_bak_sdk/example/recharge.cpp`

---

## Notes

### 1. ACK does not mean task completion

Receiving the following control callbacks:

- `OnStartRechargeTask()`
- `OnStopRechargeTask()`
- `OnStartUnDockTask()`
- `OnStopUnDockTask()`

only means that the robot acknowledged the corresponding control command. It **must not be treated as proof that the whole task has completed**.

The final result should be determined by `OnTaskStateData()`.

### 2. Inspect `error_code` first when the task fails

When the task state becomes `FAILURE`, you should inspect:

- `TaskStateInfo.error_code`

to determine the failure reason.

### 3. Use `MachineStatus` only as an auxiliary signal

It is recommended to use a combination of:

- control callbacks
- task state callbacks
- `MachineStatus`

Do not rely on only one signal to judge the entire task workflow.

### 4. The recharge task does not naturally report `SUCCESS`

According to the current robot behavior, after `StartRechargeTask()` is called, the task only reports:

- `STARTING`
- `RUNNING`
- `FAILURE`

If you need to end this task, call:

- `StopRechargeTask()`

and then wait for:

- `STOPPED`

### 5. The undock task may naturally end with `SUCCESS`

According to the current robot behavior, after `StartUnDockTask()` is called, the task may naturally reach:

- `SUCCESS`

It may also enter:

- `FAILURE`

If it is stopped manually midway, it enters:

- `STOPPED`

---

## Related Documents

- [SDKClient API Documentation](sdk_client_api_en.md) - Detailed client interface reference
- [Callback Reference](sdk_callback_en.md) - Control and data callback descriptions
- [Type Definitions](sdk_type_en.md) - `TaskType`, `TaskStatus`, `TaskStateInfo`, and related types
- [Connection Configuration Documentation](sdk_connection_en.md) - Connection and basic usage information
