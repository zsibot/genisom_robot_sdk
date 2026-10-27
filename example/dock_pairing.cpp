/**
 * @file dock_pairing.cpp
 * @brief Interactive charging dock scan and pairing example.
 *
 * Usage:
 *   ./dock_pairing <ip> <port> [dock_mac]
 */

#include <atomic>
#include <csignal>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <mutex>
#include <optional>
#include <string>

#include "robot_sdk/sdk_client.hpp"

namespace {

using namespace robot_sdk;

std::atomic<bool> g_running{true};

void SignalHandler(int signal) {
  std::cout << "\n[INFO] Received signal " << signal << ", shutting down..."
            << std::endl;
  g_running = false;
}

void PrintDockInfo(const DockInfo& info) {
  std::cout << "\n[1023 INFO] code=" << info.code
            << "\n  ble_address=" << info.ble_address
            << "\n  name=" << info.name
            << "\n  dock_connected=" << std::boolalpha << info.dock_connected
            << "\n  has_contact=" << info.has_contact
            << "\n  charging_active=" << info.charging_active
            << "\n  light_switch_status=" << info.light_switch_status
            << "\n  fill_light_brightness="
            << static_cast<unsigned int>(info.fill_light_brightness)
            << "\n  dock_sn=" << info.dock_sn
            << "\n  dock_version=" << info.dock_version
            << "\n  error_code=" << info.error_code
            << "\n  error_message=" << info.error_message
            << "\n  auto_fill_light_enabled=" << info.auto_fill_light_enabled
            << std::endl;
}

class DockPairingCallbacks final : public IDataCallback,
                                   public IControlCallback {
 public:
  void OnDockScanConfig(bool on) override {
    std::cout << "[1008 ACK] Dock scan " << (on ? "enabled" : "disabled")
              << std::endl;
  }

  void OnDockScanResult(const DockScanResult& result) override {
    {
      std::lock_guard<std::mutex> lock(mutex_);
      latest_scan_ = result;
    }
    // std::cout << "[1018 PUSH] code=" << result.code
    //           << ", scan_active=" << std::boolalpha << result.scan_active
    //           << ", devices=" << result.devices.size() << std::endl;
  }

  void OnDockConnectionAck(const DockConnectionAck& ack) override {
    std::cout << "[1022 ACK] code=" << ack.code
              << ", accepted=" << std::boolalpha << ack.accepted
              << ", error_code=" << ack.error_code
              << ", error_message=" << ack.error_message << std::endl;
  }

  void OnDockConnectionResult(const DockConnectionResult& result) override {
    std::cout << "[1024 PUSH] code=" << result.code
              << ", success=" << std::boolalpha << result.success
              << ", error_code=" << result.error_code << std::endl;
  }

  void OnDockInfo(const DockInfo& info) override { PrintDockInfo(info); }

  std::optional<DockScanResult> LatestScan() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return latest_scan_;
  }

 private:
  mutable std::mutex mutex_;
  std::optional<DockScanResult> latest_scan_;
};

void PrintScanResult(const std::optional<DockScanResult>& result) {
  if (!result) {
    std::cout << "[SCAN] No 1018 scan result has been received." << std::endl;
    return;
  }

  std::cout << "\n========== Discovered Docks ==========" << std::endl;
  std::cout << "code=" << result->code << ", accepted=" << std::boolalpha
            << result->accepted << ", scan_active=" << result->scan_active
            << ", updated_at_ms=" << result->updated_at_ms << std::endl;
  if (result->devices.empty()) {
    std::cout << "  No devices." << std::endl;
  }
  for (std::size_t i = 0; i < result->devices.size(); ++i) {
    const auto& dock = result->devices[i];
    std::cout << "  [" << i << "] name=" << dock.name
              << ", address=" << dock.address << ", rssi=" << dock.rssi
              << ", connectable=" << std::boolalpha << dock.connectable
              << std::endl;
  }
  std::cout << "======================================" << std::endl;
}

void PrintHelp(const std::string& selected_address) {
  std::cout << "\n========== Dock Pairing Commands ==========" << std::endl;
  std::cout << "  1  Enable dock scan       (1008 target=302)" << std::endl;
  std::cout << "  2  Disable dock scan      (1008 target=302)" << std::endl;
  std::cout << "  3  Print latest scan list (1018 cache)" << std::endl;
  std::cout << "  4  Select dock by index" << std::endl;
  std::cout << "  5  Connect selected dock  (1022 -> 1024)" << std::endl;
  std::cout << "  6  Query dock information (1023)" << std::endl;
  std::cout << "  7  Disconnect dock        (1022 -> 1024)" << std::endl;
  std::cout << "  h  Show this help" << std::endl;
  std::cout << "  q  Quit" << std::endl;
  std::cout << "Selected dock: "
            << (selected_address.empty() ? "<none>" : selected_address)
            << std::endl;
  std::cout << "Recommended order: 1 -> 3 -> 4 -> 2 -> 5 -> 6" << std::endl;
  std::cout << "===========================================\n" << std::endl;
}

void PrintSendResult(const char* operation, const std::error_code& ec) {
  if (ec) {
    std::cerr << "[SEND] " << operation << " failed: " << ec.message()
              << std::endl;
  } else {
    std::cout << "[SEND] " << operation
              << " written; wait for the corresponding callback." << std::endl;
  }
}

std::optional<std::string> SelectDockByIndex(
    const std::shared_ptr<DockPairingCallbacks>& callbacks) {
  const auto result = callbacks->LatestScan();
  PrintScanResult(result);
  if (!result || result->devices.empty()) {
    return std::nullopt;
  }

  std::cout << "Enter a connectable dock index: ";
  std::string input;
  if (!std::getline(std::cin, input)) {
    return std::nullopt;
  }
  try {
    const auto index = static_cast<std::size_t>(std::stoul(input));
    if (index >= result->devices.size()) {
      std::cerr << "[SELECT] Index out of range." << std::endl;
      return std::nullopt;
    }
    const auto& device = result->devices[index];
    if (!device.connectable || device.address.empty()) {
      std::cerr << "[SELECT] The selected device is not connectable."
                << std::endl;
      return std::nullopt;
    }
    return device.address;
  } catch (const std::exception&) {
    std::cerr << "[SELECT] Invalid index." << std::endl;
    return std::nullopt;
  }
}

}  // namespace

int main(int argc, char* argv[]) {
  if (argc < 3 || argc > 4) {
    std::cerr << "Usage: " << argv[0] << " <ip> <port> [dock_mac]\n"
              << "Example: " << argv[0]
              << " 192.168.234.1 8082 38:44:BE:29:C6:F6" << std::endl;
    return EXIT_FAILURE;
  }

  std::signal(SIGINT, SignalHandler);
  std::signal(SIGTERM, SignalHandler);

  SDKClient client([](const std::error_code& ec) {
    std::cerr << "SDK communication error: " << ec.message() << std::endl;
  });
  auto callbacks = std::make_shared<DockPairingCallbacks>();
  client.SetDataCallback(callbacks);
  client.SetControlCallback(callbacks);

  std::cout << "Connecting to " << argv[1] << ':' << argv[2] << "..."
            << std::endl;
  auto ec = client.Connect(argv[1], argv[2], true);
  if (ec) {
    std::cerr << "Connection failed: " << ec.message() << std::endl;
    return EXIT_FAILURE;
  }

  std::string selected_address = argc == 4 ? argv[3] : "";
  PrintHelp(selected_address);

  while (g_running && client.IsConnected()) {
    std::cout << "dock> " << std::flush;
    std::string command;
    if (!std::getline(std::cin, command)) {
      break;
    }
    if (command.empty()) {
      continue;
    }

    switch (command.front()) {
      case '1':
        PrintSendResult("enable scan", client.SetDockScanConfig(true));
        break;
      case '2':
        PrintSendResult("disable scan", client.SetDockScanConfig(false));
        break;
      case '3':
        PrintScanResult(callbacks->LatestScan());
        break;
      case '4': {
        const auto address = SelectDockByIndex(callbacks);
        if (address) {
          selected_address = *address;
          std::cout << "[SELECT] Selected dock: " << selected_address
                    << std::endl;
        }
      } break;
      case '5': {
        if (selected_address.empty()) {
          std::cerr << "[CONNECT] Select a dock first with command 4, or pass "
                       "dock_mac on the command line."
                    << std::endl;
          break;
        }
        DockConnectConfig config;
        config.address = selected_address;
        PrintSendResult("connect dock", client.ConnectDock(config));
      } break;
      case '6':
        PrintSendResult("query dock info", client.GetDockInfo());
        break;
      case '7':
        PrintSendResult("disconnect dock", client.DisconnectDock());
        break;
      case 'h':
      case 'H':
        PrintHelp(selected_address);
        break;
      case 'q':
      case 'Q':
        g_running = false;
        break;
      default:
        std::cout << "[WARN] Unknown command. Press h for help." << std::endl;
        break;
    }
  }

  std::cout << "[SHUTDOWN] Disabling scan and disconnecting SDK..."
            << std::endl;
  if (client.IsConnected()) {
    client.SetDockScanConfig(false);
    client.Disconnect(true);
  }
  return EXIT_SUCCESS;
}
