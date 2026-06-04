#pragma once
#include <openssl/evp.h>

#include <cstdint>
#include <memory>
#include <stdexcept>

#include "../utils/utils.h"

namespace ikea400::crypto {
namespace kmac {

enum class Variant {
  KMAC_128,
  KMAC_256,
};

class State {
 public:
  State(const State& other) = delete;
  State& operator=(const State& other) = delete;

  inline State() = default;
  inline State(State&& other) = default;
  inline State& operator=(State&& other) = default;

  template <bool ThrowOnFail = true>
  bool init(Variant variant, const uint8_t* key, size_t keyLen,
            const uint8_t* custom, size_t customLen) noexcept(!ThrowOnFail) {
    bool success = initImpl(variant, key, keyLen, custom, customLen);
    if constexpr (ThrowOnFail)
      if (!success) throw std::runtime_error("kmac::Sate failed to init");
    return success;
  }

  template <bool ThrowOnFail = true, utils::RangeType K, utils::RangeType C>
  bool init(Variant variant, const K& key,
            const C& custom) noexcept(!ThrowOnFail) {
    return init<ThrowOnFail>(variant, key.data(),
                             key.size() * sizeof(K::value_type), custom.data(),
                             custom.size() * sizeof(C::value_type));
  }

  template <bool ThrowOnFail = true>
  bool update(const uint8_t* data, size_t datalen) noexcept(!ThrowOnFail) {
    bool success = updateImpl(data, datalen);
    if constexpr (ThrowOnFail)
      if (!success) throw std::runtime_error("kmac::Sate update failed.");
    return success;
  }

  template <bool ThrowOnFail = true, utils::RangeType I>
  bool update(const I& data) noexcept(!ThrowOnFail) {
    return update<ThrowOnFail>(data.data(),
                               data.size() * sizeof(I::value_type));
  }

  template <bool ThrowOnFail = true>
  bool final(uint8_t* out, size_t outsize, size_t* outl,
             bool xofEnabled = false) noexcept(!ThrowOnFail) {
    bool success = finalImpl(out, outl, outsize, xofEnabled);
    if constexpr (ThrowOnFail)
      if (!success) throw std::runtime_error("kmac::Sate Final failed.");
    return success;
  }

  template <bool ThrowOnFail = true, utils::RangeType O>
  bool final(const O& out, size_t* outl,
             bool xofEnabled = false) noexcept(!ThrowOnFail) {
    return final<ThrowOnFail>(out.data(), out.size() * sizeof(O::value_type),
                              outl, xofEnabled);
  }

  template <bool ThrowOnFail = true, utils::RangeType O>
  size_t final(const O& out) noexcept(!ThrowOnFail) {
    size_t outl;
    bool success = final<ThrowOnFail>(
        out.data(), out.size() * sizeof(O::value_type), &outl);
    return success ? outl : 0;
  }

 private:
  bool initImpl(Variant variant, const uint8_t* key, size_t keyLen,
                const uint8_t* custom, size_t customLen) noexcept;
  bool updateImpl(const uint8_t* data, size_t datalen) noexcept;
  bool finalImpl(uint8_t* out, size_t* outl, size_t outsize,
                 bool xofEnabled) noexcept;

 private:
  using EVP_MAC_CTX_ptr =
      std::unique_ptr<EVP_MAC_CTX, utils::DeleterFromFn<&::EVP_MAC_CTX_free>>;
  using EVP_MAC_ptr =
      std::unique_ptr<EVP_MAC, utils::DeleterFromFn<&::EVP_MAC_free>>;

  EVP_MAC_CTX_ptr m_ctx;
};

template <bool ThrowOnFail = true>
size_t derive(const Variant variant, const uint8_t* key, size_t keyLen,
              const uint8_t* custom, size_t customLen, const uint8_t* data,
              size_t dataLen, uint8_t* out, size_t outLen,
              bool xofEnabled) noexcept(!ThrowOnFail) {
  size_t mdLen = 0;
  State kmac;
  bool success =
      kmac.init<ThrowOnFail>(variant, key, keyLen, custom, customLen) &&
      kmac.update<ThrowOnFail>(data, dataLen) &&
      kmac.final<ThrowOnFail>(out, outLen, &mdLen, xofEnabled);
  return success ? mdLen : 0;
}

template <bool ThrowOnFail = true, utils::RangeType K, utils::RangeType C,
          utils::RangeType I, utils::RangeType O>
size_t derive(const Variant variant, const K& key, const C& custom,
              const I& data, O& out,
              bool xofEnabled = false) noexcept(!ThrowOnFail) {
  return derive<ThrowOnFail>(variant,
                             reinterpret_cast<const uint8_t*>(key.data()),
                             key.size() * sizeof(K::value_type),
                             reinterpret_cast<const uint8_t*>(custom.data()),
                             custom.size() * sizeof(C::value_type),
                             reinterpret_cast<const uint8_t*>(data.data()),
                             data.size() * sizeof(I::value_type),
                             reinterpret_cast<uint8_t*>(out.data()),
                             out.size() * sizeof(O::value_type), xofEnabled);
}

}  // namespace kmac
}  // namespace ikea400::crypto