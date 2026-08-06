# LED Control Interface Guide

## 1. Purpose

This document is intended for customers and explains the **LED/light control** capabilities in the Robot SDK, including control modes, callbacks, and recommended usage patterns, so customers can quickly complete status LED integration.

> Note: The SDK currently contains two different types of lighting capabilities:
> 1. **Fill lights**: front fill light, rear fill light, and automatic fill light mode;
> 2. **Status LEDs**: LED auto/manual mode and LED effect control.
>
> If the customer requirement is for a “status light / warning light / task indicator light”, it is recommended to focus on the **status LED interfaces** described in this document. The fill light interfaces are primarily intended for illumination.

---

## 2. Capability Overview

### 2.1 Core Status LED Interfaces

The SDK currently provides 3 core interfaces for status LED control:

1. `SetLedAutoMode(bool auto_mode)`: set the LED to auto mode or manual mode;
2. `GetLedAutoMode()`: query whether the LED is currently in auto mode;
3. `SetLedCommand(const LedCommand& cmd)`: set the LED group, effect, color, and period.

### 2.2 Related Callbacks

The corresponding control callbacks are:

1. `OnSetLedAutoMode(const LedAutoModeAck& ack)`
2. `OnGetLedAutoMode(const LedAutoModeAck& ack)`
3. `OnSetLedCommand(const LedCommandAck& ack)`

These callbacks indicate that the **robot/platform has received the control command and returned an acknowledgment**.

---

## 3. Difference Between the Two Lighting Interface Groups

### 3.1 Fill Light Interfaces (illumination)

The fill light interfaces include:

- `FrontLight(bool on)`: front fill light on/off
- `BackLight(bool on)`: rear fill light on/off
- `AutoModeLight(bool on)`: automatic fill light mode on/off

Their states are also reflected in robot state data:

- `RobotState::front_fill_light`
- `RobotState::back_fill_light`
- `RobotState::auto_mode_light`

> Notes:
> - According to the current documentation, calling `FrontLight()` / `BackLight()` will disable automatic fill light mode;
> - These interfaces are mainly intended for illumination and are not recommended as a replacement for status LED control.

### 3.2 Status LED Interfaces (status / alarm / task effects)

The status LED interfaces include:

- `SetLedAutoMode()`
- `GetLedAutoMode()`
- `SetLedCommand()`

These interfaces are more suitable for status LED scenarios such as:

- fault / emergency stop indication
- alarm indication
- low battery indication
- task execution status
- normal operating status
- user-defined event indication

---

## 4. Control Modes

### 4.1 Manual Mode

**Definition:**
The user fully takes over status LED control. The status LEDs are no longer managed by the robot's internal platform software, and the user controls the effects directly through the control interfaces.

**Recommended control flow:**
1. Call `SetLedAutoMode(false)` to switch to manual mode;
2. Wait for `OnSetLedAutoMode()` to confirm the current mode;
3. Call `SetLedCommand()` to send the desired effect.

**Characteristics:**
- User control has priority;
- The robot's internal platform software will no longer switch the status LEDs automatically based on device state;
- Suitable for debugging, demos, custom light language, and special scenarios where user logic takes over.

### 4.2 Auto Mode

**Definition:**
The status LEDs are managed uniformly by the robot's internal platform software according to the current device state or event.

In auto mode, LED effects sent by the user through `SetLedCommand()` should be understood as **event triggers**, not absolute overrides. The final displayed effect is arbitrated by the robot's internal platform software strategy.

**Recommended priority order (based on the current requirement definition):**

| Priority | Scenario |
|:--|:--|
| 1 | System fault, emergency stop |
| 2 | Alarm |
| 3 | Low battery |
| 4 | User-controllable events |
| 5 | Task execution status light |
| 6 | Normal operating light state |

**Recommended interpretation in auto mode:**
- A higher-priority event should override a lower-priority effect;
- User-issued LED effects belong to the “user-controllable event” layer in auto mode;
- Whether a user effect takes effect immediately, how long it lasts, and whether it is interrupted by a higher-priority event should be determined by the robot's internal platform software strategy.
- If the user needs to turn off *user-controllable events*, simply call `SetLedCommand(LedId::ALL, LedEffect::OFF)`.

> Important:
> The above priority definition should be understood as a **platform/device-side business rule**, not SDK-local logic.

---

## 5. Interface Details

### 5.1 SetLedAutoMode

```cpp
std::error_code SetLedAutoMode(
    bool auto_mode, int timeout_ms = 0,
    WriteHandler handler = [](const std::error_code&, std::size_t) {});
```

**Purpose:**
Set the status LED to auto mode or manual mode.

**Parameters:**

| Parameter | Type | Description |
|:--|:--|:--|
| `auto_mode` | `bool` | `true`: auto mode; `false`: manual mode |
| `timeout_ms` | `int` | `0` means asynchronous sending; greater than `0` means synchronous wait for send completion |
| `handler` | `WriteHandler` | Send-result callback used in asynchronous mode |

**Return:**
- The return value indicates whether the SDK command sending flow completed normally;
- The device acknowledgment result is obtained through `OnSetLedAutoMode()`.

### 5.2 GetLedAutoMode

```cpp
std::error_code GetLedAutoMode(
    int timeout_ms = 0,
    WriteHandler handler = [](const std::error_code&, std::size_t) {});
```

**Purpose:**
Query whether the status LED is currently in auto mode.

**How to obtain the result:**
Use `OnGetLedAutoMode(const LedAutoModeAck& ack)` to get the query result.

### 5.3 SetLedCommand

```cpp
std::error_code SetLedCommand(
    const LedCommand& cmd, int timeout_ms = 0,
    WriteHandler handler = [](const std::error_code&, std::size_t) {});
```

**Purpose:**
Set the LED group, effect, color, and period.

**Parameter structure:**

```cpp
struct LedCommand {
  LedId id;
  LedEffect effect;
  LedColor color;
  uint32_t duration_ms;
};
```

**Return:**
- If `cmd.id == LedId::UNKNOWN` or `cmd.effect == LedEffect::UNKNOWN`, the SDK directly returns `std::errc::invalid_argument`;
- In other cases, the return value indicates the sending result;
- The device acknowledgment is returned through `OnSetLedCommand(const LedCommandAck& ack)`.

---

## 6. Data Types

### 6.1 LedId

```cpp
enum class LedId {
  UNKNOWN = 0,
  ALL,
  FRONT,
  BACK,
};
```

| Enum Value | Meaning |
|:--|:--|
| `ALL` | All LEDs |
| `FRONT` | Front LED group |
| `BACK` | Rear LED group |

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

| Enum Value | Protocol Value | Meaning |
|:--|:--|:--|
| `OFF` | `off` | Off |
| `ON` | `on` | Solid on |
| `BREATH` | `breath` | Breathing |
| `BLINK` | `blink` | Blinking |
| `BLINK_TRANSIENT` | `blink_transient` | Transient blink |

### 6.3 LedColor

```cpp
struct LedColor {
  uint8_t r;
  uint8_t g;
  uint8_t b;
  uint8_t a;
};
```

Each color channel ranges from `0` to `255`.

### 6.4 duration_ms

`duration_ms` represents the effect period or blink interval in milliseconds.

Recommended convention:
- `OFF` / `ON`: typically set to `0`;
- `BREATH` / `BLINK` / `BLINK_TRANSIENT`: use an actual period value such as `300 ms` or `1000 ms`.

---

## 7. Callback and Sync/Async Usage Notes

### 7.1 Difference Between Send Callback and Control Callback

There are two kinds of results in the SDK:

1. **Send result**: returned through `handler`, indicating whether the command was sent successfully;
2. **Device control acknowledgment**: returned through `IControlCallback`, indicating that the device has acknowledged the command.

For LED-related interfaces, customers should pay more attention to:

- `OnSetLedAutoMode()`
- `OnGetLedAutoMode()`
- `OnSetLedCommand()`

### 7.2 Actual Meaning of timeout_ms

In the current SDK implementation, the “synchronous mode” when `timeout_ms > 0` mainly means **waiting for command sending to complete or time out**, and does not mean that the light effect has already completed execution.

Therefore, it is recommended that:
- if the business logic needs to confirm mode switching or query results, use the corresponding control callback as the authoritative result;
- if the business logic needs to confirm the actual displayed effect, verify it during device-side integration instead of relying only on the send return value.

---

## 8. Recommended Call Flows

### 8.1 Manual Mode with Full Customer Control of the Status LEDs

```cpp
sdk.SetLedAutoMode(false);
// Wait for OnSetLedAutoMode(ack.auto_mode == false)

LedCommand cmd{LedId::ALL, LedEffect::BLINK, {255, 128, 0, 255}, 300};
sdk.SetLedCommand(cmd);
```

**Applicable scenarios:**
- showroom demos
- presales demos
- debugging tools
- user-defined light language

### 8.2 Auto Mode with the Status LEDs Managed by the Robot's Internal Platform Software

```cpp
sdk.SetLedAutoMode(true);
// Wait for OnSetLedAutoMode(ack.auto_mode == true)
```

In auto mode, if the customer still needs to trigger a temporary LED effect event, the following can be used:

```cpp
LedCommand cmd{LedId::ALL, LedEffect::BLINK_TRANSIENT, {0, 0, 255, 255}, 500};
sdk.SetLedCommand(cmd);
```

In this case, it should be understood as:
- this is an “event request”;
- whether it is displayed, how long it is displayed, and whether it is interrupted are determined by the robot's internal platform software priority strategy.

---

## 9. Customer Integration Recommendations

### 9.1 If the Customer Wants Full Custom Light Effects

Recommended approach:
1. Switch to manual mode first;
2. Then continuously send `SetLedCommand()`;
3. Maintain the current LED state in the business logic.

### 9.2 If the Customer Wants the Status LEDs to Follow Device State

Recommended approach:
1. Keep auto mode enabled;
2. Manage fault, alarm, low battery, and task-status logic uniformly on the robot's internal platform software side;
3. Insert user event effects into the auto-mode strategy based on priority.

