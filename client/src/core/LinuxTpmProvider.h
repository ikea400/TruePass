#pragma once
#include <cstdint>
#include <expected>
#include <span>
#include <string>
#include <vector>

class LinuxTpmProvider {
 public:
  enum class Error {
    None,
    HardwareUnavailable,
    KeyNotFound,
    InvalidChallenge,
    PlatformError,
    SigningFailed
  };

  LinuxTpmProvider() = default;
  ~LinuxTpmProvider() = default;

  // Prevent accidental copies of hardware handles
  LinuxTpmProvider(const LinuxTpmProvider&) = delete;
  LinuxTpmProvider& operator=(const LinuxTpmProvider&) = delete;

  /**
   * @brief Gets existing public key or generates a new nistP256 hardware-bound
   * key.
   */
  [[nodiscard]] std::expected<std::vector<uint8_t>, Error> getOrCreatePublicKey(
      const std::wstring& keyName) const {
    return std::unexpected(Error::HardwareUnavailable);
  }

  /**
   * @brief Signs a challenge using the hardware-protected private key.
   */
  [[nodiscard]] std::expected<std::vector<uint8_t>, Error> signChallenge(
      const std::wstring& keyName, std::span<const uint8_t> challenge) const {
    return std::unexpected(Error::HardwareUnavailable);
  }

  /**
   * @brief Securely removes the key from the TPM's persistent storage.
   */
  bool destroyKey(const std::wstring& keyName) const { return false; }
};