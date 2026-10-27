# Robot SDK — 回充与出桩任务错误码

## 概述

回充与出桩任务的业务错误码由机器人通过
`IDataCallback::OnTaskStateData(const TaskStateInfo& info)` 主动上报，读取字段为
`info.error_code`。建议在 `info.task_status == TaskStatus::FAILURE` 时结合
`info.task_type` 判断错误码所属任务：

同一回调中的 `info.phase` 可用于了解失败前所在的细分阶段，但它可能为空或为
后续新增值；故障判定仍应以 `task_status == FAILURE` 和 `error_code` 为准。

| 任务 | `TaskType` | 错误码范围 |
|:--|:--|:--|
| 回充任务 | `TaskType::RECHARGING` | `0x11000`～`0x11FFF` |
| 出桩任务 | `TaskType::UNDOCK` | `0x16000`～`0x16FFF` |

> 本文中的错误码是 `TaskStateInfo.error_code` 上报的任务业务错误码，不是
> SDK 接口返回的 `std::error_code`。接口发送结果的错误码请参阅
> [SDK 错误码文档](sdk_error_zh.md)。

## 使用示例

```cpp
void OnTaskStateData(const TaskStateInfo& info) override {
  if (info.task_status != TaskStatus::FAILURE) {
    return;
  }

  if (info.task_type == TaskType::RECHARGING) {
    // 回充任务错误码：0x11000～0x11FFF
  } else if (info.task_type == TaskType::UNDOCK) {
    // 出桩任务错误码：0x16000～0x16FFF
  }

  std::cout << "task error: 0x" << std::hex << info.error_code << std::dec
            << std::endl;
}
```

## 回充任务错误码

| 错误码 | C++ 枚举 | 中文说明 | 十进制 |
|:--|:--|:--|--:|
| `0x11000` | `ARC_UNKNOWN` | 未知异常 | 69632 |
| `0x11001` | `ARC_CALIBRATION_FILE_INVALID` | 相机标定文件异常 | 69633 |
| `0x11002` | `ARC_LOCALIZATION_CHECK_FAILED` | 感知模块检查失败 | 69634 |
| `0x11003` | `ARC_NAV_ALIGNMENT_CHECK_FAILED` | 对准模块检查失败 | 69635 |
| `0x11004` | `ARC_DOCK_START_TASK_FAILED` | 已保留，当前回充流程不发布 | 69636 |
| `0x11005` | `ARC_STAND_UP_FAILED` | 机器人站立失败 | 69637 |
| `0x11006` | `ARC_LOCALIZATION_START_FAILED` | 感知启动失败 | 69638 |
| `0x11007` | `ARC_NAV_ALIGNMENT_START_FAILED` | 对准任务启动失败 | 69639 |
| `0x11008` | `ARC_NAV_ALIGNMENT_FAILED` | 进桩对准失败 | 69640 |
| `0x11009` | `ARC_DOCK_CONTACT_FAILED` | 基站接触确认失败 | 69641 |
| `0x1100A` | `ARC_DOCK_CHARGING_START_FAILED` | 基站开始充电失败 | 69642 |
| `0x1100B` | `ARC_CHARGING_STATE_LOST` | 充电状态丢失 | 69643 |
| `0x1100C` | `ARC_CHARGING_DOCK_FAULT` | 充电过程中基站故障 | 69644 |
| `0x1100D` | `ARC_CHARGING_STATE_INVALID` | 充电状态异常 | 69645 |
| `0x1100E` | `ARC_CHARGING_BATTERY_STATE_LOST` | 电池状态丢失 | 69646 |
| `0x1100F` | `ARC_DOCK_COMMAND_ACK_FAILED` | 基站命令响应失败 | 69647 |
| `0x11010` | `ARC_LOCALIZATION_RUNTIME_IMAGE_INVALID` | 感知运行时图像异常 | 69648 |
| `0x11011` | `ARC_LOCALIZATION_RUNTIME_TARGET_TAG_TIMEOUT` | 感知运行时未获得目标 AprilTag，等待超时 | 69649 |
| `0x11012` | `ARC_LOCALIZATION_RUNTIME_TAG_POSE_FAILED` | 感知运行时 AprilTag 位姿计算失败 | 69650 |
| `0x11013` | `ARC_LOCALIZATION_RUNTIME_DOCK_POSE_FAILED` | 感知运行时基站位姿计算失败 | 69651 |
| `0x11014` | `ARC_LOCALIZATION_RUNTIME_UNKNOWN` | 感知运行时未知失败 | 69652 |
| `0x11015` | `ARC_LOCALIZATION_RUNTIME_DUPLICATE_TARGET_TAG` | 感知运行时目标 AprilTag 重复检测 | 69653 |
| `0x11016` | `ARC_NAV_ALIGNMENT_STOP_FAILED` | CONTACT 重试时停止对准失败 | 69654 |

## 出桩任务错误码

| 错误码 | C++ 枚举 | 中文说明 | 十进制 |
|:--|:--|:--|--:|
| `0x16000` | `UNDOCK_UNKNOWN` | 未知异常 | 90112 |
| `0x16001` | `UNDOCK_STAND_UP_FAILED` | 退桩站立失败 | 90113 |
| `0x16002` | `UNDOCK_STOP_CHARGING_FAILED` | 停止充电失败 | 90114 |
| `0x16003` | `UNDOCK_LOCALIZATION_START_FAILED` | 退桩感知启动失败 | 90115 |
| `0x16004` | `UNDOCK_NAV_ALIGNMENT_START_FAILED` | 退桩对准任务启动失败 | 90116 |
| `0x16005` | `UNDOCK_NAV_ALIGNMENT_FAILED` | 退桩对准失败 | 90117 |
| `0x16006` | `UNDOCK_CALIBRATION_FILE_INVALID` | 相机标定文件异常 | 90118 |
| `0x16007` | `UNDOCK_FILL_LIGHT_FAILED` | 补光灯操作失败 | 90119 |
| `0x16008` | `UNDOCK_LOCALIZATION_QR_NOT_DETECT` | 没有扫描到二维码 | 90120 |
| `0x16009` | `UNDOCK_LOCALIZATION_CHECK_FAILED` | 感知通信检查失败 | 90121 |
| `0x1600A` | `UNDOCK_NAV_ALIGNMENT_CHECK_FAILED` | 对准模块通信检查失败 | 90122 |
| `0x1600B` | `UNDOCK_LOCALIZATION_RUNTIME_IMAGE_INVALID` | 退桩感知运行时图像异常 | 90123 |
| `0x1600C` | `UNDOCK_LOCALIZATION_RUNTIME_TAG_POSE_FAILED` | 退桩感知运行时 AprilTag 位姿计算失败 | 90124 |
| `0x1600D` | `UNDOCK_LOCALIZATION_RUNTIME_DOCK_POSE_FAILED` | 退桩感知运行时基站位姿计算失败 | 90125 |
| `0x1600E` | `UNDOCK_LOCALIZATION_RUNTIME_UNKNOWN` | 退桩感知运行时未知失败 | 90126 |
| `0x1600F` | `UNDOCK_LOCALIZATION_RUNTIME_DUPLICATE_TARGET_TAG` | 退桩感知运行时目标 AprilTag 重复检测 | 90127 |
| `0x16010` | `UNDOCK_DETECT_OBSTACLE_FAILED` | 退桩检测到障碍物导致失败 | 90128 |

## 相关文档

- [回充与出桩任务使用说明](sdk_recharge_task_zh.md) — 任务接口、状态流转与回调处理
- [Callback 回调接口](sdk_callback_zh.md) — `OnTaskStateData()` 回调说明
- [类型定义文档](sdk_type_zh.md) — `TaskType`、`TaskStatus` 与 `TaskStateInfo`
- [回充、退桩与基站标定使用条件和故障说明](sdk_recharge_undock_calibration_conditions_zh.md) — 使用条件与故障排查
