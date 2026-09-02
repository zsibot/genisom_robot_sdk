#pragma once

#include <system_error>

#include "robot_sdk/sdk_export.hpp"

namespace robot_sdk {

/// @brief SDK-specific error codes.
enum class Errc {
  Success = 0,
  ShakeHandFailed = 10000,
  ProtocolMismatch = 10001,
  ControlledDenial = 10002,
  ConnectFailed = 10003,
  UnsupportedDeviceOperation = 10004,
  UnknownError = 19999
};

ROBOT_EXPORT_API const std::error_category& robot_category();

ROBOT_EXPORT_API std::error_code make_error_code(Errc e);

}  // namespace robot_sdk

namespace std {
template <>
struct is_error_code_enum<robot_sdk::Errc> : true_type {};
}  // namespace std
