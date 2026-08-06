# Robot SDK 充电/离桩接口差异说明（客户简版）

本文用于帮助客户快速理解两套 SDK 在**充电、离桩、状态获取**上的主要差异。

---

## 1. 一句话结论

这两套接口**不是简单改名**，而是**控制模型变了**：

- **旧版 SDK**：偏“**自主回充流程控制**”
- **新版 SDK**：偏“**机器人任务控制**”

所以客户迁移时，**不能只替换接口名，还需要调整状态判断方式**。

---

## 2. 客户最关心的接口对应关系

| 客户需求 | 旧版 SDK | 新版 SDK | 说明 |
|---|---|---|---|
| 启动充电 | `StartCharging()` | `StartRechargeTask()` | **最接近，但不等价** |
| 停止充电 | `ExitCharging()` / `ResetAllChargingTasks()` | `StopRechargeTask()` | 旧版更偏“退出/重置流程” |
| 执行离桩 | `ExitCharging()` | `StartUnDockTask()` | **仅近似对应** |
| 停止离桩 | 无完全等价接口 | `StopUnDockTask()` | 新版把离桩单独做成任务 |
| 查询状态 | `GetArcAlgStatus()` / `OnArcAlgStatus()` | `OnTaskStateData()` + `machine_status` | 状态判断方式已变化 |

---

## 3. 两套 SDK 的核心区别

### 旧版 SDK

旧版主要围绕以下接口：

- `StartCharging()`
- `ExitCharging()`
- `GetArcAlgStatus()`
- `ResetAllChargingTasks()`
- `OneKeyGoCharging()`

特点：

- 更偏**自主回充流程**
- 依赖 **nav_mapping** 侧能力
- 更关注“当前回充算法进行到哪一步”

常见状态判断方式：

- `ArcAlgStatus`
- `OnArcAlgStatus(...)`
- `OnExitCharging(...)`
- `OnOneKeyGoCharging(...)`

---

### 新版 SDK（robot_bak_sdk）

新版主要围绕以下接口：

- `StartRechargeTask()`
- `StopRechargeTask()`
- `StartUnDockTask()`
- `StopUnDockTask()`

特点：

- 更偏**机器人任务控制**
- 把“充电”和“离桩”分别建模成任务
- 更关注“任务是否启动、运行、成功、失败、停止”

常见状态判断方式：

- `OnStartRechargeTask()` / `OnStopRechargeTask()`
- `OnStartUnDockTask()` / `OnStopUnDockTask()`
- `OnTaskStateData()`
- `machine_status`

---

## 4. 客户需要特别注意的 3 点

### 4.1 `StartCharging()` 不是简单等于 `StartRechargeTask()`

- 旧版 `StartCharging()` 是一个更高层的“启动自主回充”接口
- 新版 `StartRechargeTask()` 是“启动充电任务”接口

因此：

> `StartRechargeTask()` 是新版里**最接近** `StartCharging()` 的接口，但两者不是完全等价。

---

### 4.2 `ExitCharging()` 不能直接理解成 `StartUnDockTask()`

旧版 `ExitCharging()` 更偏：

- 退出当前充电流程

新版则拆成了两类能力：

- `StopRechargeTask()`：停止充电任务
- `StartUnDockTask()`：执行离桩任务

因此：

> 旧版一个 `ExitCharging()`，在新版里通常要根据客户真实业务目的拆开看。

---

### 4.3 状态判断方式变了

旧版主要看：

- `ArcAlgStatus`

新版主要看：

- `OnTaskStateData()`
- `TaskType / TaskStatus`
- `machine_status`

因此：

> 旧版基于 `ArcAlgStatus` 的判断逻辑，不能直接平移到新版。

---

### 4.4 新版状态建议这样判断

客户在新版中可按下面方式理解状态：

#### 充电任务是否已开始

建议至少同时关注 2 个信号：

1. 收到 `OnStartRechargeTask()`
2. 收到 `OnTaskStateData()`，且 `task_type == RECHARGING`

如果进一步看到：

- `task_status == STARTING` 或 `RUNNING`
- `machine_status == RECHARGE`

通常就可以认为：

> 充电任务已经进入执行过程。

#### 充电任务是否成功/失败/停止

- `task_status == SUCCESS`：任务成功完成
- `task_status == FAILURE`：任务执行失败
- `task_status == STOPPED`：任务已被停止

#### 离桩任务是否已开始

建议至少同时关注 2 个信号：

1. 收到 `OnStartUnDockTask()`
2. 收到 `OnTaskStateData()`，且 `task_type == UNDOCK`

如果进一步看到：

- `task_status == STARTING` 或 `RUNNING`
- `machine_status == UNDOCK`

通常就可以认为：

> 离桩任务已经进入执行过程。

#### 离桩任务是否成功/失败/停止

- `task_status == SUCCESS`：离桩完成
- `task_status == FAILURE`：离桩失败
- `task_status == STOPPED`：离桩任务已停止

#### 一个简单原则

> **以 `OnTaskStateData()` 为主，以 `machine_status` 为辅。**

也就是说：

- `OnTaskStateData()` 用来判断任务结果
- `machine_status` 用来辅助确认当前机器所处状态

---

## 5. 给客户的迁移建议

建议客户迁移时按下面方式理解：

1. **如果旧代码是“开始充电”** → 优先看 `StartRechargeTask()`
2. **如果旧代码是“退出充电”** → 先区分是“停止充电”还是“执行离桩”
3. **如果旧代码依赖 `ArcAlgStatus`** → 新版改为关注 `OnTaskStateData()` 和 `machine_status`
4. **不要只改接口名** → 还要同步修改状态判断逻辑

---
