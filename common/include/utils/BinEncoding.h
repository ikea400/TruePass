#pragma once
#include <algorithm>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "../../src/utils/base64/base64.h"
#include "../../src/utils/fast-hex/hex.h"

namespace ikea400::bin {

namespace detail {
inline std::string prepareFormat(const std::string_view format,
                                 std::size_t size) {
  std::string value(format.size() + size, '\0');
  std::copy(format.begin(), format.end(), value.begin());
  return value;
}

template <typename T>
inline void encodeHex(std::span<char> dest, const T& value) {
#if defined(__AVX2__)
  ::encodeHexVec((uint8_`t*)dest.data(), (const uint8_t*)value.data(),
               value.size() * sizeof(T::value_type));
#else 
  ::encodeHex(reinterpret_cast<uint8_t*>(dest.data()),
            reinterpret_cast<const uint8_t*>(value.data()),
            value.size() * sizeof(T::value_type));
#endif
}

inline void decodeHex(std::vector<uint8_t>& dest,
                      const std::string_view value) {
#if defined(__AVX2__)
  decodeHexVec((uint8_t*)dest.data(), (const uint8_t*)value.data(),
               dest.size());
#else
  decodeHexLUT4((uint8_t*)dest.data(), (const uint8_t*)value.data(),
                dest.size());
#endif
}

}  // namespace detail

namespace hex {
inline constexpr const std::string_view kHexFormat = "hex:";

template <typename T>
inline std::string encode(const T& value) {
  using value_type = T::value_type;
  std::string output(value.size() * sizeof(value_type) * 2, '\0');
  if (!value.empty()) detail::encodeHex(std::span(output), value);
  return output;
}

template <typename T>
inline std::string format(const T& value) {
  using value_type = T::value_type;
  std::string output = detail::prepareFormat(
      kHexFormat, value.size() * sizeof(T::value_type) * 2);
  if (!value.empty())
    detail::encodeHex(std::span(output).subspan(kHexFormat.size()), value);
  return output;
}

template <bool Throw = true>
inline std::vector<uint8_t> decodeFormat(std::string_view value) {
  if (!value.starts_with(kHexFormat)) {
    if constexpr (Throw)
      throw std::exception("Hex decode format: invalid format prefix");

    return {};
  }
  return decode<Throw>(value.substr(kHexFormat.size()));
}

template <bool Throw = true>
inline std::vector<uint8_t> decode(std::string_view value) {
  if (value.empty()) return {};
  if (value.size() % 2 != 0) {
    if constexpr (Throw)
      throw std::exception("Hex decode: input size is not even");

    return {};
  }

  std::vector<uint8_t> output(value.size() / 2);

  detail::decodeHex(output, value);

  return output;
}

}  // namespace hex

namespace b64 {
inline constexpr const std::string_view kBase64Format = "b64:";

template <typename T>
inline std::string encode(const T& value) {
  return base64::encode_into<std::string>(value.begin(), value.end());
}

template <typename T>
inline std::string format(const T& value) {
  return std::string(kBase64Format) + encode(value);
}

inline std::vector<uint8_t> decodeFormat(const std::string_view value) {
  if (!value.starts_with(kBase64Format)) {
    throw std::exception("Base64 decode format: invalid format prefix");
  }
  return base64::decode_into<std::vector<uint8_t>>(
      value.substr(kBase64Format.size()));
}

template <bool Throw = true>
inline std::vector<uint8_t> decode(const std::string_view value) {
  if constexpr (Throw) {
    return base64::decode_into<std::vector<uint8_t>>(value);
  } else {
    try {
      return base64::decode_into<std::vector<uint8_t>>(value);
    } catch (...) {
      return {};
    }
  }
}
}  // namespace b64

template <bool Throw = true>
inline std::vector<uint8_t> decodeFormat(const std::string_view value) {
  if (value.starts_with(b64::kBase64Format)) {
    return b64::decode<Throw>(value.substr(b64::kBase64Format.size()));
  }
  if (value.starts_with(hex::kHexFormat)) {
    return hex::decode<Throw>(value.substr(hex::kHexFormat.size()));
  }
  if constexpr (Throw)
    throw std::exception("Decode format: unknown format prefix");
  return {};
}

}  // namespace ikea400::bin