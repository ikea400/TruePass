#pragma once
#include <cstdint>
#include <string_view>
#include <vector>
#include <stdexcept>

#include "../utils/utils.h"
#include "hash_algo.h"

namespace ikea400::crypto {
namespace hkdf {
namespace detail {
enum class Mode { Extract, Expand, Derive };

bool execute(const HashAlgo digest, Mode mode, const void* salt, size_t saltLen,
             const void* ikm, size_t ikmLen, const void* info, size_t infoLen,
             uint8_t* outKey, size_t outKeyLen) noexcept;

bool expandLabel(const HashAlgo digest, const void* ikm, size_t ikmLen,
                 const std::string_view label, const void* context,
                 size_t contextLen, uint8_t* outKey, size_t outKeyLen) noexcept;

std::vector<uint8_t> makeLabel(const std::string_view label,
                               const uint8_t* context, size_t contextLen,
                               size_t outLen) noexcept;
}  // namespace detail

template <bool ThrowOnFail = true>
inline bool derive(const HashAlgo digest, const void* salt, size_t saltLen,
                   const void* ikm, size_t ikmLen, const void* info,
                   size_t infoLen, uint8_t* outKey,
                   size_t outKeyLen) noexcept(!ThrowOnFail) {
  bool success = detail::execute(digest, detail::Mode::Derive, salt, saltLen,
                                 ikm, ikmLen, info, infoLen, outKey, outKeyLen);
  if constexpr (ThrowOnFail)
    if (!success) throw std::runtime_error("HKDF derive operation failed.");
  return success;
}

template <bool ThrowOnFail = true, utils::RangeType S, utils::RangeType K,
          utils::RangeType I, utils::RangeType O>
inline bool derive(const HashAlgo digest, const S& salt, const K& ikm,
                   const I& info, O& outKey) noexcept(!ThrowOnFail) {
  return derive<ThrowOnFail>(digest,
                             reinterpret_cast<const uint8_t*>(salt.data()),
                             salt.size() * sizeof(S::value_type),
                             reinterpret_cast<const uint8_t*>(ikm.data()),
                             ikm.size() * sizeof(K::value_type),
                             reinterpret_cast<const uint8_t*>(info.data()),
                             info.size() * sizeof(I::value_type),
                             reinterpret_cast<uint8_t*>(outKey.data()),
                             outKey.size() * sizeof(O::value_type));
}

template <bool ThrowOnFail = true>
inline bool expand(const HashAlgo digest, const void* ikm, size_t ikmLen,
                   const void* info, size_t infoLen, uint8_t* outKey,
                   size_t outKeyLen) noexcept(!ThrowOnFail) {
  bool success = detail::execute(digest, detail::Mode::Expand, nullptr, 0, ikm,
                                 ikmLen, info, infoLen, outKey, outKeyLen);
  if constexpr (ThrowOnFail)
    if (!success) throw std::runtime_error("HKDF expand operation failed.");
  return success;
}

template <bool ThrowOnFail = true, utils::RangeType K, utils::RangeType I,
          utils::RangeType O>
inline bool expand(const HashAlgo digest, const K& ikm, const I& info,
                   O& outKey) noexcept(!ThrowOnFail) {
  return expand<ThrowOnFail>(digest, ikm.data(),
                             ikm.size() * sizeof(K::value_type), info.data(),
                             info.size() * sizeof(I::value_type), outKey.data(),
                             outKey.size() * sizeof(O::value_type));
}

template <bool ThrowOnFail = true>
inline bool expandLabel(const HashAlgo digest, const void* ikm, size_t ikmLen,
                        const std::string_view label, const void* context,
                        size_t contextLen, uint8_t* outKey,
                        size_t outKeyLen) noexcept(!ThrowOnFail) {
  bool success = detail::expandLabel(digest, ikm, ikmLen, label, context,
                                     contextLen, outKey, outKeyLen);
  if constexpr (ThrowOnFail)
    if (!success) throw std::runtime_error("HKDF expand operation failed.");
  return success;
}

template <bool ThrowOnFail = true, utils::RangeType K, utils::RangeType C,
          utils::RangeType O>
inline bool expandLabel(const HashAlgo digest, const K& ikm,
                        const std::string_view label, const C& context,
                        O& outKey) noexcept(!ThrowOnFail) {
  bool success = detail::expandLabel(
      digest, ikm.data(), ikm.size() * sizeof(K::value_type), label,
      context.data(), context.size() * sizeof(C::value_type), outKey.data(),
      outKey.size() * sizeof(O::value_type));
  if constexpr (ThrowOnFail)
    if (!success) throw std::runtime_error("HKDF expand operation failed.");
  return success;
}

template <bool ThrowOnFail = true>
inline bool extract(const HashAlgo digest, const void* salt, size_t saltLen,
                    const void* ikm, size_t ikmLen, uint8_t* outKey,
                    size_t outKeyLen) noexcept(!ThrowOnFail) {
  bool success = detail::execute(digest, detail::Mode::Extract, salt, saltLen,
                                 ikm, ikmLen, nullptr, 0, outKey, outKeyLen);
  if constexpr (ThrowOnFail)
    if (!success) throw std::runtime_error("HKDF extract operation failed.");
  return success;
}

template <bool ThrowOnFail = true, utils::RangeType S, utils::RangeType K,
          utils::RangeType O>
inline bool extract(const HashAlgo digest, const S& salt, const K& ikm,
                    O& outKey) noexcept(!ThrowOnFail) {
  return extract<ThrowOnFail>(digest, salt.data(),
                              salt.size() * sizeof(S::value_type), ikm.data(),
                              ikm.size() * sizeof(K::value_type), outKey.data(),
                              outKey.size() * sizeof(O::value_type));
}

}  // namespace hkdf
}  // namespace ikea400::crypto