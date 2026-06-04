#include "../../include/crypto/cipher.h"

#include <openssl/core_names.h>

namespace ikea400::crypto {

unsigned int AuthCipher::getKeyLen(AuthCipherAlgo algo) noexcept {
  switch (algo) {
    case ikea400::crypto::AuthCipherAlgo::AES_128_GCM:
      return CipherKeyLen::AES_128_KEY_LEN;
    case ikea400::crypto::AuthCipherAlgo::AES_192_GCM:
      return CipherKeyLen::AES_192_KEY_LEN;
    case ikea400::crypto::AuthCipherAlgo::AES_256_GCM:
      return CipherKeyLen::AES_256_KEY_LEN;
    case ikea400::crypto::AuthCipherAlgo::CHACHA20_POLY1305:
      return CipherKeyLen::CHACHA20_POLY1305_KEY_LEN;
    default:
      return 0;
  }
}

std::tuple<const EVP_CIPHER*, unsigned int, bool> AuthCipher::getOpensslCipher(
    AuthCipherAlgo algo) noexcept {
  unsigned int keyLen = getKeyLen(algo);
  switch (algo) {
    case AuthCipherAlgo::AES_128_GCM:
      return std::make_tuple(EVP_aes_128_gcm(), keyLen, false);
    case AuthCipherAlgo::AES_192_GCM:
      return std::make_tuple(EVP_aes_192_gcm(), keyLen, false);
    case AuthCipherAlgo::AES_256_GCM:
      return std::make_tuple(EVP_aes_256_gcm(), keyLen, false);
    case AuthCipherAlgo::CHACHA20_POLY1305:
      return std::make_tuple(EVP_chacha20_poly1305(), keyLen, true);
    default:
      return {nullptr, 0, false};
  }
}

bool AuthCryptor::initImpl(AuthCipherAlgo algo, const uint8_t* key,
                           unsigned int keyLen, const uint8_t* iv,
                           unsigned int ivLen) noexcept {
  m_cipherCtx.reset();
  m_keyLen = 0;
  m_isStream = false;

  if (ivLen != getIvLen()) return false;

  const auto [cipher, cipherKeyLen, stream] = getOpensslCipher(algo);
  if (!cipher || keyLen != cipherKeyLen) return false;

  EVP_CIPHER_CTX_ptr ctx(EVP_CIPHER_CTX_new());
  if (!ctx) return false;

  std::array params{OSSL_PARAM_construct_uint(OSSL_CIPHER_PARAM_IVLEN, &ivLen),
                    OSSL_PARAM_construct_end()};

  if (EVP_EncryptInit_ex2(ctx.get(), cipher, key, iv, params.data()) != 1)
    return false;

  m_cipherCtx = std::move(ctx);
  m_keyLen = cipherKeyLen;
  m_isStream = stream;
  return true;
}

int AuthCryptor::encryptImpl(const uint8_t* input, int inputLen,
                             uint8_t* output, int outputLen, uint8_t* tagOutput,
                             unsigned int tagLen) noexcept {
  if (outputLen < getOutputSize(inputLen) || tagLen != getTagLen() ||
      !m_cipherCtx)
    return 0;

  int outLen1 = 0;
  if (EVP_EncryptUpdate(m_cipherCtx.get(), output, &outLen1, input, inputLen) !=
      1)
    return 0;

  int outLen2 = 0;
  if (EVP_EncryptFinal_ex(m_cipherCtx.get(), output + outLen1, &outLen2) != 1)
    return 0;

  static_assert(
      EVP_CTRL_AEAD_GET_TAG == EVP_CTRL_GCM_GET_TAG,
      "EVP_CTRL_AEAD_GET_TAG and EVP_CTRL_GCM_GET_TAG should be the same");

  if (EVP_CIPHER_CTX_ctrl(m_cipherCtx.get(), EVP_CTRL_AEAD_GET_TAG, tagLen,
                          tagOutput) != 1)
    return 0;

  return outLen1 + outLen2;
}

bool AuthDecryptor::initImpl(AuthCipherAlgo algo, const uint8_t* key,
                             unsigned int keyLen, const uint8_t* iv,
                             unsigned int ivLen, const uint8_t* tag,
                             unsigned int tagLen) noexcept {
  if (ivLen != getIvLen()) return false;
  if (!tag || tagLen != getTagLen()) return false;

  const auto [cipher, cipherKeyLen, stream] = getOpensslCipher(algo);
  if (!cipher || keyLen != cipherKeyLen) return false;

  EVP_CIPHER_CTX_ptr ctx(EVP_CIPHER_CTX_new());
  if (!ctx) return false;

  std::array params{
      OSSL_PARAM_construct_uint(OSSL_CIPHER_PARAM_IVLEN, &ivLen),
      OSSL_PARAM_construct_octet_string(OSSL_CIPHER_PARAM_AEAD_TAG,
                                        const_cast<uint8_t*>(tag), tagLen),
      OSSL_PARAM_construct_end()};

  if (EVP_DecryptInit_ex2(ctx.get(), cipher, key, iv, params.data()) != 1)
    return false;

  m_cipherCtx = std::move(ctx);
  m_keyLen = cipherKeyLen;
  m_isStream = stream;
  return true;
}

int AuthDecryptor::decryptImpl(const uint8_t* input, int inputLen,
                               uint8_t* output, int outputLen) noexcept {
  if (outputLen < inputLen || !m_cipherCtx) return 0;

  int outLen1 = 0;
  if (EVP_DecryptUpdate(m_cipherCtx.get(), output, &outLen1, input, inputLen) !=
      1)
    return 0;

  int outLen2 = 0;
  if (EVP_DecryptFinal_ex(m_cipherCtx.get(), output + outLen1, &outLen2) != 1)
    return 0;

  return outLen1 + outLen2;
}

}  // namespace ikea400::crypto