#pragma once
#ifdef _WIN32

#include <crypto/hash_algo.h>

#include <cstdint>
#include <expected>
#include <span>
#include <string>
#include <vector>

class WinTpmProvider {
 public:
  enum class Error {
    None,
    HardwareUnavailable,
    KeyNotFound,
    InvalidChallenge,
    PlatformError,
    SigningFailed
  };

  WinTpmProvider();
  ~WinTpmProvider();

  // Prevent accidental copies of hardware handles
  WinTpmProvider(const WinTpmProvider&) = delete;
  WinTpmProvider& operator=(const WinTpmProvider&) = delete;

  bool isAvailable() const noexcept { return m_provider != NULL; }

  /**
   * @brief Gets existing public key or generates a new nistP256 hardware-bound
   * key.
   */
  [[nodiscard]] std::expected<std::vector<uint8_t>, Error> getOrCreatePublicKey(
      const std::wstring& keyName) const;

  /**
   * @brief Signs data using the hardware-protected private key.
   */
  [[nodiscard]] std::expected<std::vector<uint8_t>, Error> sign(
      const std::wstring& keyName, std::span<const uint8_t> data) const;

  /**
   * @brief Securely removes the key from the TPM's persistent storage.
   */
  bool destroyKey(const std::wstring& keyName) const;

 private:
  using ProviderHandle = std::uintptr_t;
  ProviderHandle m_provider = NULL;
};
#endif