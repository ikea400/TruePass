#include "EnvelopeCodec.h"

#include <crypto/cipher.h>
#include <crypto/kmac.h>
#include <utils/SecureArray.h>
#include <utils/utils.h>

#include <array>
#include <cstdint>
#include <optional>
#include <span>
#include <vector>

#include "BufferReader.h"
#include "BufferWriter.h"

#include <utils/CryptoRandomizer.h>

using namespace ikea400;
using namespace crypto;

std::vector<uint8_t> EnvelopeCodec::encode(std::span<const uint8_t> data,
                                           std::span<const uint8_t> key,
                                           Kdf kdf) {
  std::array<uint8_t, AuthCipher::getIvLen()> nonce{};
  CryptoRandomizer randomizer;
  randomizer.bytes(nonce);

  AuthCryptor cipher;
  if (!cipher.init<false>(AuthCipherAlgo::AES_256_GCM, key, nonce)) return {};

  std::array<uint8_t, AuthCipher::getTagLen()> tag{};
  std::vector<uint8_t> encryptedData(data.size());
  if (cipher.encrypt(data, encryptedData, tag) <= 0) return {};

  std::vector<uint8_t> buffer;
  buffer.reserve(3 * sizeof(uint8_t) + AuthCipher::getTagLen() +
                 AuthCipher::getIvLen() + encryptedData.size());

  BufferWriter writer(buffer);

  writer.write(static_cast<uint8_t>(Version::V1));
  writer.write(static_cast<uint8_t>(kdf));
  writer.write(static_cast<uint8_t>(Cipher::AES_256_GCM));

  writer.write(nonce);
  writer.write(tag);
  writer.write(encryptedData);

  return buffer;
}

std::vector<uint8_t> EnvelopeCodec::encode(std::span<const uint8_t> data,
                                           std::span<const uint8_t> key,
                                           std::span<const uint8_t> kdfCustom,
                                           std::span<const uint8_t> kdfData) {
  SecureArray<uint8_t, 32> derivedKey;
  if (kmac::derive<false>(kmac::Variant::KMAC_256, key, kdfCustom, kdfData,
                          derivedKey) <= 0)
    return {};
  return encode(data, derivedKey, Kdf::KMAC256);
}

std::vector<uint8_t> EnvelopeCodec::decode(
    std::span<const uint8_t> envelopeData, std::span<const uint8_t> key,
    Kdf kdf) {
  auto envelopeRefOpt = parseEnvelope(envelopeData);
  if (!envelopeRefOpt) return {};

  const EnvelopeRef& envelopeRef = *envelopeRefOpt;
  if (envelopeRef.header.version != Version::V1) return {};
  if (envelopeRef.header.kdf != kdf) return {};

  return decryptData(envelopeRef, key);
}

std::vector<uint8_t> EnvelopeCodec::decode(
    std::span<const uint8_t> envelopeData, std::span<const uint8_t> key,
    std::span<const uint8_t> kdfCustom, std::span<const uint8_t> kdfData) {
  auto envelopeRefOpt = parseEnvelope(envelopeData);
  if (!envelopeRefOpt) return {};

  if (envelopeRefOpt->header.version != Version::V1) return {};
  if (envelopeRefOpt->header.kdf != Kdf::KMAC256) return {};
  std::array<uint8_t, 32> derivedKey;
  if (kmac::derive<false>(kmac::Variant::KMAC_256, key, kdfCustom, kdfData,
                          derivedKey) <= 0)
    return {};

  return decryptData(*envelopeRefOpt, derivedKey);
}

std::optional<EnvelopeCodec::EnvelopeRef> EnvelopeCodec::parseEnvelope(
    std::span<const uint8_t> envelopeData) {
  try {
    BufferReader reader(envelopeData);
    return EnvelopeRef{
        .header = parseHeader(reader).value(),
        .data = parseData(reader).value(),
    };
  } catch (...) {
    return std::nullopt;
  }
}

std::optional<EnvelopeCodec::EnvelopeHeader> EnvelopeCodec::parseHeader(
    BufferReader& reader) {
  try {
    return EnvelopeHeader{
        .version = static_cast<Version>(reader.read<uint8_t>()),
        .kdf = static_cast<Kdf>(reader.read<uint8_t>()),
        .cipher = static_cast<Cipher>(reader.read<uint8_t>()),
    };
  } catch (...) {
    return std::nullopt;
  }
}

std::optional<EnvelopeCodec::EnvelopeDataRef> EnvelopeCodec::parseData(
    BufferReader& reader) {
  try {
    return EnvelopeDataRef{
        .nonce = reader.readBytes(AuthCipher::getIvLen()),
        .tag = reader.readBytes(AuthCipher::getTagLen()),
        .encryptedData = reader.readBytes(reader.remainingSize()),
    };
  } catch (...) {
    return std::nullopt;
  }
}

std::vector<uint8_t> EnvelopeCodec::decryptData(const EnvelopeRef& envelopeRef,
                                                std::span<const uint8_t> key) {
  AuthCipherAlgo algo = AuthCipherAlgo::None;
  switch (envelopeRef.header.cipher) {
    case Cipher::AES_256_GCM: {
      algo = AuthCipherAlgo::AES_256_GCM;
    } break;
    case Cipher::CHACHA20_POLY1305: {
      algo = AuthCipherAlgo::CHACHA20_POLY1305;
    } break;
    default:
      break;
  }

  if (algo == AuthCipherAlgo::None) return {};

  AuthDecryptor decryptor;
  if (!decryptor.init<false>(algo, key, envelopeRef.data.nonce,
                             envelopeRef.data.tag))
    return {};
  std::vector<uint8_t> decryptedData(envelopeRef.data.encryptedData.size());
  size_t decryptedLen =
      decryptor.decrypt(envelopeRef.data.encryptedData, decryptedData);
  if (decryptedLen <= 0) return {};
  decryptedData.resize(decryptedLen);
  return decryptedData;
}
