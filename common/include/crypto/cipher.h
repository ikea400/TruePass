#pragma once
#include <openssl/evp.h>
#include <openssl/params.h>

#include <memory>
#include <stdexcept>

#include "../utils/utils.h"

namespace ikea400 {

namespace crypto {
class AuthCipher;

class Cipher {
  friend class AuthCipher;

 public:
  Cipher(const Cipher&) = delete;
  Cipher& operator=(const Cipher&) = delete;

  inline unsigned int getKeyLen() const { return m_keyLen; }

 private:
  inline Cipher() : m_cipherCtx(nullptr), m_keyLen(0), m_isStream(false) {}

 protected:
  using EVP_CIPHER_CTX_ptr =
      std::unique_ptr<EVP_CIPHER_CTX,
                      utils::DeleterFromFn<&::EVP_CIPHER_CTX_free>>;

  EVP_CIPHER_CTX_ptr m_cipherCtx;
  unsigned int m_keyLen : 31;
  bool m_isStream : 1;
};

enum class AuthCipherAlgo {
  None = 0,
  AES_128_GCM,
  AES_192_GCM,
  AES_256_GCM,
  CHACHA20_POLY1305
};

enum CipherKeyLen {
  AES_128_KEY_LEN = 16,
  AES_192_KEY_LEN = 24,
  AES_256_KEY_LEN = 32,
  CHACHA20_POLY1305_KEY_LEN = 32
};

class AuthCryptor;
class AuthDecryptor;

class AuthCipher : private Cipher {
  friend class AuthCryptor;
  friend class AuthDecryptor;

 public:
  // For AEAD ciphers, output size is input size
  static constexpr inline int getOutputSize(int inputLen) noexcept {
    // (inl + cipher_block_size - 1)
    return (inputLen + getBlockLen() - 1);
  }

  static unsigned int getKeyLen(AuthCipherAlgo algo) noexcept;

  static inline constexpr unsigned int getIvLen() noexcept { return kIvLen; }
  static inline constexpr unsigned int getBlockLen() noexcept {
    return kBlockLen;
  }
  static inline constexpr unsigned int getTagLen() noexcept { return kTagLen; }
  static inline constexpr unsigned int getMaxKeyLen() noexcept {
    return kMaxKeyLen;
  }

 private:
  AuthCipher() = default;

  static std::tuple<const EVP_CIPHER*, unsigned int, bool> getOpensslCipher(
      AuthCipherAlgo algo) noexcept;

 private:
  static inline constexpr unsigned int kIvLen = 12;  // Value for gcm/chacha
  static inline constexpr unsigned int kBlockLen =
      1;  // openssl treat aead as stream cipher
  static inline constexpr unsigned int kTagLen = 16;  // Value for aes/chacha
  static inline constexpr unsigned int kMaxKeyLen =
      32;  // Value for aes-256/chacha
};

class AuthCryptor : public AuthCipher {
 public:
  inline AuthCryptor() = default;

  inline AuthCryptor(AuthCipherAlgo algo, const uint8_t* key,
                     unsigned int keyLen, const uint8_t* iv,
                     unsigned int ivLen) {
    init(algo, key, keyLen, iv, ivLen);
  }

  template <utils::RangeType K, utils::RangeType I>
  inline AuthCryptor(AuthCipherAlgo algo, const K& key, const I& iv) {
    init(algo, key, iv);
  }

  template <bool ThrowOnFail = true>
  inline bool init(AuthCipherAlgo algo, const uint8_t* key, unsigned int keyLen,
                   const uint8_t* iv,
                   unsigned int ivLen) noexcept(!ThrowOnFail) {
    bool success = initImpl(algo, key, keyLen, iv, ivLen);
    if constexpr (ThrowOnFail)
      if (!success)
        throw std::runtime_error("Failed to initialize AuthCipher.");
    return success;
  }

  template <bool ThrowOnFail = true, utils::RangeType K, utils::RangeType I>
  inline bool init(AuthCipherAlgo algo, const K& key,
                   const I& iv) noexcept(!ThrowOnFail) {
    if (key.size() * sizeof(K::value_type) >
            static_cast<size_t>(std::numeric_limits<unsigned int>::max()) ||
        iv.size() * sizeof(I::value_type) >
            static_cast<size_t>(std::numeric_limits<unsigned int>::max())) {
      if constexpr (ThrowOnFail)
        throw std::runtime_error(
            "AuthCrypto init key/iv size too large. Max is of unsigned int.");
      return false;
    }
    bool success =
        initImpl(algo, key.data(),
                 static_cast<unsigned int>(key.size() * sizeof(K::value_type)),
                 iv.data(),
                 static_cast<unsigned int>(iv.size() * sizeof(I::value_type)));
    if constexpr (ThrowOnFail)
      if (!success)
        throw std::runtime_error("Failed to initialize AuthCipher.");
    return success;
  }

  template <bool ThrowOnFail = true>
  int encrypt(const uint8_t* input, int inputLen, uint8_t* output,
              int outputLen, uint8_t* tagOutput,
              unsigned int tagLen) noexcept(!ThrowOnFail) {
    int encryptedLen =
        encryptImpl(input, inputLen, output, outputLen, tagOutput, tagLen);
    if constexpr (ThrowOnFail)
      if (!encryptedLen) throw std::runtime_error("AuthCrypto encrypt failed.");
    return encryptedLen;
  }

  template <bool ThrowOnFail = true, utils::RangeType I, utils::RangeType O,
            utils::RangeType T>
  int encrypt(const I& input, O& output, T& tag) noexcept(!ThrowOnFail) {
    if (input.size() * sizeof(I::value_type) >
            static_cast<size_t>(std::numeric_limits<int>::max()) ||
        output.size() * sizeof(O::value_type) >
            static_cast<size_t>(std::numeric_limits<int>::max()) ||
        tag.size() * sizeof(T::value_type) >
            static_cast<size_t>(std::numeric_limits<int>::max())) {
      if constexpr (ThrowOnFail)
        throw std::runtime_error(
            "AuthCrypto encrypt input/output/tag size too large.");
      return 0;
    }
    return encrypt<ThrowOnFail>(
        reinterpret_cast<const uint8_t*>(input.data()),
        static_cast<int>(input.size() * sizeof(I::value_type)),
        reinterpret_cast<uint8_t*>(output.data()),
        static_cast<int>(output.size() * sizeof(O::value_type)),
        reinterpret_cast<uint8_t*>(tag.data()),
        static_cast<int>(tag.size() * sizeof(T::value_type)));
  }

 private:
  bool initImpl(AuthCipherAlgo algo, const uint8_t* key, unsigned int keyLen,
                const uint8_t* iv, unsigned int ivLen) noexcept;

  int encryptImpl(const uint8_t* input, int inputLen, uint8_t* output,
                  int outputLen, uint8_t* tagOutput,
                  unsigned int tagLen) noexcept;
};

class AuthDecryptor : public AuthCipher {
 public:
  inline AuthDecryptor() = default;

  inline AuthDecryptor(AuthCipherAlgo algo, const uint8_t* key,
                       unsigned int keyLen, const uint8_t* iv,
                       unsigned int ivLen, const uint8_t* tag,
                       unsigned int tagLen) {
    init(algo, key, keyLen, iv, ivLen, tag, tagLen);
  }

  template <utils::RangeType K, utils::RangeType I, utils::RangeType T>
  inline AuthDecryptor(AuthCipherAlgo algo, const K& key, const I& iv,
                       const T& tag) {
    init(algo, key, iv, tag);
  }

  template <bool ThrowOnFail = true>
  inline bool init(AuthCipherAlgo algo, const uint8_t* key, unsigned int keyLen,
                   const uint8_t* iv, unsigned int ivLen, const uint8_t* tag,
                   unsigned int tagLen) noexcept(!ThrowOnFail) {
    bool success = initImpl(algo, key, keyLen, iv, ivLen, tag, tagLen);
    if constexpr (ThrowOnFail)
      if (!success)
        throw std::runtime_error("Failed to initialize AuthDecryptor.");
    return success;
  }

  template <bool ThrowOnFail = true, utils::RangeType K, utils::RangeType I,
            utils::RangeType T>
  inline bool init(AuthCipherAlgo algo, const K& key, const I& iv,
                   const T& tag) noexcept(!ThrowOnFail) {
    if (key.size() * sizeof(K::value_type) >
            static_cast<size_t>(std::numeric_limits<unsigned int>::max()) ||
        iv.size() * sizeof(I::value_type) >
            static_cast<size_t>(std::numeric_limits<unsigned int>::max()) ||
        tag.size() * sizeof(T::value_type) >
            static_cast<size_t>(std::numeric_limits<unsigned int>::max())) {
      if constexpr (ThrowOnFail)
        throw std::runtime_error(
            "AuthCrypto init key/iv/tag size too large. Max is of unsigned "
            "int.");
      return false;
    }
    bool success =
        initImpl(algo, reinterpret_cast<const uint8_t*>(key.data()),
                 static_cast<unsigned int>(key.size() * sizeof(K::value_type)),
                 reinterpret_cast<const uint8_t*>(iv.data()),
                 static_cast<unsigned int>(iv.size() * sizeof(I::value_type)),
                 reinterpret_cast<const uint8_t*>(tag.data()),
                 static_cast<unsigned int>(tag.size() * sizeof(T::value_type)));
    if constexpr (ThrowOnFail)
      if (!success)
        throw std::runtime_error("Failed to initialize AuthDecryptor.");
    return success;
  }

  template <bool ThrowOnFail = true>
  int decrypt(const uint8_t* input, int inputLen, uint8_t* output,
              int outputLen) noexcept(!ThrowOnFail) {
    int encryptedLen = decryptImpl(input, inputLen, output, outputLen);
    if constexpr (ThrowOnFail)
      if (!encryptedLen) throw std::runtime_error("AuthCrypto decrypt failed.");
    return encryptedLen;
  }

  template <bool ThrowOnFail = true, utils::RangeType I, utils::RangeType O>
  int decrypt(const I& input, O& output) noexcept(!ThrowOnFail) {
    if (input.size() * sizeof(I::value_type) >
            static_cast<size_t>(std::numeric_limits<int>::max()) ||
        output.size() * sizeof(O::value_type) >
            static_cast<size_t>(std::numeric_limits<int>::max())) {
      if constexpr (ThrowOnFail)
        throw std::runtime_error(
            "AuthCrypto decrypt input/output size too large.");
      return 0;
    }
    return decrypt<ThrowOnFail>(
        reinterpret_cast<const uint8_t*>(input.data()),
        static_cast<int>(input.size() * sizeof(I::value_type)),
        reinterpret_cast<uint8_t*>(output.data()),
        static_cast<int>(output.size() * sizeof(O::value_type)));
  }

 private:
  bool initImpl(AuthCipherAlgo algo, const uint8_t* key, unsigned int keyLen,
                const uint8_t* iv, unsigned int ivLen, const uint8_t* tag,
                unsigned int tagLen) noexcept;

  int decryptImpl(const uint8_t* input, int inputLen, uint8_t* output,
                  int outputLen) noexcept;
};
}  // namespace crypto

}  // namespace ikea400