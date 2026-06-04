#pragma once
#include <cstdint>
#include <stdexcept>

#include "../utils/utils.h"

#define USE_OPENSSL_ARGON2 true

namespace ikea400::crypto {
namespace argon2 {
enum class Variant {
  Argon2d,
  Argon2i,
  Argon2id,
};

namespace detail {
bool derive(Variant variant, const void* password, size_t passwordLen,
            const void* salt, size_t saltLen, uint32_t tCost, uint32_t mCost,
            uint32_t parallelism, uint8_t* out, size_t outLen) noexcept;

constexpr inline const size_t kMinOutputSize = 16;
constexpr inline const size_t kMaxOutputSize = 128;

}  // namespace detail

template <bool ThrowOnFail = true>
inline bool derive(Variant variant, const void* password, size_t passwordLen,
                   const void* salt, size_t saltLen, uint32_t tCost,
                   uint32_t mCost, uint32_t parallelism, uint8_t* out,
                   size_t outLen) noexcept(!ThrowOnFail) {
  bool success = detail::derive(variant, password, passwordLen, salt, saltLen,
                                tCost, mCost, parallelism, out, outLen);
  if constexpr (ThrowOnFail)
    if (!success) throw std::runtime_error("Argon2 key derivation failed.");
  return success;
}

template <bool ThrowOnFail = true, utils::RangeType P, utils::RangeType S,
          utils::RangeType O>
inline bool derive(Variant variant, const P& password, const S& salt,
                   uint32_t tCost, uint32_t mCost, uint32_t parallelism,
                   O& out) noexcept(!ThrowOnFail) {
  return derive<ThrowOnFail>(
      variant, password.data(), password.size() * sizeof(P::value_type),
      salt.data(), salt.size() * sizeof(S::value_type), tCost, mCost,
      parallelism, out.data(), out.size() * sizeof(O::value_type));
}

}  // namespace argon2
}  // namespace ikea400::crypto