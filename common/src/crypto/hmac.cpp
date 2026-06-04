#include "../../include/crypto/hmac.h"

#include <openssl/core_names.h>

namespace ikea400::crypto {
namespace hmac {
bool State::initImpl(HashAlgo algo, const uint8_t* key,
                     size_t keyLen) noexcept {
  const char* cipher = getHashProviderName(algo);
  if (!cipher) return false;

  EVP_MAC_ptr mac(EVP_MAC_fetch(nullptr, "HMAC", nullptr));
  if (!mac) return false;

  OSSL_PARAM params[] = {
      OSSL_PARAM_construct_utf8_string(OSSL_MAC_PARAM_DIGEST,
                                       const_cast<char*>(cipher), 0),
      OSSL_PARAM_construct_end()};

  EVP_MAC_CTX_ptr ctx(EVP_MAC_CTX_new(mac.get()));

  if (!ctx || !EVP_MAC_init(ctx.get(), key, keyLen, params)) return false;

  m_ctx = std::move(ctx);
  return true;
}

bool State::updateImpl(const uint8_t* data, size_t datalen) noexcept {
  if (!m_ctx) return false;
  return EVP_MAC_update(m_ctx.get(), data, datalen) == 1;
}

bool State::finalImpl(uint8_t* out, size_t* outl, size_t outsize) noexcept {
  if (!m_ctx) return false;
  return EVP_MAC_final(m_ctx.get(), out, outl, outsize) == 1;
}

}  // namespace hmac
}  // namespace ikea400::crypto