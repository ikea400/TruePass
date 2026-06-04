#pragma once
#include <cstdint>
#include <expected>
#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <span>

#include "TpmProvider.h"

enum class DeviceBindingError {
  None,
  HardwareUnavailable,
  PlatformError,
  BindingFailed,
  UnbindingFailed
};

class DeviceBinder;
using DeviceBinderPtr = std::unique_ptr<DeviceBinder>;

class DeviceBinder {
 public:
  static std::expected<DeviceBinderPtr, DeviceBindingError> create(
      const std::string& identifier);

  explicit DeviceBinder(std::wstring deviceKeyName);

  // Move-only as it manages hardware resources/handles
  DeviceBinder(const DeviceBinder&) = delete;
  DeviceBinder& operator=(const DeviceBinder&) = delete;
  DeviceBinder(DeviceBinder&&) noexcept = default;
  DeviceBinder& operator=(DeviceBinder&&) noexcept = default;

  std::expected<std::vector<uint8_t>, DeviceBindingError> getPublicKey() const;

  std::expected<std::vector<uint8_t>, DeviceBindingError> signChallenge(std::span<const uint8_t> challenge);

 private:
  const std::wstring m_deviceKeyName;
  const TpmProvider m_tpmProvider;

  static inline constexpr std::string_view kDeviceBindingKey =
      "TruePass_v1_DeviceKey";
};