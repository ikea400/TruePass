#ifdef _WIN32
#include "WinTpmProvider.h"

#include <Windows.h>
#include <crypto/hash_algo.h>
#include <crypto/hasher.h>
#include <utils/SecureArray.h>
#include <utils/utils.h>

#include <cstdint>
#include <expected>
#include <span>
#include <string>
#include <type_traits>
#include <vector>

using namespace ikea400;
using namespace ikea400::crypto;

class ScopedHandle {
 public:
  explicit ScopedHandle(NCRYPT_KEY_HANDLE handle) : m_handle(handle) {}
  ~ScopedHandle() {
    if (m_handle) {
      NCryptFreeObject(m_handle);
    }
  }

  ScopedHandle& operator=(const ScopedHandle&) = delete;
  ScopedHandle(const ScopedHandle&) = delete;

  NCRYPT_KEY_HANDLE get() const { return m_handle; }
  void reset(NCRYPT_KEY_HANDLE newHandle = NULL) {
    if (m_handle) {
      NCryptFreeObject(m_handle);
    }
    m_handle = newHandle;
  }

 private:
  NCRYPT_KEY_HANDLE m_handle;
};

static std::vector<uint8_t> exportKeyToDER(NCRYPT_KEY_HANDLE hKey) {
  if (!hKey) return {};

  // Step 1: Export the structural information from the native NCRYPT handle
  DWORD infoSize = 0;
  if (!CryptExportPublicKeyInfoEx(hKey, 0, X509_ASN_ENCODING, nullptr, 0,
                                  nullptr, nullptr, &infoSize)) {
    return {};
  }

  std::vector<BYTE> infoBuffer(infoSize);
  auto publicKeyInfo =
      reinterpret_cast<PCERT_PUBLIC_KEY_INFO>(infoBuffer.data());
  if (!CryptExportPublicKeyInfoEx(hKey, 0, X509_ASN_ENCODING, nullptr, 0,
                                  nullptr, publicKeyInfo, &infoSize)) {
    return {};
  }

  // Step 2: Encode the CERT_PUBLIC_KEY_INFO to DER format
  DWORD derSize = 0;
  if (!CryptEncodeObjectEx(X509_ASN_ENCODING, X509_PUBLIC_KEY_INFO,
                           publicKeyInfo, 0, nullptr, nullptr, &derSize)) {
    return {};
  }

  std::vector<BYTE> derBlob(derSize);
  if (!CryptEncodeObjectEx(X509_ASN_ENCODING, X509_PUBLIC_KEY_INFO,
                           publicKeyInfo, 0, nullptr, derBlob.data(),
                           &derSize)) {
    return {};
  }

  return derBlob;
}

static std::expected<std::vector<uint8_t>, WinTpmProvider::Error>
exportSignatureToDER(std::span<const uint8_t> signature) {
  if (signature.empty() || (signature.size() % 2 != 0)) {
    return std::unexpected(WinTpmProvider::Error::PlatformError);
  }

  // Win signatures for ECDSA are typically raw concatenations of r and s
  
  // 1. Slice out the raw blocks for r and s
  const size_t halfSize = signature.size() / 2;
  std::span<const uint8_t> rSub = signature.subspan(0, halfSize);
  std::span<const uint8_t> sSub = signature.subspan(halfSize);

  // Helper lambda to trim leading zeros according to ASN.1 rules
  auto trimLeadingZeros =
      [](std::span<const uint8_t> span) -> std::span<const uint8_t> {
    while (span.size() > 1 && span.front() == 0x00) {
      span = span.subspan(1);
    }
    return span;
  };

  rSub = trimLeadingZeros(rSub);
  sSub = trimLeadingZeros(sSub);

  // 2. Determine if ASN.1 positive-integer padding (0x00) is required
  // (If MSB is 1, prepend 0x00 to prevent it from being interpreted as
  // negative)
  const bool rPad = (!rSub.empty() && (rSub.front() & 0x80) != 0);
  const bool sPad = (!sSub.empty() && (sSub.front() & 0x80) != 0);

  // 3. Calculate exact DER lengths safely
  const size_t rDerLen = rSub.size() + (rPad ? 1 : 0);
  const size_t sDerLen = sSub.size() + (sPad ? 1 : 0);


  // Each identifier block uses: 1 byte (Tag) + 1 byte (Len) + Content Length
  const size_t totalInnerLen = (2 + rDerLen) + (2 + sDerLen);

  // DER short-form length limit check (Length must fit in 7 bits, i.e., < 128)
  if (totalInnerLen > 127) {
    return std::unexpected(WinTpmProvider::Error::PlatformError);
  }

  // 5. Construct the final DER byte array sequence
  std::vector<uint8_t> der;
  der.reserve(2uz + totalInnerLen);

 // Sequence Header: Tag (0x30) + Total Length
  der.push_back(0x30);
  der.push_back(static_cast<uint8_t>(totalInnerLen));

  // Append R block: Tag (0x02) + Length + [Padding] + Data
  der.push_back(0x02);
  der.push_back(static_cast<uint8_t>(rDerLen));
  if (rPad) der.push_back(0x00);
  der.insert(der.end(), rSub.begin(), rSub.end());

  // Append S block: Tag (0x02) + Length + [Padding] + Data
  der.push_back(0x02);
  der.push_back(static_cast<uint8_t>(sDerLen));
  if (sPad) der.push_back(0x00);
  der.insert(der.end(), sSub.begin(), sSub.end());

  return der;
}

WinTpmProvider::WinTpmProvider() {
  static_assert(std::is_same_v<ProviderHandle, NCRYPT_PROV_HANDLE>,
                "ProviderHandle type must match NCRYPT_PROV_HANDLE");

  SECURITY_STATUS status = NCryptOpenStorageProvider(
      &m_provider, MS_PLATFORM_KEY_STORAGE_PROVIDER, 0);

  if (status != ERROR_SUCCESS) {
    m_provider = NULL;
  }
}

WinTpmProvider::~WinTpmProvider() {
  if (m_provider) {
    NCryptFreeObject(m_provider);
  }
}

std::expected<std::vector<uint8_t>, WinTpmProvider::Error>
WinTpmProvider::getOrCreatePublicKey(const std::wstring& keyName) const {
  if (!m_provider) {
    return std::unexpected(Error::HardwareUnavailable);
  }

  NCRYPT_KEY_HANDLE hKeyRaw = NULL;

  // 1. Try to open existing key
  SECURITY_STATUS status =
      NCryptOpenKey(m_provider, &hKeyRaw, keyName.c_str(), 0, 0);
  ScopedHandle hKey(hKeyRaw);

  if (status == NTE_BAD_KEYSET) {
    // 2. Key doesn't exist, create it
    status = NCryptCreatePersistedKey(m_provider, &hKeyRaw,
                                      BCRYPT_ECDSA_P256_ALGORITHM,
                                      keyName.c_str(), 0, 0);
    hKey.reset(hKeyRaw);

    if (status != ERROR_SUCCESS) return std::unexpected(Error::PlatformError);

    // Finalize creates the key physically in the TPM
    status = NCryptFinalizeKey(hKey.get(), 0);
    if (status != ERROR_SUCCESS) return std::unexpected(Error::PlatformError);
  } else if (status != ERROR_SUCCESS) {
    return std::unexpected(Error::PlatformError);
  }

  // 3. Export the public key in DER format
  std::vector<uint8_t> pubKey = exportKeyToDER(hKey.get());
  if (pubKey.empty()) return std::unexpected(Error::PlatformError);

  return pubKey;
}

std::expected<std::vector<uint8_t>, WinTpmProvider::Error> WinTpmProvider::sign(
    const std::wstring& keyName, std::span<const uint8_t> data) const {
  if (!m_provider) return std::unexpected(Error::HardwareUnavailable);

  NCRYPT_KEY_HANDLE hKeyRaw = NULL;
  SECURITY_STATUS status =
      NCryptOpenKey(m_provider, &hKeyRaw, keyName.c_str(), 0, 0);
  ScopedHandle hKey(hKeyRaw);

  if (status != ERROR_SUCCESS) return std::unexpected(Error::KeyNotFound);

  SecureArray<uint8_t, SHA256_HASH_SIZE> md;
  if (!Hasher::hash<false>(HashAlgo::Sha256, data, md))
    return std::unexpected(Error::PlatformError);

  constexpr DWORD mdLen = SHA256_HASH_SIZE;

  // Get signature size
  DWORD cbSignature = 0;
  status = NCryptSignHash(hKey.get(), NULL, const_cast<PBYTE>(md.data()), mdLen,
                          NULL, 0, &cbSignature, 0);

  if (status != ERROR_SUCCESS) return std::unexpected(Error::PlatformError);

  std::vector<uint8_t> signature(cbSignature);
  status = NCryptSignHash(hKey.get(), NULL, const_cast<PBYTE>(md.data()), mdLen,
                          signature.data(), (DWORD)signature.size(),
                          &cbSignature, 0);

  if (status != ERROR_SUCCESS) return std::unexpected(Error::SigningFailed);
  return exportSignatureToDER(signature);
}

bool WinTpmProvider::destroyKey(const std::wstring& keyName) const {
  if (!m_provider) return false;

  NCRYPT_KEY_HANDLE hKey = NULL;
  if (NCryptOpenKey(m_provider, &hKey, keyName.c_str(), 0, 0) ==
      ERROR_SUCCESS) {
    // NCryptDeleteKey also frees the handle
    return NCryptDeleteKey(hKey, 0) == ERROR_SUCCESS;
  }
  return false;
}
#endif