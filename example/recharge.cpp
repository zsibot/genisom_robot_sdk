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
static std::atomic<bool> g_estop_on{false};

// Synchronization primitives
static std::mutex g_connect_mtx;
static std::condition_variable g_connect_cv;

// Robot state cache
static std::mutex g_state_mtx;
static RobotState g_robot_state;

// State string mappings

const std::unordered_map<TaskType, const char*> g_task_type_map = {
    {TaskType::UNKNOWN, "Unknown"},
    {TaskType::SCAN_QR, "Scan QR"},
    {TaskType::MAPPING, "Mapping"},
    {TaskType::NAV, "Navigation"},
    {TaskType::RECHARGING, "Recharging"},
    {TaskType::UNDOCK, "Undock"},
    {TaskType::UWB_FOLLOW, "UWB Follow"},
    {TaskType::VISUAL_TRACK, "Visual Track"},
};

const std::unordered_map<TaskStatus, const char*> g_task_status_map = {
    {TaskStatus::UNKNOWN, "Unknown"}, {TaskStatus::STARTING, "Starting"},
    {TaskStatus::RUNNING, "Running"}, {TaskStatus::SUCCESS, "Success"},
    {TaskStatus::FAILURE, "Failure"}, {TaskStatus::STOPPED, "Stopped"},
};

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

  void OnTaskStateData(const TaskStateInfo& info) override {
    std::cout << "\n[TASK] Task State Update:" << std::endl;
    std::cout << "  Task Type: " << g_task_type_map.at(info.task_type)
              << ", Status: " << g_task_status_map.at(info.task_status)
              << ", Phase: " << info.phase
              << ", Error Code: " << info.error_code << std::endl;
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
  void OnStartRechargeTask() override {
    std::cout << "\n[CTRL] ✓ Start Recharge Task command acknowledged by robot"
              << std::endl;
  }

  void OnStopRechargeTask() override {
    std::cout << "\n[CTRL] ✓ Stop Recharge Task command acknowledged by robot"
              << std::endl;
  }

  void OnStartUnDockTask() override {
    std::cout << "\n[CTRL] ✓ Start UnDock Task command acknowledged by robot"
              << std::endl;
  }

  void OnStopUnDockTask() override {
    std::cout << "\n[CTRL] ✓ Stop UnDock Task command acknowledged by robot"
              << std::endl;
  }

  void OnSwitchRemote() override {
    std::cout << "\n[CTRL] ✓ Robot switched to remote control mode"
              << std::endl;
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

  std::cout << "================================\n" << std::endl;
}

// Command handler function type
using CommandHandler = std::function<void(SDKClient&)>;

/**
 * @brief Print control help information
 */
void PrintHelp() {
  std::cout << "\n========== Control Commands ==========" << std::endl;
  std::cout << "[CHARGING] 1:StartRecharge  2:StopRecharge  3:StartUnDock  "
               "4:StopUnDock"
            << std::endl;
  std::cout << "[Move]     W:Forward  S:Backward  A:Left  D:Right" << std::endl;
  std::cout << "[Control]  O:Status H:Help  Q:Quit" << std::endl;
  std::cout << "======================================\n" << std::endl;
}

/**
 * @brief Create keyboard command handler mapping table
 * @param sdk_client SDK client reference
 * @return Command mapping table
 */
std::map<char, CommandHandler> CreateCommandTable(SDKClient& sdk_client) {
  return {

      {'1', [](SDKClient& client) { client.StartRechargeTask(); }},

      {'2', [](SDKClient& client) { client.StopRechargeTask(); }},

      {'3', [](SDKClient& client) { client.StartUnDockTask(); }},

      {'4', [](SDKClient& client) { client.StopUnDockTask(); }},

      {'r', [](SDKClient& client) { client.SwitchRemoteState(); }},
      // ============ Directional Movement ============
      {'w', [](SDKClient& client) { client.Move(0.0, 0.11, 0.0); }},  // Forward
      {'a', [](SDKClient& client) { client.Move(0.1, 0.0, 0.0); }},   // Left
      {'s',
       [](SDKClient& client) { client.Move(0.0, -0.11, 0.0); }},  // Backward
      {'d', [](SDKClient& client) { client.Move(-0.1, 0.0, 0.0); }},  // Right
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
  std::cout << "  Robot SDK Recharge Demo" << std::endl;
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
  std::cout << "[INFO] System Version: " << sdk_client.SystemVersion() << "\n"
            << std::endl;

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