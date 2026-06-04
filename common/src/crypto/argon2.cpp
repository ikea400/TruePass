#include "../../include/crypto/argon2.h"

#include <openssl/core_names.h>
#include <openssl/kdf.h>
#include <openssl/params.h>
#if USE_OPENSSL_ARGON2
#include <openssl/thread.h>
#else
#include <argon2.h>
#endif

#include <memory>

namespace ikea400::crypto {
using EVP_KDF_ptr =
    std::unique_ptr<EVP_KDF, utils::DeleterFromFn<&::EVP_KDF_free>>;
using EVP_KDF_CTX_ptr =
    std::unique_ptr<EVP_KDF_CTX, utils::DeleterFromFn<&::EVP_KDF_CTX_free>>;
using OSSL_LIB_CTX_ptr =
    std::unique_ptr<OSSL_LIB_CTX, utils::DeleterFromFn<&::OSSL_LIB_CTX_free>>;

namespace argon2 {
bool detail::derive(Variant variant, const void* password, size_t passwordLen,
                    const void* salt, size_t saltLen, uint32_t tCost,
                    uint32_t mCost, uint32_t parallelism, uint8_t* out,
                    size_t outLen) noexcept {
  if (!password || !passwordLen || !salt || !saltLen || !out ||
      outLen > kMaxOutputSize || outLen < kMinOutputSize)
    return false;

#if USE_OPENSSL_ARGON2
  const char* variantName = nullptr;
  switch (variant) {
    case argon2::Variant::Argon2d:
      variantName = "ARGON2D";
      break;
    case argon2::Variant::Argon2i:
      variantName = "ARGON2I";
      break;
    case argon2::Variant::Argon2id:
      variantName = "ARGON2ID";
      break;
    default:
      break;
  }

  if (!variantName) return false;

  OSSL_LIB_CTX_ptr localCtx(OSSL_LIB_CTX_new());
  if (!localCtx) return false;

  uint32_t threads = parallelism;
  if (threads > 1) {
    /* required if threads > 1 */
    if (OSSL_set_max_threads(localCtx.get(), threads) != 1) return false;
  }

  OSSL_PARAM params[7] = {
      OSSL_PARAM_construct_uint32(OSSL_KDF_PARAM_THREADS, &threads),
      OSSL_PARAM_construct_uint32(OSSL_KDF_PARAM_ARGON2_LANES, &parallelism),
      OSSL_PARAM_construct_uint32(OSSL_KDF_PARAM_ARGON2_MEMCOST, &mCost),
      OSSL_PARAM_construct_uint32(OSSL_KDF_PARAM_ITER, &tCost),
      OSSL_PARAM_construct_octet_string(OSSL_KDF_PARAM_SALT,
                                        const_cast<void*>(salt), saltLen),
      OSSL_PARAM_construct_octet_string(
          OSSL_KDF_PARAM_PASSWORD, const_cast<void*>(password), passwordLen),
      OSSL_PARAM_construct_end()};

  EVP_KDF_ptr kdf(EVP_KDF_fetch(localCtx.get(), variantName, nullptr));
  if (!kdf) return false;

  EVP_KDF_CTX_ptr kctx(EVP_KDF_CTX_new(kdf.get()));
  if (!kctx || EVP_KDF_derive(kctx.get(), out, outLen, params) != 1)
    return false;

  return true;
#else
  using HashFnPtr = int (*)(uint32_t, uint32_t, uint32_t, const void*, size_t,
                            const void*, size_t, uint8_t*, size_t);

  static constexpr std::array<HashFnPtr, 3> hashFns = {
      [](auto... args) { return argon2d_hash_raw(args...); },
      [](auto... args) { return argon2i_hash_raw(args...); },
      [](auto... args) { return argon2id_hash_raw(args...); }};

  size_t index = static_cast<size_t>(variant);
  if (index >= hashFns.size()) return false;

  return hashFns[index](tCost, mCost, parallelism, password, passwordLen, salt,
                        saltLen, out, outLen) == ARGON2_OK;
#endif
}
}  // namespace argon2
}  // namespace ikea400::crypto