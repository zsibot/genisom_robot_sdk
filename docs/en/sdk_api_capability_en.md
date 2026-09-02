# Robot SDK — API Capability Matrix

## Scope

This document lists every public `SDKClient` API and its availability for each
known `DeviceType` value.

Capability symbols:

| Symbol | Meaning |
|:--:|:--|
| ✓ | The current SDK does not reject the API for this device type |
| — | Known to be unsupported; the SDK or gateway rejects it |

> “✓” indicates that the device-type capability allows the API. Calls can still
> fail because of connection state, control ownership, robot state, parameters,
> or unavailable peripherals.
>
> When the SDK rejects an API by device type, it returns
> `Errc::UnsupportedDeviceOperation`. See [SDK Error Code Documentation](sdk_error_en.md).

---

## Device Column Legend

| Abbreviation | `DeviceType` | Device |
|:--|:--|:--|
| M1 | `M1` | Medium dog, wheeled-foot |
| M1F | `M1F` | Medium dog, point-foot |
| M1P | `M1_PRO` | Medium dog, wheeled-foot laser version |
| M1FP | `M1F_PRO` | Medium dog, point-foot laser version |
| M1U | `M1_ULTRA` | Medium dog, wheeled-foot panoramic version |
| M1FU | `M1F_ULTRA` | Medium dog, point-foot panoramic version |
| M1A | `M1_AIR` | Medium dog, wheeled-foot Air version |
| M1FA | `M1F_AIR` | Medium dog, point-foot Air version |
| L2 | `L2` | Small dog, wheeled sport version |
| L2U | `L2_ULTRA` | Small dog, wheeled panoramic version |
| L2F | `L2F` | Small dog, point-foot sport version |
| L2FU | `L2F_ULTRA` | Small dog, point-foot panoramic version |

---

## Client, Connection, and Information APIs

| API | Brief Description | M1 | M1F | M1P | M1FP | M1U | M1FU | M1A | M1FA | L2 | L2U | L2F | L2FU |
|:--|:--|:--:|:--:|:--:|:--:|:--:|:--:|:--:|:--:|:--:|:--:|:--:|:--:|
| `SDKClient()` | Creates a client and configures transport and reconnection | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `~SDKClient()` | Releases the client, connection, and internal threads | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `Connect()` | Connects to the robot and completes the protocol handshake | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `Disconnect()` | Actively disconnects from the robot | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `IsConnected()` | Checks whether the client is connected | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `GetConnectionState()` | Returns the complete connection state | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `SetControlCallback()` | Registers the incoming control-command callback | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `SetDataCallback()` | Registers state and sensor-data callbacks | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `Version()` | Returns the SDK version | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `ProtocolVersion()` | Returns the SDK protocol version | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `SystemVersion()` | Returns the robot system version from the handshake | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `GetDeviceInfo()` | Returns the cached device type and serial number | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |

---

## Posture and Basic Control APIs

| API | Brief Description | M1 | M1F | M1P | M1FP | M1U | M1FU | M1A | M1FA | L2 | L2U | L2F | L2FU |
|:--|:--|:--:|:--:|:--:|:--:|:--:|:--:|:--:|:--:|:--:|:--:|:--:|:--:|
| `SoftEmergencyStop()` | Enables or releases software emergency stop | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `StandUp()` | Switches to the standing posture | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `BalanceStandUp()` | Switches to balance-standing posture | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `LieDown()` | Switches to the lying-down posture | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `Stair()` | Switches to the stair posture | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `Crawl()` | Switches to the crawl posture | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `CrawlWalk()` | Switches to the crawling-walk posture | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `Climb()` | Switches to the high-platform climbing posture | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `Gait()` | Switches to the fine-gait posture | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | — | — |
| `Slim()` | Switches to the narrow-passage posture | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | — | — | — | — |
| `DSB()` | Switches to the door-sill/barrier posture | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | — | — | — | — |
| `PosControl()` | Switches to position-control posture | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `ReverseHeadTail()` | Reverses the robot's head and tail | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | — | — | — | — | — | — |
| `SetSpeed()` | Sets the movement speed level | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `Locked()` | Switches to the locked posture | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `SkWalk()` | Switches to the same-knee posture | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | — | — | — | — |
| `Sand()` | Switches to the sand posture | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | — | — | — | — |

---

## Lighting and Environment APIs

| API | Brief Description | M1 | M1F | M1P | M1FP | M1U | M1FU | M1A | M1FA | L2 | L2U | L2F | L2FU |
|:--|:--|:--:|:--:|:--:|:--:|:--:|:--:|:--:|:--:|:--:|:--:|:--:|:--:|
| `FrontLight()` | Turns the front fill light on or off | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `BackLight()` | Turns the rear fill light on or off | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | — | — | — | — | — | — |
| `AutoModeLight()` | Enables or disables automatic fill-light mode | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `ObstacleAvoidance()` | Enables or disables obstacle stopping | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | — | — | — | — |

---

## Motion Control APIs

| API | Brief Description | M1 | M1F | M1P | M1FP | M1U | M1FU | M1A | M1FA | L2 | L2U | L2F | L2FU |
|:--|:--|:--:|:--:|:--:|:--:|:--:|:--:|:--:|:--:|:--:|:--:|:--:|:--:|
| `PosMove()` | Sends a position target in position-control posture | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `Move()` | Sends translational and rotational velocity commands | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `Turn()` | Controls body roll in balance-standing mode | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `ControlHead()` | Controls lateral head movement and pitch | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `HighLowStance()` | Selects high, low, or restored stance | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |

---

## Data Reporting Configuration APIs

| API | Brief Description | M1 | M1F | M1P | M1FP | M1U | M1FU | M1A | M1FA | L2 | L2U | L2F | L2FU |
|:--|:--|:--:|:--:|:--:|:--:|:--:|:--:|:--:|:--:|:--:|:--:|:--:|:--:|
| `SetImuConfig()` | Configures IMU reporting and frequency | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `SetLuxConfig()` | Configures illuminance reporting | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `SetMcConfig()` | Configures motion-control state reporting | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `SetSpeedReportConfig()` | Configures body-speed reporting and frequency | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `SetJointStateConfig()` | Configures joint-state reporting | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |

---

## Control Ownership and Camera APIs

| API | Brief Description | M1 | M1F | M1P | M1FP | M1U | M1FU | M1A | M1FA | L2 | L2U | L2F | L2FU |
|:--|:--|:--:|:--:|:--:|:--:|:--:|:--:|:--:|:--:|:--:|:--:|:--:|:--:|
| `TakeControl()` | Requests robot control ownership | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `ReleaseControl()` | Releases robot control ownership | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `UpdateCameraBitrate()` | Updates the selected camera stream bitrate | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |

---

## Task and Machine-State APIs

| API | Brief Description | M1 | M1F | M1P | M1FP | M1U | M1FU | M1A | M1FA | L2 | L2U | L2F | L2FU |
|:--|:--|:--:|:--:|:--:|:--:|:--:|:--:|:--:|:--:|:--:|:--:|:--:|:--:|
| `StartRechargeTask()` | Starts an autonomous recharge task | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `StopRechargeTask()` | Stops an autonomous recharge task | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `StartUnDockTask()` | Starts an autonomous undocking task | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `StopUnDockTask()` | Stops an autonomous undocking task | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `SwitchIdleState()` | Requests the idle machine state | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `SwitchRemoteState()` | Requests the remote-control machine state | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |

Recharge and undocking also require an available charging dock and related
configuration. “✓” only means that the API is not disabled by `DeviceType`.

---

## Peripheral Power and LED APIs

| API | Brief Description | M1 | M1F | M1P | M1FP | M1U | M1FU | M1A | M1FA | L2 | L2U | L2F | L2FU |
|:--|:--|:--:|:--:|:--:|:--:|:--:|:--:|:--:|:--:|:--:|:--:|:--:|:--:|
| `SetPeriphPower()` | Sets M1 12V, 24V, or 48V peripheral power | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `GetPeriphPower()` | Queries the selected M1 peripheral power state | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `SetLedAutoMode()` | Selects automatic or manual LED control | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `GetLedAutoMode()` | Queries the current LED automatic-control mode | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| `SetLedCommand()` | Sets LED group, effect, color, and period | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |

Peripheral-power and LED execution also depends on installed hardware and robot
configuration.

---

## Maintenance Rules

1. Add a row whenever a public `SDKClient` API is introduced.
2. Add a device column or update the grouping whenever a `DeviceType` value is introduced.
3. Update this matrix, the API reference, and error examples whenever an internal device gate changes.
4. Known unsupported APIs should return `Errc::UnsupportedDeviceOperation` without sending a protocol command.
