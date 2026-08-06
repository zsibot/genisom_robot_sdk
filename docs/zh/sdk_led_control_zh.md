# LED 控制接口说明（客户版）

## 1. 文档目的

本文档面向客户说明 Robot SDK 中与 **LED/灯光控制** 相关的接口能力、控制模式、回调方式以及推荐使用方法，便于客户快速完成状态灯联调。

> 说明：SDK 中实际存在两类“灯光”能力：
> 1. **补光灯**：前补光灯、后补光灯、自动补光灯模式；
> 2. **状态 LED**：LED 自动/手动模式、LED 灯效控制。
>
> 如果需求是“设备状态灯/氛围灯/告警灯”，建议重点使用本文介绍的 **状态 LED 接口**；补光灯接口更偏向照明用途。

---

## 2. 能力总览

### 2.1 状态 LED 相关核心接口

SDK 当前提供 3 个状态 LED 核心接口：

1. `SetLedAutoMode(bool auto_mode)`：设置 LED 为自动模式或手动模式；
2. `GetLedAutoMode()`：查询当前 LED 是否处于自动模式；
3. `SetLedCommand(const LedCommand& cmd)`：设置 LED 分组、灯效、颜色与周期。

### 2.2 相关回调

与上述接口对应的控制回调如下：

1. `OnSetLedAutoMode(const LedAutoModeAck& ack)`
2. `OnGetLedAutoMode(const LedAutoModeAck& ack)`
3. `OnSetLedCommand(const LedCommandAck& ack)`

这些回调表示：**机器人/平台已收到该控制命令并返回应答**。

---

## 3. 两类灯光接口的区别

### 3.1 补光灯接口（照明）

补光灯接口包括：

- `FrontLight(bool on)`：前补光灯开关
- `BackLight(bool on)`：后补光灯开关
- `AutoModeLight(bool on)`：自动补光灯模式开关

补光灯状态还会体现在机器人状态数据中：

- `RobotState::front_fill_light`
- `RobotState::back_fill_light`
- `RobotState::auto_mode_light`

> 注意：
> - `FrontLight()` / `BackLight()` 在现有文档定义中，设置后会关闭补光灯自动模式；
> - 这组接口主要用于补光照明，不建议替代状态 LED 使用。

### 3.2 状态 LED 接口（状态/告警/任务灯效）

状态 LED 接口包括：

- `SetLedAutoMode()`
- `GetLedAutoMode()`
- `SetLedCommand()`

这组接口用于控制设备的状态灯效，更适合承载：

- 故障/急停提示
- 告警提示
- 低电量提示
- 任务执行状态
- 正常运行状态
- 用户自定义事件提示

---

## 4. 控制模式说明

### 4.1 手动模式

**定义：**
用户完全接管状态灯控制权，状态灯不再接受狗内部平台软件管理，用户通过控制接口直接控制灯效。

**推荐控制方式：**
1. 调用 `SetLedAutoMode(false)` 切换到手动模式；
2. 等待 `OnSetLedAutoMode()` 确认当前模式；
3. 调用 `SetLedCommand()` 下发具体灯效。

**特点：**
- 用户控制优先；
- 狗内部平台软件不再主动按照设备状态切换状态灯；
- 适合调试、演示、自定义灯语、特殊业务接管场景。

### 4.2 自动模式

**定义：**
状态灯由狗内部平台软件根据设备当前状态或事件统一管理。

在自动模式下，用户通过 `SetLedCommand()` 下发的灯效，建议按“**事件触发**”理解，而不是“绝对覆盖”。最终显示效果由狗内部平台软件策略统一仲裁。

**推荐优先级（按当前需求口径整理）：**

| 优先级 | 场景 |
|:--|:--|
| 1 | 系统故障、急停 |
| 2 | 告警 |
| 3 | 低电量 |
| 4 | 用户可控制事件 |
| 5 | 任务执行状态灯 |
| 6 | 正常运行时灯态 |

**自动模式下的建议理解：**
- 更高优先级事件出现时，应覆盖低优先级灯效；
- 用户下发灯效在自动模式中属于“可控事件”层级；
- 用户灯效是否立即生效、持续多久、是否被更高优先级打断，应由狗内部平台软件策略决定。
- 如果用户需要关闭*用户可控制事件*，调用 `SetLedCommand(LedId::ALL, LedEffect::OFF)` 即可。

> 重要说明：
> 上述优先级应理解为 **平台/设备侧业务规则**，而不是 SDK 本地逻辑。

---

## 5. 接口明细

### 5.1 SetLedAutoMode

```cpp
std::error_code SetLedAutoMode(
    bool auto_mode, int timeout_ms = 0,
    WriteHandler handler = [](const std::error_code&, std::size_t) {});
```

**用途：**
设置状态 LED 为自动模式或手动模式。

**参数说明：**

| 参数 | 类型 | 说明 |
|:--|:--|:--|
| `auto_mode` | `bool` | `true`：自动模式；`false`：手动模式 |
| `timeout_ms` | `int` | `0` 表示异步发送；大于 `0` 表示同步等待发送完成 |
| `handler` | `WriteHandler` | 异步模式下的发送结果回调 |

**返回：**
- 返回值表示 SDK 命令发送流程是否正常；
- 设备应答结果通过 `OnSetLedAutoMode()` 获取。

### 5.2 GetLedAutoMode

```cpp
std::error_code GetLedAutoMode(
    int timeout_ms = 0,
    WriteHandler handler = [](const std::error_code&, std::size_t) {});
```

**用途：**
查询当前状态 LED 是否处于自动模式。

**结果获取方式：**
通过 `OnGetLedAutoMode(const LedAutoModeAck& ack)` 获取查询结果。

### 5.3 SetLedCommand

```cpp
std::error_code SetLedCommand(
    const LedCommand& cmd, int timeout_ms = 0,
    WriteHandler handler = [](const std::error_code&, std::size_t) {});
```

**用途：**
设置 LED 分组、灯效、颜色和周期。

**参数结构：**

```cpp
struct LedCommand {
  LedId id;
  LedEffect effect;
  LedColor color;
  uint32_t duration_ms;
};
```

**返回：**
- 当 `cmd.id == LedId::UNKNOWN` 或 `cmd.effect == LedEffect::UNKNOWN` 时，SDK 会直接返回 `std::errc::invalid_argument`；
- 其他情况下，返回值表示发送流程结果；
- 设备应答内容通过 `OnSetLedCommand(const LedCommandAck& ack)` 获取。

---

## 6. 数据类型说明

### 6.1 LedId

```cpp
enum class LedId {
  UNKNOWN = 0,
  ALL,
  FRONT,
  BACK,
};
```

| 枚举值 | 含义 |
|:--|:--|
| `ALL` | 全部 LED |
| `FRONT` | 前部 LED |
| `BACK` | 后部 LED |

### 6.2 LedEffect

```cpp
enum class LedEffect {
  UNKNOWN = 0,
  OFF,
  ON,
  BREATH,
  BLINK,
  BLINK_TRANSIENT,
};
```

| 枚举值 | 协议值 | 含义 |
|:--|:--|:--|
| `OFF` | `off` | 关闭 |
| `ON` | `on` | 常亮 |
| `BREATH` | `breath` | 呼吸 |
| `BLINK` | `blink` | 闪烁 |
| `BLINK_TRANSIENT` | `blink_transient` | 瞬时闪烁 |

### 6.3 LedColor

```cpp
struct LedColor {
  uint8_t r;
  uint8_t g;
  uint8_t b;
  uint8_t a;
};
```

颜色通道均为 `0~255`。

### 6.4 duration_ms

`duration_ms` 表示灯效周期或闪烁间隔，单位为毫秒。

建议约定：
- `OFF` / `ON`：通常可传 `0`；
- `BREATH` / `BLINK` / `BLINK_TRANSIENT`：建议传实际周期值，例如 `300ms`、`1000ms`。

---

## 7. 回调与同步/异步使用说明

### 7.1 发送回调与控制回调的区别

SDK 中有两类结果：

1. **发送结果**：通过 `handler` 返回，表示命令是否成功发出；
2. **设备控制应答**：通过 `IControlCallback` 返回，表示设备侧对该命令有应答。

对于 LED 相关接口，客户更应该关注：

- `OnSetLedAutoMode()`
- `OnGetLedAutoMode()`
- `OnSetLedCommand()`

### 7.2 timeout_ms 的实际含义

当前 SDK 实现中，`timeout_ms > 0` 的“同步模式”主要是 **等待命令发送完成或超时**，并不等价于“灯效已经执行完成”。

因此建议：
- 如果业务需要确认模式切换/查询结果，仍以对应控制回调为准；
- 如果业务需要确认实际显示效果，建议在设备侧联调验证，不要只依据发送返回值判断。

---

## 8. 推荐调用流程

### 8.1 手动模式下由客户完全控制状态灯

```cpp
sdk.SetLedAutoMode(false);
// 等待 OnSetLedAutoMode(ack.auto_mode == false)

LedCommand cmd{LedId::ALL, LedEffect::BLINK, {255, 128, 0, 255}, 300};
sdk.SetLedCommand(cmd);
```

**适用场景：**
- 展厅演示
- 售前演示
- 调试工具
- 用户自定义灯语

### 8.2 自动模式下由狗内部平台软件统一管理状态灯

```cpp
sdk.SetLedAutoMode(true);
// 等待 OnSetLedAutoMode(ack.auto_mode == true)
```

自动模式下，如果客户仍需要触发某个临时灯效事件，可调用：

```cpp
LedCommand cmd{LedId::ALL, LedEffect::BLINK_TRANSIENT, {0, 0, 255, 255}, 500};
sdk.SetLedCommand(cmd);
```

但此时应理解为：
- 这是一次“事件请求”；
- 是否显示、显示时长、是否被中断，由狗内部平台软件优先级策略决定。

---

## 9. 客户联调建议

### 9.1 如果客户要“完全自定义灯效”

建议：
1. 先切到手动模式；
2. 再连续下发 `SetLedCommand()`；
3. 业务侧自行维护当前灯态。

### 9.2 如果客户要“状态灯跟随设备状态”

建议：
1. 保持自动模式；
2. 将故障、告警、低电量、任务状态等逻辑放在狗内部平台软件侧统一管理；
3. 用户事件灯效按照优先级插入自动模式策略。



