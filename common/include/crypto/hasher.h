#pragma once
#include <openssl/evp.h>

#include <memory>
#include <stdexcept>

#include "../utils/utils.h"
#include "hash_algo.h"

namespace ikea400::crypto {

class Hasher {
 public:
  Hasher& operator=(const Hasher&) = delete;
  Hasher(const Hasher& other) = delete;

  Hasher() = default;
  explicit Hasher(HashAlgo algorithm);

  inline Hasher(Hasher&& other) noexcept = default;
  inline Hasher& operator=(Hasher&& other) noexcept = default;

  template <bool ThrowOnFail = true>
  bool init(const HashAlgo algorithm) {
    bool success = initImpl(algorithm);
    if constexpr (ThrowOnFail)
      if (!success) throw std::runtime_error("Hasher initialization failed.");
    return success;
  }

  template <bool ThrowOnFail = true>
  inline Hasher clone() noexcept(!ThrowOnFail) {
    Hasher newHasher;
    clone<ThrowOnFail>(newHasher);
    return newHasher;
  }

  template <bool ThrowOnFail = true>
  inline bool clone(Hasher& other) noexcept(!ThrowOnFail) {
    bool success = cloneImpl(other);
    if constexpr (ThrowOnFail)
      if (!success) throw std::runtime_error("Hasher clone failed.");
    return success;
  }

  bool isValid() const noexcept { return m_mdCtx != nullptr; }

  inline unsigned int getHashSize() const noexcept { return m_mdSize; }

  inline const EVP_MD* getNative() const noexcept {
    if (!m_mdCtx) return nullptr;
    return EVP_MD_CTX_get0_md(m_mdCtx.get());
  }

  inline HashAlgo getAlgo() const noexcept {
    if (!m_mdCtx) return HashAlgo::None;
    return m_algo;
  }

  template <bool ThrowOnFail = true, class T>
    requires requires(const T& v) {
      v.data();
      v.size();
      typename T::value_type;
    }
  bool update(const T& data) noexcept(!ThrowOnFail) {
    return update<ThrowOnFail>(data.data(),
                               data.size() * sizeof(typename T::value_type));
  }

  template <bool ThrowOnFail = true, class T, std::size_t N>
  bool update(const T (&data)[N]) noexcept(!ThrowOnFail) {
    return update<ThrowOnFail>(data, sizeof(T) * N);
  }

  template <bool ThrowOnFail = true, class T>
    requires(std::is_arithmetic_v<T> || std::is_enum_v<T>)
  bool update(const T& data) noexcept(!ThrowOnFail) {
    return update<ThrowOnFail>(&data, sizeof(T));
  }

  template <bool ThrowOnFail = true, typename... Args>
  bool updateMultiple(Args&&... args) noexcept(!ThrowOnFail) {
    return (update<ThrowOnFail>(std::forward<Args>(args)) && ...);
  }

  template <bool ThrowOnFail = true>
  inline bool update(const void* data, size_t dataSize) noexcept(!ThrowOnFail) {
    bool success = updateImpl(data, dataSize);
    if constexpr (ThrowOnFail)
      if (!success) throw std::runtime_error("Hasher update failed.");

    return success;
  }

  template <bool bResetOnSuccess = true, bool ThrowOnFail = true>
  inline unsigned int final(uint8_t* outHash,
                            size_t outHashSize) noexcept(!ThrowOnFail) {
    if (outHashSize < m_mdSize) {
      if constexpr (ThrowOnFail)
        throw std::runtime_error("Output buffer is too small for the hash.");
      return 0;
    }

    if (outHashSize >=
        static_cast<size_t>(std::numeric_limits<unsigned int>::max())) {
      if constexpr (ThrowOnFail)
        throw std::runtime_error("Hash final outhash too big");
      return 0;
    }

    unsigned int hashSize =
        finalImpl(outHash, static_cast<unsigned int>(outHashSize));
    if constexpr (ThrowOnFail)
      if (hashSize == 0)
        throw std::runtime_error("Hasher finalization failed.");
    if constexpr (bResetOnSuccess)
      if (hashSize > 0) resetImpl();
    return hashSize;
  }

  template <bool bResetOnSuccess = true, bool ThrowOnFail = true,
            utils::RangeType T>
  inline unsigned int final(T& outHash) noexcept(!ThrowOnFail) {
    return final<bResetOnSuccess, ThrowOnFail>(
        outHash.data(), outHash.size() * sizeof(T::value_type));
  }

  template <bool ThrowOnFail = true>
  inline void reset() noexcept(!ThrowOnFail) {
    if (!m_mdCtx) {
      if constexpr (ThrowOnFail)
        throw std::runtime_error("Hasher is not initialized.");
      return;
    }
    resetImpl();
  }

  template <bool bResetOnSuccess = true, bool ThrowOnFail = true>
  inline bool hash(const void* data, size_t dataSize, uint8_t* outHash,
                   size_t outHashSize) noexcept(!ThrowOnFail) {
    if (outHashSize >=
        static_cast<size_t>(std::numeric_limits<unsigned int>::max())) {
      if constexpr (ThrowOnFail)
        throw std::runtime_error("Hash hash outhash too big");
      return false;
    }
    bool success =
        updateImpl(data, dataSize) &&
        finalImpl(outHash, static_cast<unsigned int>(outHashSize)) > 0;
    if constexpr (ThrowOnFail)
      if (!success) throw std::runtime_error("Hasher hash operation failed.");
    if constexpr (bResetOnSuccess)
      if (success) resetImpl();
    return success;
  }

  template <bool ThrowOnFail = true, utils::RangeType I, utils::RangeType O,
            bool bResetOnSuccess = true>
  inline bool hash(const I& inData, O& outHash) noexcept(!ThrowOnFail) {
    return hash<bResetOnSuccess, ThrowOnFail>(
        inData.data(), inData.size() * sizeof(I::value_type), outHash.data(),
        outHash.size() * sizeof(O::value_type));
  }

  template <bool ThrowOnFail = true>
  static inline bool hash(HashAlgo algorithm, const void* data, size_t dataSize,
                          uint8_t* outHash,
                          size_t outHashSize) noexcept(!ThrowOnFail) {
    if (outHashSize >=
        static_cast<size_t>(std::numeric_limits<unsigned int>::max())) {
      if constexpr (ThrowOnFail)
        throw std::runtime_error("Hash hash outhash too big");
      return 0;
    }

    Hasher hasher;
    bool success =
        hasher.init(algorithm) && hasher.updateImpl(data, dataSize) &&
        hasher.finalImpl(outHash, static_cast<unsigned int>(outHashSize)) > 0;
    if constexpr (ThrowOnFail)
      if (!success)
        throw std::runtime_error("Hasher static hash operation failed.");
    return success;
  }

  template <bool ThrowOnFail = true, utils::RangeType I, utils::RangeType O>
  static inline bool hash(HashAlgo algorithm, const I& inData,
                          O& outHash) noexcept(!ThrowOnFail) {
    return hash<ThrowOnFail>(
        algorithm, inData.data(), inData.size() * sizeof(I::value_type),
        outHash.data(), outHash.size() * sizeof(O::value_type));
  }

  static inline bool isValidAlgorithm(HashAlgo algorithm) noexcept {
    return getHashSize(algorithm) != 0;
  }

  static constexpr size_t maxHashSize() noexcept {
    // The maximum hash size supported (SHA-512, 64 bytes)
    return 64;
  }

  static constexpr unsigned int getHashSize(HashAlgo algorithm) noexcept {
    switch (algorithm) {
      case HashAlgo::Sha1:
        return SHA1_HASH_SIZE;
      case HashAlgo::Sha256:
        return SHA256_HASH_SIZE;
      case HashAlgo::Sha384:
        return SHA384_HASH_SIZE;
      case HashAlgo::Sha3_256:
        return SHA3_256_HASH_SIZE;
      case HashAlgo::Sha3_384:
        return SHA3_384_HASH_SIZE;
      case HashAlgo::Sha3_512:
        return SHA3_512_HASH_SIZE;
      case HashAlgo::Blake2b512:
        return BLAKE2B512_HASH_SIZE;
      case HashAlgo::Blake2s256:
        return BLAKE2S256_HASH_SIZE;
      default:
        return 0;
    }
  }

 private:
  bool initImpl(HashAlgo algorithm) noexcept;
  bool cloneImpl(Hasher& outClone) const noexcept;
  bool updateImpl(const void* data, size_t dataSize) noexcept;
  unsigned int finalImpl(uint8_t* outHash, unsigned int outHashSize) noexcept;
  void resetImpl() noexcept;

  static std::pair<const EVP_MD*, unsigned int> getOpensslMd(
      HashAlgo algorithm) noexcept;

 private:
  using EVP_MD_CTX_ptr =
      std::unique_ptr<EVP_MD_CTX, utils::DeleterFromFn<&::EVP_MD_CTX_free>>;

  EVP_MD_CTX_ptr m_mdCtx;
  unsigned int m_mdSize{0};
  HashAlgo m_algo{HashAlgo::None};
};
}  // namespace ikea400::crypto