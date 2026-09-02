/**
 * @file control.cpp
 * @brief Robot SDK Interactive Control Example
 *
 * This example demonstrates complete interactive robot control functionality:
 * - Real-time robot motion control via keyboard input
 * - Mode switching, speed adjustment, posture control, light control commands,
 * etc.
 * - Asynchronous connection management
 *
 * Applicable scenarios:
 * - Remote robot teleoperation
 * - Feature testing and debugging
 * - SDK API feature demonstration
 */

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <csignal>
#include <functional>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <thread>

#include "robot_sdk/sdk_client.hpp"

using namespace robot_sdk;

// Global control flags
static std::atomic<bool> g_running{true};
static std::atomic<bool> g_front_light_on{false};
static std::atomic<bool> g_back_light_on{false};
static std::atomic<bool> g_auto_mode_light_on{false};
static std::atomic<bool> g_led_auto_mode_on{true};
static std::atomic<bool> g_obstacle_avoidance_on{false};
static std::atomic<bool> g_estop_on{false};

// Synchronization primitives
static std::mutex g_connect_mtx;
static std::condition_variable g_connect_cv;

// Robot state cache
static std::mutex g_state_mtx;
static RobotState g_robot_state;
static std::atomic<bool> g_robot_m1_12v{true};

// State string mappings

const std::unordered_map<SpeedLevel, const char*> g_speed_level_map = {
    {SpeedLevel::SPEED_LEVEL_SLOW, "Slow"},
    {SpeedLevel::SPEED_LEVEL_MEDIUM, "Medium"},
    {SpeedLevel::SPEED_LEVEL_HIGH, "High"},
    {SpeedLevel::SPEED_LEVEL_UNKNOWN, "Unknown"}};

const std::unordered_map<MotionStatus, const char*> g_motion_status_map = {
    {MotionStatus::MOTION_STATUS_STAND_UP, "StandUp"},
    {MotionStatus::MOTION_STATUS_WALK, "Walk"},
    {MotionStatus::MOTION_STATUS_BALANCE_STAND, "BalanceStand"},
    {MotionStatus::MOTION_STATUS_LIE_DOWN, "LieDown"},
    {MotionStatus::MOTION_STATUS_CRAWL, "Crawl"},
    {MotionStatus::MOTION_STATUS_CRAWL_WALK, "CrawlWalk"},
    {MotionStatus::MOTION_STATUS_LOCKED, "Locked"},
    {MotionStatus::MOTION_STATUS_CLIMB, "Climb"},
    {MotionStatus::MOTION_STATUS_STAIR, "Stair"},
    {MotionStatus::MOTION_STATUS_SLIM, "Slim"},
    {MotionStatus::MOTION_STATUS_GAIT, "Gait"},
    {MotionStatus::MOTION_STATUS_DSB, "DSB"},
    {MotionStatus::MOTION_STATUS_POS_CONTROL, "PosControl"},
    {MotionStatus::MOTION_STATUS_SK_WALK, "SkWalk"},
    {MotionStatus::MOTION_STATUS_SAND, "Sand"},
    {MotionStatus::MOTION_STATUS_UNKNOWN, "Unknown"}};

const std::unordered_map<MachineStatus, const char*> g_machine_status_map = {
    {MachineStatus::IDLE, "Idle"},
    {MachineStatus::REMOTE, "Remote"},
    {MachineStatus::OTA, "OTA"},
    {MachineStatus::RECHARGE, "Recharge"},
    {MachineStatus::MAPPING, "Mapping"},
    {MachineStatus::NAVIGATION, "Navigation"},
    {MachineStatus::SAFETY, "Safety"},
    {MachineStatus::SELFTEST, "SelfTest"},
    {MachineStatus::SOFT_SHUTDOWN, "SoftShutdown"},
    {MachineStatus::SILENCE, "Silence"},
    {MachineStatus::FOLLOW, "Follow"},
    {MachineStatus::TRACK, "Track"},
    {MachineStatus::UNDOCK, "Undock"},
    {MachineStatus::DOCK_CALIBRATION, "DockCalibration"},
    {MachineStatus::ESTOP, "Estop"},
    {MachineStatus::FALL, "Fall"},
    {MachineStatus::LOCAL_REMOTE, "LocalRemote"},
    {MachineStatus::LOW_LEVEL, "LowLevel"},
    {MachineStatus::UNKNOWN, "Unknown"}};

// Signal handler function
void SignalHandler(int signal) {
  std::cout << "\n[INFO] Received signal " << signal << ", shutting down..."
            << std::endl;
  g_running = false;
}

/**
 * @brief Data callback class - receive robot sensor data
 *
 * @note Callback functions must be lightweight and cannot perform
 * time-consuming operations
 * - Only perform fast operations such as data copying and validation
 * - Time-consuming operations (database writes, file I/O, network transmission,
 * etc.) must be handled in separate threads
 */
class DataCallback : public IDataCallback {
 public:
  void OnRobotStateData(const RobotState& data) override {
    std::lock_guard<std::mutex> lock(g_state_mtx);
    g_robot_state = data;
  }

  void OnFaultData(const FaultDatas& data) override {
    if (data.empty()) return;
    std::cout << "\n[FAULT] Detected " << data.size()
              << " fault(s):" << std::endl;
    for (const auto& fault : data) {
      std::cout << "  └─ Level: " << static_cast<int>(fault.level)
                << ", Code: " << static_cast<int>(fault.code)
                << ", Message: " << fault.message << std::endl;
    }
  }

  void OnControlLost([[maybe_unused]] const ControlLostInfo& info) override {
    std::cout << "\n[WARN] Control Lost! " << std::endl;
  }

  void OnControlAvailable(
      [[maybe_unused]] const ControlAvailableInfo& info) override {
    std::cout << "\n[INFO] Control Available! " << std::endl;
  }
};

/**
 * @brief Control Callback Class - Receives robot confirmation for control
 * commands
 *
 * After the SDK sends control commands, the robot confirms receipt through this
 * callback
 */
class ControlCallback : public IControlCallback {
 public:
  void OnSoftEmergencyStop(bool on) override {
    std::cout << "[CTRL] ✓ Emergency Stop: " << (on ? "ON" : "OFF")
              << std::endl;
  }

  void OnStandUp() override { std::cout << "[CTRL] ✓ Stand Up" << std::endl; }

  void OnBalanceStandUp() override {
    std::cout << "[CTRL] ✓ Balance Stand Up" << std::endl;
  }

  void OnLieDown() override { std::cout << "[CTRL] ✓ Lie Down" << std::endl; }

  void OnStair() override { std::cout << "[CTRL] ✓ Stair" << std::endl; }

  void OnCrawl() override { std::cout << "[CTRL] ✓ Crawl" << std::endl; }

  void OnCrawlWalk() override {
    std::cout << "[CTRL] ✓ Crawl Walk" << std::endl;
  }

  void OnClimb() override { std::cout << "[CTRL] ✓ Climb" << std::endl; }

  void OnGait() override { std::cout << "[CTRL] ✓ Gait" << std::endl; }

  void OnSlim() override { std::cout << "[CTRL] ✓ Slim" << std::endl; }

  void OnDSB() override { std::cout << "[CTRL] ✓ DSB" << std::endl; }

  void OnPosControl() override {
    std::cout << "[CTRL] ✓ Pos Control" << std::endl;
  }

  void OnSkWalk() override {
    std::cout << "[CTRL] ✓ SameKnee Walk" << std::endl;
  }

  void OnSand() override { std::cout << "[CTRL] ✓ Sand" << std::endl; }

  void OnReverseHeadTail() override {
    std::cout << "[CTRL] ✓ Reverse Head/Tail" << std::endl;
  }

  void OnSpeed(int speed_level) override {
    const char* speed_name[] = {"Stop", "Low", "Medium", "High"};
    std::cout << "[CTRL] ✓ Speed: "
              << (speed_level < 4 ? speed_name[speed_level] : "Invalid") << " ("
              << speed_level << ")" << std::endl;
  }

  void OnLocked() override { std::cout << "[CTRL] ✓ Locked" << std::endl; }

  void OnFrontLight(bool on) override {
    std::cout << "[CTRL] ✓ Front Light: " << (on ? "ON" : "OFF") << std::endl;
  }

  void OnBackLight(bool on) override {
    std::cout << "[CTRL] ✓ Back Light: " << (on ? "ON" : "OFF") << std::endl;
  }

  void OnAutoModeLight(bool on) override {
    std::cout << "[CTRL] ✓ Auto Mode Light: " << (on ? "ON" : "OFF")
              << std::endl;
  }

  void OnObstacleAvoidance(bool on) override {
    std::cout << "[CTRL] ✓ Obstacle Avoidance: " << (on ? "ON" : "OFF")
              << std::endl;
  }

  void OnTakeControlAck(const TakeControlAck& ack) override {
    if (ack.error_code == 0) {
      std::cout << "[CTRL] ✓ Take Control Success" << std::endl;
    } else {
      std::cout << "[CTRL] ✗ Take Control Failed, Reason: " << ack.reason
                << std::endl;
    }
  }
  void OnReleaseControlAck(const ReleaseControlAck& ack) override {
    if (ack.error_code == 0) {
      std::cout << "[CTRL] ✓ Release Control Success" << std::endl;
    } else {
      std::cout << "[CTRL] ✗ Release Control Failed, Reason: " << ack.reason
                << std::endl;
    }
  }

  void OnSwitchRemote() override {
    std::cout << "[CTRL] ✓ Switch Remote State" << std::endl;
  }

  void OnSwitchIdle() override {
    std::cout << "[CTRL] ✓ Switch Idle State" << std::endl;
  }

  void OnSetPeriphPower(const PowerCtrlAck& ack) override {
    std::cout << "[CTRL] ✓ Set Peripheral Power Control: "
              << (ack.enable ? "ON" : "OFF") << std::endl;
  }

  void OnGetPeriphPower(const PowerCtrlAck& ack) override {
    std::cout << "[CTRL] ✓ Get Peripheral Power Control: "
              << (ack.enable ? "ON" : "OFF") << std::endl;
  }

  void OnSetLedAutoMode(const LedAutoModeAck& ack) override {
    std::cout << "[CTRL] ✓ Set LED Auto Mode: "
              << (ack.auto_mode ? "AUTO" : "MANUAL") << std::endl;
  }

  void OnGetLedAutoMode(const LedAutoModeAck& ack) override {
    std::cout << "[CTRL] ✓ Get LED Auto Mode: "
              << (ack.auto_mode ? "AUTO" : "MANUAL") << std::endl;
  }

  void OnSetLedCommand(const LedCommandAck& ack) override {
    std::cout << "[CTRL] ✓ Set LED Command: id=" << static_cast<int>(ack.id)
              << ", effect=" << static_cast<int>(ack.effect) << ", color=("
              << static_cast<int>(ack.color.r) << ", "
              << static_cast<int>(ack.color.g) << ", "
              << static_cast<int>(ack.color.b) << ", "
              << static_cast<int>(ack.color.a) << ")"
              << ", duration_ms=" << ack.duration_ms << std::endl;
  }
};

/**
 * @brief Print robot state information
 */
void PrintRobotState(const RobotState& data) {
  std::cout << "\n========== Robot State ==========" << std::endl;

  // Posture information
  std::cout << "[Pose]" << std::endl;
  std::cout << "  Head Angle: " << std::fixed << std::setprecision(2)
            << data.head_angle << "°" << std::endl;
  std::cout << "  Head Direction: " << static_cast<int>(data.head_direction)
            << std::endl;
  std::cout << "  Motion Status: " << g_motion_status_map.at(data.motion_status)
            << std::endl;
  std::cout << "  Machine Status: "
            << g_machine_status_map.at(data.machine_status) << std::endl;

  // Speed information
  std::cout << "[Speed]" << std::endl;
  std::cout << "  Speed Level: " << g_speed_level_map.at(data.speed_level)
            << std::endl;
  std::cout << "  Linear: " << data.speed.line << " m/s" << std::endl;
  std::cout << "  Translation: " << data.speed.translation << " m/s"
            << std::endl;
  std::cout << "  Angular: " << data.speed.angle << " rad/s" << std::endl;

  // Light status
  std::cout << "[Lights]" << std::endl;
  std::cout << "  Front: "
            << (data.front_fill_light == FillLightStatus::FILL_LIGHT_STATUS_ON
                    ? "ON"
                    : "OFF")
            << ", Back: "
            << (data.back_fill_light == FillLightStatus::FILL_LIGHT_STATUS_ON
                    ? "ON"
                    : "OFF")
            << ", Auto: " << (data.auto_mode_light ? "ON" : "OFF") << std::endl;

  // Safety status
  std::cout << "[Safety]" << std::endl;
  std::cout << "  Obstacle Avoidance: "
            << (data.obstacle_avoidance ? "ON" : "OFF") << std::endl;
  std::cout << "  Charging Pile: "
            << (data.charging_pile_connected ? "CONNECTED" : "DISCONNECTED")
            << std::endl;

  // Emergency stop status
  std::cout << "[Emergency]" << std::endl;
  std::cout << "  Software: "
            << (data.software_emergency_status ==
                        EmergencyStatus::EMERGENCY_STATUS_STOP
                    ? "ACTIVE"
                    : "INACTIVE")
            << ", Hardware: "
            << (data.hardware_emergency_status ==
                        EmergencyStatus::EMERGENCY_STATUS_STOP
                    ? "ACTIVE"
                    : "INACTIVE")
            << std::endl;

  // Battery information
  std::cout << "[Battery]" << std::endl;
  std::cout << "  Battery 1: " << static_cast<int>(data.battery.power1) << "%"
            << ", " << data.battery.voltage1 << "V"
            << ", " << data.battery.current1 << "A"
            << ", " << data.battery.temperature1 << "°C"
            << ", Status: "
            << static_cast<int>(data.battery.power_supply_status1) << std::endl;
  std::cout << "  Battery 2: " << static_cast<int>(data.battery.power2) << "%"
            << ", " << data.battery.voltage2 << "V"
            << ", " << data.battery.current2 << "A"
            << ", " << data.battery.temperature2 << "°C"
            << ", Status: "
            << static_cast<int>(data.battery.power_supply_status2) << std::endl;

  // Odometry information
  std::cout << "[Odometry]" << std::endl;
  std::cout << "  Total Distance: " << data.mile_data << " m" << std::endl;

  std::cout << "[ControlSource:]" << std::endl;
  std::cout << "  Control Source: "
            << (data.control_source == CtrlSource::CTRL_SOURCE_APP   ? "APP"
                : data.control_source == CtrlSource::CTRL_SOURCE_SDK ? "SDK"
                : data.control_source == CtrlSource::CTRL_SOURCE_OTHER
                    ? "OTHER"
                    : "UNKNOWN")
            << std::endl;

  std::cout << "[Joint Temperature]" << std::endl;
  std::cout << "  Joint Temperatures: " << std::endl;
  for (const auto& [joint, temp] : data.joint_temps) {
    std::cout << "    " << joint << ": " << temp << "°C" << std::endl;
  }

  std::cout << "================================\n" << std::endl;
}

// Command handler function type
using CommandHandler = std::function<void(SDKClient&)>;

/**
 * @brief Print control help information
 */
void PrintHelp() {
  std::cout << "\n========== Control Commands ==========" << std::endl;
  std::cout << "[Speed]    4:Low  5:Medium  6:High" << std::endl;
  std::cout << "[Move]     W:Forward  S:Backward  A:Left  D:Right  P:Stop"
            << std::endl;
  std::cout << "[PosMove]  +:Z+  -:Z- " << std::endl;
  std::cout << "[Yaw]      L:Left  R:Right" << std::endl;
  std::cout << "[Roll]     7:Left roll  8:Right roll" << std::endl;
  std::cout << "[Stance]   <:Low  >:High  ?:Restore" << std::endl;
  std::cout << "[Head]     9:Look Left  0:Look Up" << std::endl;
  std::cout << "[Pose]     1:BalanceStandUp  2:CrawlWalk  3:Stair  Z:Stand  "
               "X:Crawl  C:Lie  G:Gait  J:Climb  K:Slim  U:PosControl  /:SkWalk"
               "  ;:Sand"
            << std::endl;
  std::cout << "[Light]    F:Front  B:Back  N:Auto" << std::endl;
  std::cout << "[LED]      Shift+N:AutoMode  =:GetAuto  {:BlinkOrange  }:Off"
            << std::endl;
  std::cout << "[Safety]   I:ObstacleAvoidance" << std::endl;
  std::cout << "[System]   E:E-Stop  M:Lock  V:Reverse Head/Tail" << std::endl;
  std::cout << "[Control]  T:TakeControl Y:ReleaseControl Shift+R:RemoteState "
               "Shift+I:IdleState"
            << std::endl;
  std::cout << "[General]  Space:Stop  O:Status  H:Help  Q:Quit"
            << std::endl;
  std::cout << "[PowerCtrl] ,:Set M1_12V  .:Get M1_12V" << std::endl;
  std::cout << "======================================\n" << std::endl;
}

/**
 * @brief Create keyboard command handler mapping table
 * @param sdk_client SDK client reference
 * @return Command mapping table
 */
std::map<char, CommandHandler> CreateCommandTable(SDKClient& sdk_client) {
  return {
      // ============ Posture Control ============
      {'1', [](SDKClient& client) { client.BalanceStandUp(); }},
      {'2', [](SDKClient& client) { client.CrawlWalk(); }},
      {'3', [](SDKClient& client) { client.Stair(); }},

      // ============ Speed Control ============
      {'4', [](SDKClient& client) { client.SetSpeed(1); }},  // Low
      {'5', [](SDKClient& client) { client.SetSpeed(2); }},  // Medium
      {'6', [](SDKClient& client) { client.SetSpeed(3); }},  // High

      // ============ BalanceStandUp Operations ============
      {'7', [](SDKClient& client) { client.Turn(1); }},  // Left roll
      {'8', [](SDKClient& client) { client.Turn(2); }},  // Right roll
      {'9',
       [](SDKClient& client) { client.ControlHead(0.5, 0.0); }},  // Left peek
      {'0',
       [](SDKClient& client) { client.ControlHead(0.0, 0.5); }},  // Look up
      {'<',
       [](SDKClient& client) { client.HighLowStance(2); }},  // Low stance
      {'>',
       [](SDKClient& client) { client.HighLowStance(1); }},  // High stance
      {'?',
       [](SDKClient& client) { client.HighLowStance(0); }},  // Restore stance

      // ============ Directional Movement ============
      {'w', [](SDKClient& client) { client.Move(0.0, 0.11, 0.0); }},  // Forward
      {'a', [](SDKClient& client) { client.Move(0.1, 0.0, 0.0); }},   // Left
      {'s',
       [](SDKClient& client) { client.Move(0.0, -0.11, 0.0); }},  // Backward
      {'d', [](SDKClient& client) { client.Move(-0.1, 0.0, 0.0); }},  // Right

      // ============ Turn Control ============
      {'l',
       [](SDKClient& client) { client.Move(0.0, 0.0, 0.1); }},  // Left turn
      {'r',
       [](SDKClient& client) { client.Move(0.0, 0.0, -0.1); }},  // Right turn

      // ============ Posture Control ============
      {'z', [](SDKClient& client) { client.StandUp(); }},  // Stand
      {'x', [](SDKClient& client) { client.Crawl(); }},    // Crawl
      {'c', [](SDKClient& client) { client.LieDown(); }},  // Lie down
      {'v',
       [](SDKClient& client) { client.ReverseHeadTail(); }},  // Toggle head
      {'g', [](SDKClient& client) { client.Gait(); }},        // Gait
      {'j', [](SDKClient& client) { client.Climb(); }},       // Climb
      {'k', [](SDKClient& client) { client.Slim(); }},        // Slim
      {'u', [](SDKClient& client) { client.PosControl(); }},  // PosControl
      {'/', [](SDKClient& client) { client.SkWalk(); }},      // SkWalk
      {';', [](SDKClient& client) { client.Sand(); }},        // Sand

      {'+',
       [](SDKClient& client) {
         PosControlCmd cmd{0.0, 0.0, 0.2, 0.0, 0.0, 0.0};
         client.PosMove(cmd);
       }},
      {'-',
       [](SDKClient& client) {
         PosControlCmd cmd{0.0, 0.0, -0.2, 0.0, 0.0, 0.0};
         client.PosMove(cmd);
       }},
      {',',
       [](SDKClient& client) {
         PowerCtrlCfg cfg{PeripheralPower::M1_12V, !g_robot_m1_12v.load()};
         g_robot_m1_12v = !g_robot_m1_12v.load();
         client.SetPeriphPower(cfg);
       }},
      {'.',
       [](SDKClient& client) {
         PowerCtrlCfg cfg{PeripheralPower::M1_12V};
         client.GetPeriphPower(cfg);
       }},

      // ============ Control Authority Management ============
      {'t', [](SDKClient& client) { client.TakeControl(); }},  // Take control
      {'y',
       [](SDKClient& client) { client.ReleaseControl(); }},  // Release control
      {'R',
       [](SDKClient& client) {
         client.SwitchRemoteState();
       }},  // Switch to remote state
      {'I',
       [](SDKClient& client) {
         client.SwitchIdleState();
       }},  // Switch to idle state

      // ============ Light Control ============
      {'f',
       [](SDKClient& client) {  // Front fill light
         bool new_state = !g_front_light_on.load();
         g_front_light_on = new_state;
         auto err = client.FrontLight(
             new_state, 0, [](const std::error_code& ec, std::size_t) {
               if (ec) {
                 std::cerr << "[ERROR] FrontLight command failed: "
                           << ec.message() << std::endl;
               }
             });

       }},
      {'b',
       [](SDKClient& client) {  // Back fill light
         bool new_state = !g_back_light_on.load();
         g_back_light_on = new_state;
         auto err = client.BackLight(
             new_state, 0, [](const std::error_code& ec, std::size_t) {
               if (ec) {
                 std::cerr << "[ERROR] BackLight command failed: "
                           << ec.message() << std::endl;
               }
             });
       }},
      {'n',
       [](SDKClient& client) {  // Auto mode light
         bool new_state = !g_auto_mode_light_on.load();
         g_auto_mode_light_on = new_state;
         client.AutoModeLight(new_state);
       }},
      {'N',
       [](SDKClient& client) {  // LED auto/manual mode
         bool new_state = !g_led_auto_mode_on.load();
         g_led_auto_mode_on = new_state;
         auto err = client.SetLedAutoMode(
             new_state, 0, [](const std::error_code& ec, std::size_t) {
               if (ec) {
                 std::cerr << "[ERROR] SetLedAutoMode command failed: "
                           << ec.message() << std::endl;
               }
             });
         if (err) {
           std::cerr << "[ERROR] SetLedAutoMode command rejected: "
                     << err.message() << std::endl;
         }
       }},
      {'=',
       [](SDKClient& client) {  // Get LED auto/manual mode
         auto err = client.GetLedAutoMode(
             0, [](const std::error_code& ec, std::size_t) {
               if (ec) {
                 std::cerr << "[ERROR] GetLedAutoMode command failed: "
                           << ec.message() << std::endl;
               }
             });
         if (err) {
           std::cerr << "[ERROR] GetLedAutoMode command rejected: "
                     << err.message() << std::endl;
         }
       }},
      {'{',
       [](SDKClient& client) {  // LED blink orange
         LedCommand cmd{LedId::ALL, LedEffect::BLINK, {255, 128, 0, 255}, 1000};
         auto err = client.SetLedCommand(
             cmd, 0, [](const std::error_code& ec, std::size_t) {
               if (ec) {
                 std::cerr << "[ERROR] SetLedCommand blink failed: "
                           << ec.message() << std::endl;
               }
             });
         if (err) {
           std::cerr << "[ERROR] SetLedCommand blink rejected: "
                     << err.message() << std::endl;
         }
       }},
      {'}',
       [](SDKClient& client) {  // LED off
         LedCommand cmd{LedId::ALL, LedEffect::OFF, {0, 0, 0, 255}, 0};
         auto err = client.SetLedCommand(
             cmd, 0, [](const std::error_code& ec, std::size_t) {
               if (ec) {
                 std::cerr << "[ERROR] SetLedCommand off failed: "
                           << ec.message() << std::endl;
               }
             });
         if (err) {
           std::cerr << "[ERROR] SetLedCommand off rejected: " << err.message()
                     << std::endl;
         }
       }},

      // ============ Safety Test ============
      {'i',
       [](SDKClient& client) {  // Obstacle avoidance
         bool new_state = !g_obstacle_avoidance_on.load();
         g_obstacle_avoidance_on = new_state;
         auto err = client.ObstacleAvoidance(
             new_state, 0, [](const std::error_code& ec, std::size_t) {
               if (ec) {
                 std::cerr << "[ERROR] ObstacleAvoidance command failed: "
                           << ec.message() << std::endl;
               }
             });
         if (err) {
           std::cerr << "[ERROR] ObstacleAvoidance command rejected: "
                     << err.message() << std::endl;
         }
       }},

      // ============ System Control ============
      {'e',
       [](SDKClient& client) {  // Emergency stop
         bool new_state = !g_estop_on.load();
         g_estop_on = new_state;
         client.SoftEmergencyStop(new_state);
       }},
      {'m', [](SDKClient& client) { client.Locked(); }},  // Lock

      // ============ Stop and Status ============
      {' ',
       [](SDKClient& client) {  // Stop all motion
         client.Move(0.0, 0.0, 0.0);
         client.Turn(0);
         client.ControlHead(0.0, 0.0);
         std::cout << "[CMD] Stop all motion" << std::endl;
       }},
      {'p',
       [](SDKClient& client) {  // Stop all motion
         client.Move(0.0, 0.0, 0.0);
         client.Turn(0);
         client.ControlHead(0.0, 0.0);
         std::cout << "[CMD] Stop all motion" << std::endl;
       }},
      {'o',
       [](SDKClient& client) {  // Print status
         std::lock_guard<std::mutex> lock(g_state_mtx);
         PrintRobotState(g_robot_state);
       }},
      {'h', [](SDKClient& client) { PrintHelp(); }},  // Help

      // ============ Others ============
      {'\n', [](SDKClient& client) {}},  // Enter (no-op
  };
}

int main(int argc, char* argv[]) {
  // Install signal handlers
  std::signal(SIGINT, SignalHandler);
  std::signal(SIGTERM, SignalHandler);

  // Parse command line arguments
  if (argc < 3) {
    std::cerr << "Usage: " << argv[0] << " <ip> <port>" << std::endl;
    std::cerr << "Example: " << argv[0] << " 192.168.234.1 8082" << std::endl;
    return EXIT_FAILURE;
  }

  const std::string ip = argv[1];
  const std::string port = argv[2];

  std::cout << "========================================" << std::endl;
  std::cout << "  Robot SDK Interactive Control Demo" << std::endl;
  std::cout << "========================================" << std::endl;
  std::cout << "Target: " << ip << ":" << port << std::endl;
  std::cout << "========================================\n" << std::endl;

  // Initialize SDK client
  SDKClient sdk_client;

  std::cout << "[INFO] SDK Version: " << sdk_client.Version() << std::endl;
  std::cout << "[INFO] Protocol Version: " << sdk_client.ProtocolVersion()
            << "\n"
            << std::endl;

  // Set callbacks
  auto data_cb = std::make_shared<DataCallback>();
  auto ctrl_cb = std::make_shared<ControlCallback>();
  sdk_client.SetDataCallback(data_cb);
  sdk_client.SetControlCallback(ctrl_cb);

  // Asynchronous connection to robot
  std::cout << "[INIT] Connecting to robot..." << std::endl;
  {
    std::unique_lock<std::mutex> lock(g_connect_mtx);
    sdk_client.Connect(ip, port, false, [](const std::error_code& ec) {
      std::unique_lock<std::mutex> lock(g_connect_mtx);
      if (ec) {
        std::cerr << "[ERROR] Connect failed: " << ec.message() << std::endl;
      } else {
        std::cout << "[INIT] ✓ Connected successfully" << std::endl;
      }
      g_connect_cv.notify_one();
    });

    // Wait for connection to complete
    auto status = g_connect_cv.wait_for(lock, std::chrono::seconds(10));
    if (status == std::cv_status::timeout) {
      std::cerr << "[ERROR] Connection timeout" << std::endl;
      return EXIT_FAILURE;
    }
  }

  // Check connection status
  if (sdk_client.GetConnectionState() != ConnectionState::CONNECTED) {
    std::cerr << "[ERROR] Not connected. State: "
              << static_cast<int>(sdk_client.GetConnectionState()) << std::endl;
    return EXIT_FAILURE;
  }

  // Get system version
  std::cout << "[INFO] System Version: " << sdk_client.SystemVersion()
            << std::endl;

  // Print device information cached from the handshake
  const auto device_info = sdk_client.GetDeviceInfo();
  std::cout << "[INFO] Device Type: "
            << robot_sdk::DeviceTypeName(device_info.device_type) << std::endl;
  std::cout << "[INFO] Device SN: " << device_info.sn << "\n" << std::endl;

  // Print control help
  PrintHelp();

  // Create command mapping table
  auto command_table = CreateCommandTable(sdk_client);

  // Main control loop
  std::cout << "[READY] Waiting for commands...\n" << std::endl;
  while (g_running) {
    char c = std::getchar();

    // Quit command
    if (c == 'q' || c == 'Q') {
      std::cout << "[INFO] Quit command received" << std::endl;
      break;
    }

    // Find and execute command
    auto it = command_table.find(c);
    if (it != command_table.end()) {
      try {
        it->second(sdk_client);
      } catch (const std::exception& e) {
        std::cerr << "[ERROR] Command execution failed: " << e.what()
                  << std::endl;
      }
    } else if (c != '\n') {
      // Unrecognized command (ignore newline)
      std::cout << "[WARN] Unknown command: '" << c << "' (Press H for help)"
                << std::endl;
    }
  }

  // Clean up and exit
  std::cout << "\n[SHUTDOWN] Disconnecting..." << std::endl;
  {
    std::unique_lock<std::mutex> lock(g_connect_mtx);
    sdk_client.Disconnect(false, [](const std::error_code& ec) {
      std::unique_lock<std::mutex> lock(g_connect_mtx);
      if (ec) {
        std::cerr << "[ERROR] Disconnect failed: " << ec.message() << std::endl;
      } else {
        std::cout << "[SHUTDOWN] ✓ Disconnected" << std::endl;
      }
      g_connect_cv.notify_one();
    });

    auto status = g_connect_cv.wait_for(lock, std::chrono::seconds(3));
    if (status == std::cv_status::timeout) {
      std::cerr << "[WARN] Disconnect timeout" << std::endl;
    }
  }

  std::cout << "[SHUTDOWN] ✓ Exited cleanly" << std::endl;
  return EXIT_SUCCESS;
}
