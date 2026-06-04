#pragma once
#include <openssl/evp.h>

#include <memory>
#include <stdexcept>

#include "../utils/utils.h"
#include "hash_algo.h"

namespace ikea400::crypto {
namespace hmac {
class State {
 public:
  State(const State& other) = delete;
  State& operator=(const State& other) = delete;

  inline State() = default;
  inline State(State&& other) = default;
  inline State& operator=(State&& other) = default;

  template <bool ThrowOnFail = true>
  bool init(HashAlgo algo, const uint8_t* key,
            size_t keyLen) noexcept(!ThrowOnFail) {
    bool success = initImpl(algo, key, keyLen);
    if constexpr (ThrowOnFail)
      if (!success) throw std::runtime_error("HMAC::Sate failed to init");
    return success;
  }

  template <bool ThrowOnFail = true, utils::RangeType K>
  bool init(HashAlgo algo, const K& key) noexcept(!ThrowOnFail) {
    return init<ThrowOnFail>(algo, key.data(),
                             key.size() * sizeof(K::value_type));
  }

  template <bool ThrowOnFail = true>
  bool update(const uint8_t* data, size_t datalen) noexcept(!ThrowOnFail) {
    bool success = updateImpl(data, datalen);
    if constexpr (ThrowOnFail)
      if (!success) throw std::runtime_error("HMAC::Sate update failed.");
    return success;
  }

  template <bool ThrowOnFail = true, utils::RangeType I>
  bool update(const I& data) noexcept(!ThrowOnFail) {
    return update<ThrowOnFail>(data.data(),
                               data.size() * sizeof(I::value_type));
  }

  template <bool ThrowOnFail = true>
  bool final(uint8_t* out, size_t outsize,
             size_t* outl) noexcept(!ThrowOnFail) {
    bool success = finalImpl(out, outl, outsize);
    if constexpr (ThrowOnFail)
      if (!success) throw std::runtime_error("HMAC::Sate Final failed.");
    return success;
  }

  template <bool ThrowOnFail = true, utils::RangeType O>
  bool final(const O& out, size_t* outl) noexcept(!ThrowOnFail) {
    return final<ThrowOnFail>(out.data(), out.size() * sizeof(O::value_type),
                              outl);
  }

  template <bool ThrowOnFail = true, utils::RangeType O>
  size_t final(const O& out) noexcept(!ThrowOnFail) {
    size_t outl;
    bool success = final<ThrowOnFail>(
        out.data(), out.size() * sizeof(O::value_type), &outl);
    return success ? outl : 0;
  }

 private:
  bool initImpl(HashAlgo algo, const uint8_t* key, size_t keyLen) noexcept;
  bool updateImpl(const uint8_t* data, size_t datalen) noexcept;
  bool finalImpl(uint8_t* out, size_t* outl, size_t outsize) noexcept;

 private:
  using EVP_MAC_CTX_ptr =
      std::unique_ptr<EVP_MAC_CTX, utils::DeleterFromFn<&::EVP_MAC_CTX_free>>;
  using EVP_MAC_ptr =
      std::unique_ptr<EVP_MAC, utils::DeleterFromFn<&::EVP_MAC_free>>;

  EVP_MAC_CTX_ptr m_ctx;
};

template <bool ThrowOnFail = true>
size_t derive(const HashAlgo digest, const uint8_t* key, size_t keyLen,
              const uint8_t* data, size_t dataLen, uint8_t* out,
              size_t outLen) noexcept(!ThrowOnFail) {
  size_t mdLen = 0;
  State hmac;
  bool success = hmac.init<ThrowOnFail>(digest, key, keyLen) &&
                 hmac.update<ThrowOnFail>(data, dataLen) &&
                 hmac.final<ThrowOnFail>(out, outLen, &mdLen);
  return success ? mdLen : 0;
}

template <bool ThrowOnFail = true, utils::RangeType K, utils::RangeType I,
          utils::RangeType O>
size_t derive(const HashAlgo digest, const K& key, const I& data,
              O& out) noexcept(!ThrowOnFail) {
  return derive<ThrowOnFail>(digest,
                             reinterpret_cast<const uint8_t*>(key.data()),
                             key.size() * sizeof(K::value_type),
                             reinterpret_cast<const uint8_t*>(data.data()),
                             data.size() * sizeof(I::value_type),
                             reinterpret_cast<uint8_t*>(out.data()),
                             out.size() * sizeof(O::value_type));
}

}  // namespace hmac
}  // namespace ikea400::crypto