#pragma once
#include <utils/utils.h>

#include <cstdint>
#include <optional>
#include <span>
#include <vector>

#include "BufferReader.h"

class EnvelopeCodec {
 public:
  enum class Version : uint8_t {
    V1 = 0x01,
  };

  enum class Kdf : uint8_t { NONE = 0x00, KMAC256 = 0x01 };

  enum class Cipher : uint8_t { AES_256_GCM = 0x01, CHACHA20_POLY1305 = 0x02 };

  struct EnvelopeHeader {
    Version version;
    Kdf kdf;
    Cipher cipher;
  };

  struct EnvelopeData {
    std::vector<uint8_t> nonce;
    std::vector<uint8_t> tag;
    std::vector<uint8_t> encryptedData;
  };

  struct EnvelopeDataRef {
    std::span<const uint8_t> nonce;
    std::span<const uint8_t> tag;
    std::span<const uint8_t> encryptedData;
  };

  struct Envelope {
    EnvelopeHeader header;
    EnvelopeData data;
  };

  struct EnvelopeRef {
    EnvelopeHeader header;
    EnvelopeDataRef data;
  };

  template <ikea400::utils::RangeType T>
  static std::vector<uint8_t> encode(const T& data,
                                     std::span<const uint8_t> key, Kdf kdf) {
    return encode(
        std::span<const uint8_t>(data.data(),
                                 data.size() * sizeof(typename T::value_type)),
        key, kdf);
  }

  template <ikea400::utils::RangeType T, ikea400::utils::RangeType C,
            ikea400::utils::RangeType D>
  static std::vector<uint8_t> encode(const T& data,
                                     std::span<const uint8_t> key,
                                     const C& kdfCustom, const D& kdfData) {
    return encode(
        std::span<const uint8_t>(data.data(),
                                 data.size() * sizeof(typename T::value_type)),
        key,
        std::span<const uint8_t>(
            reinterpret_cast<const uint8_t*>(kdfCustom.data()),
            kdfCustom.size() * sizeof(typename C::value_type)),
        std::span<const uint8_t>(
            reinterpret_cast<const uint8_t*>(kdfData.data()),
            kdfData.size() * sizeof(typename D::value_type)));
  }

  template <ikea400::utils::RangeType T>
  static std::vector<uint8_t> decode(const T& envelopeData,
                                     std::span<const uint8_t> key, Kdf kdf) {
    return decode(std::span<const uint8_t>(
                      envelopeData.data(),
                      envelopeData.size() * sizeof(typename T::value_type)),
                  key, kdf);
  }

  template <ikea400::utils::RangeType T, ikea400::utils::RangeType C,
            ikea400::utils::RangeType D>
  static std::vector<uint8_t> decode(const T& envelopeData,
                                     std::span<const uint8_t> key,
                                     const C& kdfCustom, const D& kdfData) {
    return decode(std::span<const uint8_t>(
                      envelopeData.data(),
                      envelopeData.size() * sizeof(typename T::value_type)),
                  key,
                  std::span<const uint8_t>(
                      reinterpret_cast<const uint8_t*>(kdfCustom.data()),
                      kdfCustom.size() * sizeof(typename C::value_type)),
                  std::span<const uint8_t>(
                      reinterpret_cast<const uint8_t*>(kdfData.data()),
                      kdfData.size() * sizeof(typename D::value_type)));
  }

  static std::vector<uint8_t> encode(std::span<const uint8_t> data,
                                     std::span<const uint8_t> key, Kdf kdf);
  static std::vector<uint8_t> encode(std::span<const uint8_t> data,
                                     std::span<const uint8_t> key,
                                     std::span<const uint8_t> kdfCustom,
                                     std::span<const uint8_t> kdfData);

  static std::vector<uint8_t> decode(std::span<const uint8_t> envelopeData,
                                     std::span<const uint8_t> key, Kdf kdf);
  static std::vector<uint8_t> decode(std::span<const uint8_t> envelopeData,
                                     std::span<const uint8_t> key,
                                     std::span<const uint8_t> kdfCustom,
                                     std::span<const uint8_t> kdfData);

  static std::optional<EnvelopeRef> parseEnvelope(
      std::span<const uint8_t> envelopeData);
  static std::optional<EnvelopeHeader> parseHeader(BufferReader& reader);
  static std::optional<EnvelopeDataRef> parseData(BufferReader& reader);

  static std::vector<uint8_t> decryptData(const EnvelopeRef& envelopeRef,
                                          std::span<const uint8_t> key);
};
