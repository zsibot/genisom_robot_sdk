#pragma once

#include "robot_sdk/sdk_export.hpp"
#include "robot_sdk/sdk_type.hpp"
namespace robot_sdk {

/// @brief User implementation is required as needed.
/// Callback functions must be lightweight and cannot perform time-consuming
/// operations.
/// --Within the callback function, only quick operations such as data copying
/// and data validation should be performed.
/// --For time-consuming operations such as database writes, file I/O, complex
/// calculations, and network transmission, users need to copy the data to
/// another thread (e.g., user's own thread pool) within the callback function
/// for processing.
class ROBOT_EXPORT_API IDataCallback {
 public:
  /// @brief IMU data callback (reported at fixed frequency after
  /// configuration).
  /// @param data IMU data.
  virtual void OnImuData(const ImuData& data) {}

  /// @brief Lux (illuminance) data callback (reported at fixed 1Hz frequency
  /// after configuration).
  /// @param data Lux data.
  virtual void OnLuxData(const LuxData& data) {}

  /// @brief Motion control data callback (reported at fixed 50Hz frequency
  /// after configuration).
  /// @param data Motion data.
  virtual void OnMcData(const MotionData& data) {}

  /// @brief Speed data callback (reported at fixed frequency after
  /// configuration).
  /// @param data
  virtual void OnSpeedData(const SpeedData& data) {}

  /// @brief  Joint state data callback (reported at fixed frequency after
  /// configuration).
  /// @param data
  virtual void OnJointStateData(const JointStateData& data) {}

  /// @brief Robot state data callback (actively reported at 1Hz frequency).
  /// @param data Robot state data.
  virtual void OnRobotStateData(const RobotState& data) {}

  /// @brief Fault information data callback (actively reported when a fault
  /// occurs).
  /// @param data Fault data.
  virtual void OnFaultData(const FaultDatas& data) {}

  /// @brief Control ownership lost callback.
  virtual void OnControlLost(const ControlLostInfo& info) {}

  /// @brief Control ownership available callback.
  virtual void OnControlAvailable(const ControlAvailableInfo& info) {}

  /// @brief Task state data callback (actively reported when task state
  /// changes).
  virtual void OnTaskStateData(const TaskStateInfo& info) {}

  virtual ~IDataCallback() = default;
};

/// @brief User implementation is required as needed.
///        Used in non-blocking mode: indicates that the Robot has received the
///        control command.
class ROBOT_EXPORT_API IControlCallback {
 public:
  /// @brief Acknowledgment for soft emergency stop command received.
  /// @param on true: emergency stop was activated; false: emergency stop was
  /// deactivated.
  virtual void OnSoftEmergencyStop(bool on) {}

  /// @brief Acknowledgment for stand up command received.
  virtual void OnStandUp() {}

  /// @brief Acknowledgment for balance stand up command received.
  virtual void OnBalanceStandUp() {}

  /// @brief Acknowledgment for lie down command received.
  virtual void OnLieDown() {}

  /// @brief Acknowledgment for stair command received.
  virtual void OnStair() {}

  /// @brief Acknowledgment for crawl command received.
  virtual void OnCrawl() {}

  /// @brief Acknowledgment for crawl walk command received.
  virtual void OnCrawlWalk() {}

  /// @brief Acknowledgment for climb command received.
  virtual void OnClimb() {}

  /// @brief Acknowledgment for slim (body compress) command received.
  virtual void OnSlim() {}

  /// @brief Acknowledgment for gait command received.
  virtual void OnGait() {}

  /// @brief Acknowledgment for DSB command received.
  virtual void OnDSB() {}

  /// @brief Acknowledgment for PosControl command received.
  virtual void OnPosControl() {}

  /// @brief Acknowledgment for SameKnee Walk command received.
  virtual void OnSkWalk() {}

  /// @brief Acknowledgment for sand posture command received.
  virtual void OnSand() {}

  /// @brief Acknowledgment for reverse head-tail command received.
  virtual void OnReverseHeadTail() {}

  /// @brief Acknowledgment for speed level switch command received.
  /// @param speed_level The speed level set by the user.
  virtual void OnSpeed(int speed_level) {}

  /// @brief Acknowledgment for lock command received.
  virtual void OnLocked() {}

  /// @brief Acknowledgment for front fill light command received.
  /// @param on true: enabled; false: disabled.
  virtual void OnFrontLight(bool on) {}

  /// @brief Acknowledgment for back fill light command received.
  /// @param on true: enabled; false: disabled.
  virtual void OnBackLight(bool on) {}

  /// @brief Acknowledgment for auto mode light command received.
  /// @param on true: enabled; false: disabled.
  virtual void OnAutoModeLight(bool on) {}

  /// @brief Acknowledgment for obstacle avoidance command received.
  /// @param on true: enabled; false: disabled.
  virtual void OnObstacleAvoidance(bool on) {}

  /// @brief Acknowledgment for illuminance configuration command received.
  /// @param on true: enabled; false: disabled.
  virtual void OnLuxConfig(bool on) {}

  /// @brief Acknowledgment for IMU configuration command received.
  /// @param freq The frequency set by the user.
  virtual void OnImuConfig(int freq) {}

  /// @brief Acknowledgment for motion control configuration command received.
  /// @param on true: enabled; false: disabled.
  virtual void OnMcConfig(bool on) {}

  /// @brief Acknowledgment for speed report configuration command received.
  /// @param on true: enabled; false: disabled.
  /// @param frequency The frequency set by the user.
  virtual void OnSpeedReportConfig(bool on, uint32_t frequency) {}

  /// @brief Acknowledgment for joint state configuration command received.
  /// @param on true: enabled; false: disabled.
  virtual void OnJointStateConfig(bool on) {}

  /// @brief Acknowledgment for take control command received.
  /// @param ack Control acknowledgment information.
  virtual void OnTakeControlAck(const TakeControlAck& ack) {}

  /// @brief Acknowledgment for release control command received.
  /// @param ack Control acknowledgment information.
  virtual void OnReleaseControlAck(const ReleaseControlAck& ack) {}

  /// @brief Acknowledgment for camera bitrate update command received.
  /// @param ack Camera bitrate acknowledgment information.
  virtual void OnUpdateCameraBitrateAck(const CameraBitrateAck& ack) {}

  /// @brief Acknowledgment for take photo command received.
  /// @param ack Take photo acknowledgment information.
  virtual void OnTakePhotoAck(const TakePhotoAck& ack) {}

  /// @brief Notification that the robot has received the switching instruction.
  virtual void OnSwitchRemote() {}

  /// @brief Notification that the robot has received the switching instruction.
  virtual void OnSwitchIdle() {}

  /// @brief Notification that the robot has entered recharge mode.
  virtual void OnStartRechargeTask() {}

  /// @brief Notification that the robot has exited recharge mode.
  virtual void OnStopRechargeTask() {}

  /// @brief Notification that the robot has entered undock mode.
  virtual void OnStartUnDockTask() {}

  /// @brief Notification that the robot has exited undock mode.
  virtual void OnStopUnDockTask() {}

  /// @brief Acknowledgment for peripheral power control command received.
  /// @param ack Peripheral power control acknowledgment information.
  virtual void OnSetPeriphPower(const PowerCtrlAck& ack) {}

  /// @brief Acknowledgment for peripheral power control command received.
  /// @param ack Peripheral power control acknowledgment information.
  virtual void OnGetPeriphPower(const PowerCtrlAck& ack) {}

  /// @brief Acknowledgment for LED auto/manual mode setting received.
  /// @param ack LED auto mode acknowledgment information.
  virtual void OnSetLedAutoMode(const LedAutoModeAck& ack) {}

  /// @brief Acknowledgment for LED auto/manual mode query received.
  /// @param ack LED auto mode acknowledgment information.
  virtual void OnGetLedAutoMode(const LedAutoModeAck& ack) {}

  /// @brief Acknowledgment for LED command received.
  /// @param ack LED command acknowledgment information.
  virtual void OnSetLedCommand(const LedCommandAck& ack) {}

  virtual ~IControlCallback() = default;
};
}  // namespace robot_sdk
