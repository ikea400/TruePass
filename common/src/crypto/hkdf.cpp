#include "../../include/crypto/hkdf.h"

#include <crypto/hash_algo.h>
#include <openssl/core_names.h>
#include <openssl/evp.h>
#include <openssl/kdf.h>
#include <openssl/params.h>
#include <openssl/types.h>
#include <utils/utils.h>

#include <array>
#include <bit>
#include <cstdint>
#include <limits>
#include <memory>
#include <string_view>
#include <vector>

namespace ikea400::crypto {
using EVP_KDF_ptr =
    std::unique_ptr<EVP_KDF, utils::DeleterFromFn<&::EVP_KDF_free>>;
using EVP_KDF_CTX_ptr =
    std::unique_ptr<EVP_KDF_CTX, utils::DeleterFromFn<&::EVP_KDF_CTX_free>>;

namespace hkdf {
bool detail::execute(const HashAlgo digest, Mode mode, const void* salt,
                     size_t saltLen, const void* ikm, size_t ikmLen,
                     const void* info, size_t infoLen, uint8_t* outKey,
                     size_t outKeyLen) noexcept {
  std::string_view digestName = getHashProviderName(digest);
  if (digestName.empty()) return false;

  EVP_KDF_ptr kdf(EVP_KDF_fetch(nullptr, "HKDF", nullptr));
  EVP_KDF_CTX_ptr ctx;
  if (!kdf || !(ctx = EVP_KDF_CTX_ptr(EVP_KDF_CTX_new(kdf.get()))))
    return false;

  int modeValue = EVP_KDF_HKDF_MODE_EXTRACT_AND_EXPAND;
  switch (mode) {
    case detail::Mode::Extract:
      modeValue = EVP_KDF_HKDF_MODE_EXTRACT_ONLY;
      break;
    case detail::Mode::Expand:
      modeValue = EVP_KDF_HKDF_MODE_EXPAND_ONLY;
      break;
    default:
      break;
  }

  OSSL_PARAM params[6];
  int p = 0;

  params[p++] = OSSL_PARAM_construct_int(OSSL_KDF_PARAM_MODE, &modeValue);
  params[p++] = OSSL_PARAM_construct_utf8_string(
      OSSL_KDF_PARAM_DIGEST, const_cast<char*>(digestName.data()), 0);

  if (ikm && ikmLen)
    params[p++] = OSSL_PARAM_construct_octet_string(
        OSSL_KDF_PARAM_KEY, const_cast<void*>(ikm), ikmLen);

  if (salt && saltLen)
    params[p++] = OSSL_PARAM_construct_octet_string(
        OSSL_KDF_PARAM_SALT, const_cast<void*>(salt), saltLen);

  if (info && infoLen)
    params[p++] = OSSL_PARAM_construct_octet_string(
        OSSL_KDF_PARAM_INFO, const_cast<void*>(info), infoLen);

  params[p] = OSSL_PARAM_construct_end();

  return EVP_KDF_derive(ctx.get(), outKey, outKeyLen, params) == 1;
}

bool detail::expandLabel(const HashAlgo digest, const void* prk, size_t prkLen,
                         const std::string_view label, const void* context,
                         size_t contextLen, uint8_t* outKey,
                         size_t outKeyLen) noexcept {
  std::vector<uint8_t> hkdfLabel = makeLabel(
      label, static_cast<const uint8_t*>(context), contextLen, outKeyLen);
  if (hkdfLabel.empty()) return false;

  return execute(digest, Mode::Expand, nullptr, 0, prk, prkLen,
                 hkdfLabel.data(), hkdfLabel.size(), outKey, outKeyLen);
}

std::vector<uint8_t> detail::makeLabel(const std::string_view label,
                                       const uint8_t* context,
                                       size_t contextLen,
                                       size_t outLen) noexcept {
  constexpr std::string_view prefix = "truepass ";

  if (outLen > std::numeric_limits<uint16_t>::max()) return {};
  if (contextLen != 0 && context == nullptr) return {};
  if (contextLen > std::numeric_limits<uint8_t>::max() ||
      prefix.size() + label.size() > std::numeric_limits<uint8_t>::max()) {
    return {};
  }

  try {
    std::vector<uint8_t> hkdfLabel;
    hkdfLabel.reserve(2 + 1 + prefix.size() + label.size() + 1 + contextLen);

    uint16_t bigLen = utils::native_to_big(static_cast<uint16_t>(outLen));

    auto lenBytes = std::bit_cast<std::array<uint8_t, 2>>(bigLen);
    hkdfLabel.push_back(lenBytes[0]);
    hkdfLabel.push_back(lenBytes[1]);

    hkdfLabel.push_back(static_cast<uint8_t>(prefix.size() + label.size()));
    hkdfLabel.insert(hkdfLabel.end(), prefix.begin(), prefix.end());
    hkdfLabel.insert(hkdfLabel.end(), label.begin(), label.end());

    hkdfLabel.push_back(static_cast<uint8_t>(contextLen));
    hkdfLabel.insert(hkdfLabel.end(), context, context + contextLen);

    return hkdfLabel;
  } catch (...) {
    return {};
  }
}
}  // namespace hkdf
}  // namespace ikea400::crypto