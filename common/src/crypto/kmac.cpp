#include "../../include/crypto/kmac.h"

#include <openssl/core_names.h>
#include <openssl/evp.h>
#include <openssl/params.h>
#include <openssl/types.h>

#include <cstdint>
#include <utility>

namespace ikea400::crypto {

namespace kmac {
bool State::initImpl(Variant variant, const uint8_t *key, size_t keyLen,
                     const uint8_t *custom, size_t customLen) noexcept {
  const char *algo = nullptr;
  switch (variant) {
    case ikea400::crypto::kmac::Variant::KMAC_128:
      algo = "KMAC-128";
      break;
    case ikea400::crypto::kmac::Variant::KMAC_256:
      algo = "KMAC-256";
      break;
    default:
      break;
  }

  if (!algo) return false;

  EVP_MAC_ptr mac(EVP_MAC_fetch(nullptr, algo, nullptr));
  if (!mac) return false;

  OSSL_PARAM params[] = {OSSL_PARAM_construct_octet_string(
                             OSSL_MAC_PARAM_CUSTOM, (void *)custom, customLen),
                         OSSL_PARAM_construct_end()};

  EVP_MAC_CTX_ptr ctx(EVP_MAC_CTX_new(mac.get()));

  if (!ctx || !EVP_MAC_init(ctx.get(), key, keyLen, params)) return false;

  m_ctx = std::move(ctx);
  return true;
}

bool State::updateImpl(const uint8_t *data, size_t datalen) noexcept {
  if (!m_ctx) return false;
  return EVP_MAC_update(m_ctx.get(), data, datalen) == 1;
}

bool State::finalImpl(uint8_t *out, size_t *outl, size_t outsize,
                      bool xofEnabled) noexcept {
  if (!m_ctx) return false;

  if (outsize > static_cast<size_t>(std::numeric_limits<int>::max()))
    return false;

  int xofEnabledInt = xofEnabled ? 1 : 0;
  int outsizeInt = static_cast<int>(outsize);

  OSSL_PARAM params[] = {
      OSSL_PARAM_construct_int(OSSL_MAC_PARAM_XOF, &xofEnabledInt),
      OSSL_PARAM_construct_int(OSSL_MAC_PARAM_SIZE, &outsizeInt),
      OSSL_PARAM_construct_end()};
  if (!EVP_MAC_CTX_set_params(m_ctx.get(), params)) return false;
  return EVP_MAC_final(m_ctx.get(), out, outl, outsize) == 1;
}
}  // namespace kmac
}  // namespace ikea400::crypto