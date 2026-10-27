# Robot SDK — Recharge and Undock Task Error Codes

## Overview

The robot reports recharge and undock task error codes through
`IDataCallback::OnTaskStateData(const TaskStateInfo& info)`. Read the code from
`info.error_code`. When `info.task_status == TaskStatus::FAILURE`, use
`info.task_type` to identify the corresponding task:

The accompanying `info.phase` can help identify the subphase reached before
failure, but it may be empty or contain a future value. Diagnose failure from
`task_status == FAILURE` and `error_code`.

| Task | `TaskType` | Error Code Range |
|:--|:--|:--|
| Recharge | `TaskType::RECHARGING` | `0x11000`–`0x11FFF` |
| Undock | `TaskType::UNDOCK` | `0x16000`–`0x16FFF` |

> These are task-level error codes reported in `TaskStateInfo.error_code`. They are
> distinct from the `std::error_code` returned by an SDK API. See
> [SDK Error Codes](sdk_error_en.md) for command-send errors.

## Usage Example

```cpp
void OnTaskStateData(const TaskStateInfo& info) override {
  if (info.task_status != TaskStatus::FAILURE) {
    return;
  }

  if (info.task_type == TaskType::RECHARGING) {
    // Recharge task error code: 0x11000-0x11FFF
  } else if (info.task_type == TaskType::UNDOCK) {
    // Undock task error code: 0x16000-0x16FFF
  }

  std::cout << "task error: 0x" << std::hex << info.error_code << std::dec
            << std::endl;
}
```

## Recharge Task Error Codes

| Error Code | C++ Enumerator | Description | Decimal |
|:--|:--|:--|--:|
| `0x11000` | `ARC_UNKNOWN` | Unknown error | 69632 |
| `0x11001` | `ARC_CALIBRATION_FILE_INVALID` | Invalid camera calibration file | 69633 |
| `0x11002` | `ARC_LOCALIZATION_CHECK_FAILED` | Localization module check failed | 69634 |
| `0x11003` | `ARC_NAV_ALIGNMENT_CHECK_FAILED` | Alignment module check failed | 69635 |
| `0x11004` | `ARC_DOCK_START_TASK_FAILED` | Reserved; not currently reported by the recharge workflow | 69636 |
| `0x11005` | `ARC_STAND_UP_FAILED` | Robot failed to stand up | 69637 |
| `0x11006` | `ARC_LOCALIZATION_START_FAILED` | Localization failed to start | 69638 |
| `0x11007` | `ARC_NAV_ALIGNMENT_START_FAILED` | Alignment task failed to start | 69639 |
| `0x11008` | `ARC_NAV_ALIGNMENT_FAILED` | Docking alignment failed | 69640 |
| `0x11009` | `ARC_DOCK_CONTACT_FAILED` | Dock contact confirmation failed | 69641 |
| `0x1100A` | `ARC_DOCK_CHARGING_START_FAILED` | Dock failed to start charging | 69642 |
| `0x1100B` | `ARC_CHARGING_STATE_LOST` | Charging state lost | 69643 |
| `0x1100C` | `ARC_CHARGING_DOCK_FAULT` | Dock fault during charging | 69644 |
| `0x1100D` | `ARC_CHARGING_STATE_INVALID` | Invalid charging state | 69645 |
| `0x1100E` | `ARC_CHARGING_BATTERY_STATE_LOST` | Battery state lost | 69646 |
| `0x1100F` | `ARC_DOCK_COMMAND_ACK_FAILED` | Dock command acknowledgment failed | 69647 |
| `0x11010` | `ARC_LOCALIZATION_RUNTIME_IMAGE_INVALID` | Invalid localization runtime image | 69648 |
| `0x11011` | `ARC_LOCALIZATION_RUNTIME_TARGET_TAG_TIMEOUT` | Timed out waiting for the target AprilTag | 69649 |
| `0x11012` | `ARC_LOCALIZATION_RUNTIME_TAG_POSE_FAILED` | AprilTag pose calculation failed at runtime | 69650 |
| `0x11013` | `ARC_LOCALIZATION_RUNTIME_DOCK_POSE_FAILED` | Dock pose calculation failed at runtime | 69651 |
| `0x11014` | `ARC_LOCALIZATION_RUNTIME_UNKNOWN` | Unknown localization runtime failure | 69652 |
| `0x11015` | `ARC_LOCALIZATION_RUNTIME_DUPLICATE_TARGET_TAG` | Duplicate target AprilTag detected at runtime | 69653 |
| `0x11016` | `ARC_NAV_ALIGNMENT_STOP_FAILED` | Failed to stop alignment during a CONTACT retry | 69654 |

## Undock Task Error Codes

| Error Code | C++ Enumerator | Description | Decimal |
|:--|:--|:--|--:|
| `0x16000` | `UNDOCK_UNKNOWN` | Unknown error | 90112 |
| `0x16001` | `UNDOCK_STAND_UP_FAILED` | Failed to stand up for undocking | 90113 |
| `0x16002` | `UNDOCK_STOP_CHARGING_FAILED` | Failed to stop charging | 90114 |
| `0x16003` | `UNDOCK_LOCALIZATION_START_FAILED` | Undock localization failed to start | 90115 |
| `0x16004` | `UNDOCK_NAV_ALIGNMENT_START_FAILED` | Undock alignment task failed to start | 90116 |
| `0x16005` | `UNDOCK_NAV_ALIGNMENT_FAILED` | Undock alignment failed | 90117 |
| `0x16006` | `UNDOCK_CALIBRATION_FILE_INVALID` | Invalid camera calibration file | 90118 |
| `0x16007` | `UNDOCK_FILL_LIGHT_FAILED` | Fill-light operation failed | 90119 |
| `0x16008` | `UNDOCK_LOCALIZATION_QR_NOT_DETECT` | QR code was not detected | 90120 |
| `0x16009` | `UNDOCK_LOCALIZATION_CHECK_FAILED` | Localization communication check failed | 90121 |
| `0x1600A` | `UNDOCK_NAV_ALIGNMENT_CHECK_FAILED` | Alignment module communication check failed | 90122 |
| `0x1600B` | `UNDOCK_LOCALIZATION_RUNTIME_IMAGE_INVALID` | Invalid undock localization runtime image | 90123 |
| `0x1600C` | `UNDOCK_LOCALIZATION_RUNTIME_TAG_POSE_FAILED` | AprilTag pose calculation failed during undock | 90124 |
| `0x1600D` | `UNDOCK_LOCALIZATION_RUNTIME_DOCK_POSE_FAILED` | Dock pose calculation failed during undock | 90125 |
| `0x1600E` | `UNDOCK_LOCALIZATION_RUNTIME_UNKNOWN` | Unknown undock localization runtime failure | 90126 |
| `0x1600F` | `UNDOCK_LOCALIZATION_RUNTIME_DUPLICATE_TARGET_TAG` | Duplicate target AprilTag detected during undock | 90127 |
| `0x16010` | `UNDOCK_DETECT_OBSTACLE_FAILED` | Undock failed because an obstacle was detected | 90128 |

## Related Documents

- [Recharge and Undock Task Guide](sdk_recharge_task_en.md) — Task APIs, state flow, and callback handling
- [Callback Reference](sdk_callback_en.md) — `OnTaskStateData()` callback
- [Type Definitions](sdk_type_en.md) — `TaskType`, `TaskStatus`, and `TaskStateInfo`
- [Recharge, Undock, and Dock Calibration Conditions and Troubleshooting](sdk_recharge_undock_calibration_conditions_en.md) — Prerequisites and troubleshooting
