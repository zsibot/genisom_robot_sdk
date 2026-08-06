/**
 * @file led.cpp
 * @brief Robot SDK LED Control Example
 *
 * This example demonstrates LED auto/manual mode control and LED effect
 * commands through SDK protocol 1019/1020.
 */

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <csignal>
#include <iostream>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>

#include "robot_sdk/sdk_client.hpp"

using namespace robot_sdk;

static std::atomic<bool> g_running{true};
static std::mutex g_connect_mtx;
static std::condition_variable g_connect_cv;

const char* LedIdName(LedId id) {
  switch (id) {
    case LedId::ALL:
      return "all";
    case LedId::FRONT:
      return "front";
    case LedId::BACK:
      return "back";
    default:
      return "unknown";
  }
}

const char* LedEffectName(LedEffect effect) {
  switch (effect) {
    case LedEffect::OFF:
      return "off";
    case LedEffect::ON:
      return "on";
    case LedEffect::BREATH:
      return "breath";
    case LedEffect::BLINK:
      return "blink";
    case LedEffect::BLINK_TRANSIENT:
      return "blink_transient";
    default:
      return "unknown";
  }
}

LedId ParseLedId(const std::string& value) {
  if (value == "all") return LedId::ALL;
  if (value == "front") return LedId::FRONT;
  if (value == "back") return LedId::BACK;
  return LedId::UNKNOWN;
}

LedEffect ParseLedEffect(const std::string& value) {
  if (value == "off") return LedEffect::OFF;
  if (value == "on") return LedEffect::ON;
  if (value == "breath") return LedEffect::BREATH;
  if (value == "blink") return LedEffect::BLINK;
  if (value == "blink_transient") return LedEffect::BLINK_TRANSIENT;
  return LedEffect::UNKNOWN;
}

class LedCallback : public IControlCallback {
 public:
  void OnSetLedAutoMode(const LedAutoModeAck& ack) override {
    std::cout << "[ACK] Set LED mode: "
              << (ack.auto_mode ? "auto" : "manual") << std::endl;
  }

  void OnGetLedAutoMode(const LedAutoModeAck& ack) override {
    std::cout << "[ACK] Current LED mode: "
              << (ack.auto_mode ? "auto" : "manual") << std::endl;
  }

  void OnSetLedCommand(const LedCommandAck& ack) override {
    std::cout << "[ACK] LED command: id=" << LedIdName(ack.id)
              << ", effect=" << LedEffectName(ack.effect) << ", color=("
              << static_cast<int>(ack.color.r) << ", "
              << static_cast<int>(ack.color.g) << ", "
              << static_cast<int>(ack.color.b) << ", "
              << static_cast<int>(ack.color.a)
              << "), duration_ms=" << ack.duration_ms << std::endl;
  }
};

void SignalHandler(int signal) {
  std::cout << "\n[INFO] Received signal " << signal << ", shutting down..."
            << std::endl;
  g_running = false;
}

void PrintHelp() {
  std::cout << "\n========== LED Commands ==========" << std::endl;
  std::cout << "  auto                         - Enable LED auto mode" << std::endl;
  std::cout << "  manual                       - Enable LED manual mode" << std::endl;
  std::cout << "  get                          - Query LED auto/manual mode"
            << std::endl;
  std::cout << "  off [all|front|back]          - Turn LED group off" << std::endl;
  std::cout << "  on <id> <r> <g> <b> [a]       - Solid color" << std::endl;
  std::cout << "  breath <id> <r> <g> <b> <ms>  - Breathing effect"
            << std::endl;
  std::cout << "  blink <id> <r> <g> <b> <ms>   - Blink effect" << std::endl;
  std::cout << "  transient <id> <r> <g> <b> <ms> - Transient blink"
            << std::endl;
  std::cout << "  h                            - Show help" << std::endl;
  std::cout << "  q                            - Quit" << std::endl;
  std::cout << "\nExamples:" << std::endl;
  std::cout << "  manual" << std::endl;
  std::cout << "  blink all 255 128 0 300" << std::endl;
  std::cout << "  breath front 0 0 255 1000" << std::endl;
  std::cout << "  off all" << std::endl;
  std::cout << "==================================\n" << std::endl;
}

bool ParseByte(const std::string& value, uint8_t& out) {
  try {
    auto parsed = std::stoul(value);
    if (parsed > 255) return false;
    out = static_cast<uint8_t>(parsed);
    return true;
  } catch (...) {
    return false;
  }
}

bool ParseLedCommandLine(const std::string& line, LedCommand& cmd) {
  std::istringstream iss(line);
  std::string effect;
  std::string id;
  std::string r;
  std::string g;
  std::string b;
  std::string a_or_duration;
  std::string duration;

  iss >> effect;
  if (effect == "transient") effect = "blink_transient";
  if (effect == "off") {
    if (!(iss >> id)) id = "all";
    cmd = {ParseLedId(id), LedEffect::OFF, {0, 0, 0, 255}, 0};
    return cmd.id != LedId::UNKNOWN;
  }

  if (!(iss >> id >> r >> g >> b)) return false;

  cmd.id = ParseLedId(id);
  cmd.effect = ParseLedEffect(effect);
  cmd.color.a = 255;
  if (cmd.id == LedId::UNKNOWN || cmd.effect == LedEffect::UNKNOWN) {
    return false;
  }
  if (!ParseByte(r, cmd.color.r) || !ParseByte(g, cmd.color.g) ||
      !ParseByte(b, cmd.color.b)) {
    return false;
  }

  if (cmd.effect == LedEffect::ON) {
    if (iss >> a_or_duration) {
      if (!ParseByte(a_or_duration, cmd.color.a)) return false;
    }
    cmd.duration_ms = 0;
    return true;
  }

  if (!(iss >> a_or_duration)) return false;
  try {
    cmd.duration_ms = static_cast<uint32_t>(std::stoul(a_or_duration));
  } catch (...) {
    return false;
  }
  return true;
}

void ProcessCommand(const std::string& line, SDKClient& sdk_client) {
  if (line.empty()) return;

  if (line == "h" || line == "help") {
    PrintHelp();
    return;
  }
  if (line == "q" || line == "quit") {
    g_running = false;
    return;
  }

  std::error_code ec;
  if (line == "auto") {
    ec = sdk_client.SetLedAutoMode(true, 0);
  } else if (line == "manual") {
    ec = sdk_client.SetLedAutoMode(false, 0);
  } else if (line == "get") {
    ec = sdk_client.GetLedAutoMode(0);
  } else {
    LedCommand cmd{};
    if (!ParseLedCommandLine(line, cmd)) {
      std::cout << "[ERROR] Invalid LED command. Press h for help."
                << std::endl;
      return;
    }
    ec = sdk_client.SetLedCommand(cmd, 0);
  }

  if (ec) {
    std::cerr << "[ERROR] Command rejected: " << ec.message() << std::endl;
  }
}

int main(int argc, char* argv[]) {
  std::signal(SIGINT, SignalHandler);
  std::signal(SIGTERM, SignalHandler);

  if (argc < 3) {
    std::cerr << "Usage: " << argv[0] << " <ip> <port>" << std::endl;
    std::cerr << "Example: " << argv[0] << " 192.168.234.1 8082"
              << std::endl;
    return EXIT_FAILURE;
  }

  SDKClient sdk_client;
  sdk_client.SetControlCallback(std::make_shared<LedCallback>());

  std::cout << "[INIT] Connecting to " << argv[1] << ":" << argv[2] << "..."
            << std::endl;
  {
    std::unique_lock<std::mutex> lock(g_connect_mtx);
    sdk_client.Connect(argv[1], argv[2], false, [](const std::error_code& ec) {
      std::unique_lock<std::mutex> lock(g_connect_mtx);
      if (ec) {
        std::cerr << "[ERROR] Connect failed: " << ec.message() << std::endl;
      } else {
        std::cout << "[INIT] Connected successfully" << std::endl;
      }
      g_connect_cv.notify_one();
    });

    if (g_connect_cv.wait_for(lock, std::chrono::seconds(10)) ==
        std::cv_status::timeout) {
      std::cerr << "[ERROR] Connection timeout" << std::endl;
      return EXIT_FAILURE;
    }
  }

  if (sdk_client.GetConnectionState() != ConnectionState::CONNECTED) {
    std::cerr << "[ERROR] Not connected" << std::endl;
    return EXIT_FAILURE;
  }

  PrintHelp();
  std::string line;
  while (g_running) {
    std::cout << "led> " << std::flush;
    if (!std::getline(std::cin, line)) break;
    ProcessCommand(line, sdk_client);
  }

  std::cout << "[SHUTDOWN] Disconnecting..." << std::endl;
  sdk_client.Disconnect(true);
  std::cout << "[SHUTDOWN] Exited" << std::endl;
  return EXIT_SUCCESS;
}
