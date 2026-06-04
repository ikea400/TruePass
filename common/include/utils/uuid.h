#pragma once
#include <array>
#include <glaze/glaze.hpp>
#include <string>

#include "../../include/utils/CryptoRandomizer.h"
#include "../../include/utils/utils.h"

namespace ikea400 {
class uuid {
 public:
  static constexpr std::array<uint8_t, 16> NULL_UUID = {0};

  inline uuid() noexcept : m_data(NULL_UUID) {}

  inline explicit uuid(const std::array<uint8_t, 16>& data) noexcept
      : m_data(data) {
    if (!validV4(m_data)) {
      m_data = NULL_UUID;
    }
  }

  inline explicit uuid(std::array<uint8_t, 16>&& data) noexcept
      : m_data(std::move(data)) {
    if (!validV4(m_data)) {
      m_data = NULL_UUID;
    }
  }

  static inline uuid rand() {
    uuid uuid;
    uuid.generate();
    return uuid;
  }

  static inline uuid null() noexcept { return uuid(NULL_UUID); }

  inline bool isNull() const noexcept { return m_data == NULL_UUID; }

  inline const std::array<uint8_t, 16>& bytes() const noexcept {
    return m_data;
  }

  template <bool Uppercase = false>
  inline std::string toString() const noexcept {
    if (isNull()) return "00000000-0000-0000-0000-000000000000";

    static constexpr const char hex[17] =
        Uppercase ? "0123456789ABCDEF" : "0123456789abcdef";
    std::string out(36, '0');

    size_t j = 0;
    for (size_t i = 0; i < 16; ++i) {
      out[j++] = hex[(m_data[i] >> 4) & 0xF];
      out[j++] = hex[m_data[i] & 0xF];

      if (j == 8 || j == 13 || j == 18 || j == 23) out[j++] = '-';
    }
    return out;
  }

  template <bool Throw = true>
  static inline uuid fromString(std::string_view s) noexcept(!Throw) {
    if (s.size() != 36 || s[8] != '-' || s[13] != '-' || s[18] != '-' ||
        s[23] != '-') {
      if constexpr (Throw) throw std::exception("Invalid UUID string");
      return null();
    }

    std::array<uint8_t, 16> data{};
    size_t j = 0;

    for (size_t i = 0; i < 36; ++i) {
      if (s[i] == '-') continue;

      int hi = hexVal(s[i]);
      int lo = hexVal(s[i + 1]);

      if (hi < 0 || lo < 0) {
        if constexpr (Throw)
          throw std::exception("Invalid character in UUID string");
        return null();
      }

      data[j++] = static_cast<uint8_t>((hi << 4) | lo);
      ++i;
    }

    if (!validV4(data)) {
      if constexpr (Throw)
        throw std::exception("Invalid UUID version or variant");
      return null();
    }

    return uuid(data);
  }

  inline bool operator==(const uuid&) const noexcept = default;

 private:
  inline void generate() {
    CryptoRandomizer randomizer;
    randomizer.bytes(m_data);

    // Set version 4 (0100xxxx)
    m_data[6] = (m_data[6] & 0x0F) | 0x40;

    // Set RFC 4122 variant (10xxxxxx)
    m_data[8] = (m_data[8] & 0x3F) | 0x80;
  }

  static inline constexpr bool validV4(
      const std::array<uint8_t, 16>& d) noexcept {
    if (d == NULL_UUID) return true;
    return (d[6] & 0xF0) == 0x40 && (d[8] & 0xC0) == 0x80;
  }

  static inline constexpr int hexVal(char c) noexcept {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;  // invalid hex
  }

 private:
  std::array<uint8_t, 16> m_data{};
};

static_assert(sizeof(uuid) == 16, "Must be exactly 16 bytes.");
static_assert(std::is_standard_layout_v<uuid>, "Must be safe for FFI/Rust.");
static_assert(std::is_trivially_copyable_v<uuid>,
              "Must be SIMD/Register optimized.");
static_assert(std::is_trivially_destructible_v<uuid>,
              "Must have zero-cost destruction.");

static_assert(std::is_nothrow_default_constructible_v<uuid>);
static_assert(std::is_nothrow_copy_constructible_v<uuid>);
static_assert(std::is_nothrow_move_constructible_v<uuid>);

static_assert(!std::is_polymorphic_v<uuid>,
              "UUID must not have virtual functions.");

}  // namespace ikea400

namespace std {
template <>
struct hash<ikea400::uuid> {
  size_t operator()(const ikea400::uuid& u) const {
    // We use the standard hash for std::string
    return std::hash<std::string_view>{}(
        {reinterpret_cast<const char*>(u.bytes().data()), u.bytes().size()});
  }
};
}  // namespace std

namespace glz {

template <>
struct from<JSON, ikea400::uuid> {
  template <auto Opts>
  static void op(ikea400::uuid& value, is_context auto&& ctx, auto&& it,
                 auto&& end) {
    // Parse the JSON string first
    std::string encoded;
    parse<JSON>::op<Opts>(encoded, ctx, it, end);

    // Stop if parsing already failed
    if (ctx.error != glz::error_code::none) return;

    value = ikea400::uuid::fromString<false>(encoded);
    if (value.isNull()) {
      ctx.error = glz::error_code::parse_error;
    }
  }
};

template <>
struct to<JSON, ikea400::uuid> {
  template <auto Opts>
  static void op(const ikea400::uuid& value, is_context auto&& ctx, auto&& b,
                 auto&& ix) noexcept {
    serialize<JSON>::op<Opts>(value.toString(), ctx, b, ix);
  }
};
}  // namespace glz