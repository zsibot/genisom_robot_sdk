/**
 * @file quick_start.cpp
 * @brief Minimal read-only Robot SDK connectivity example.
 */

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <thread>

#include "robot_sdk/sdk_client.hpp"

namespace {

class DataCallback final : public robot_sdk::IDataCallback {
 public:
  void OnRobotStateData(const robot_sdk::RobotState&) override {
    if (!received_state_.exchange(true)) {
      std::cout << "[OK] First robot state received" << std::endl;
    }
  }

  void OnFaultData(const robot_sdk::FaultDatas& faults) override {
    if (!faults.empty()) {
      std::cout << "[WARN] Active faults: " << faults.size() << std::endl;
    }
  }

  bool ReceivedState() const { return received_state_.load(); }

 private:
  std::atomic<bool> received_state_{false};
};

}  // namespace

int main(int argc, char* argv[]) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <robot_ip> <port>\n"
              << "Example: " << argv[0] << " 192.168.234.1 8082"
              << std::endl;
    return EXIT_FAILURE;
  }

  robot_sdk::SDKClient client([](const std::error_code& ec) {
    std::cerr << "[SDK ERROR] " << ec.value() << ": " << ec.message()
              << std::endl;
  });

  auto data_callback = std::make_shared<DataCallback>();
  client.SetDataCallback(data_callback);

  const auto connect_ec = client.Connect(argv[1], argv[2], true);
  if (connect_ec) {
    std::cerr << "[FAILED] Connect: " << connect_ec.value() << ": "
              << connect_ec.message() << std::endl;
    return EXIT_FAILURE;
  }

  const auto device = client.GetDeviceInfo();
  std::cout << "[OK] Connected\n"
            << "  SDK version: " << client.Version() << '\n'
            << "  Protocol version: " << client.ProtocolVersion() << '\n'
            << "  Robot system version: " << client.SystemVersion() << '\n'
            << "  Device: " << robot_sdk::DeviceTypeName(device.device_type)
            << '\n'
            << "  Serial number: " << device.sn << std::endl;

  const auto deadline =
      std::chrono::steady_clock::now() + std::chrono::seconds(5);
  while (!data_callback->ReceivedState() &&
         std::chrono::steady_clock::now() < deadline) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  if (!data_callback->ReceivedState()) {
    std::cerr << "[WARN] Connected, but no robot state was received in 5 seconds"
              << std::endl;
  }

  const auto disconnect_ec = client.Disconnect(true);
  if (disconnect_ec) {
    std::cerr << "[FAILED] Disconnect: " << disconnect_ec.value() << ": "
              << disconnect_ec.message() << std::endl;
    return EXIT_FAILURE;
  }

  std::cout << "[OK] Disconnected cleanly" << std::endl;
  return data_callback->ReceivedState() ? EXIT_SUCCESS : EXIT_FAILURE;
}
