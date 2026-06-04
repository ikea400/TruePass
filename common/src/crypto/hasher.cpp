#include "../../include/crypto/hasher.h"

namespace ikea400::crypto {

Hasher::Hasher(HashAlgo algorithm) { init(algorithm); }

bool Hasher::initImpl(HashAlgo algorithm) noexcept {
  m_mdCtx.reset();
  m_mdSize = 0;
  m_algo = HashAlgo::None;

  if (algorithm == HashAlgo::None) return false;

  const auto [md, mdSize] = getOpensslMd(algorithm);
  EVP_MD_CTX_ptr mdCtx(EVP_MD_CTX_new());
  if (!md || !mdSize || !mdCtx || !EVP_DigestInit(mdCtx.get(), md))
    return false;

  m_mdCtx = std::move(mdCtx);
  m_mdSize = mdSize;
  m_algo = algorithm;
  return true;
}

bool Hasher::cloneImpl(Hasher& outClone) const noexcept {
  if (!m_mdCtx) return false;

  outClone.m_mdCtx.reset(EVP_MD_CTX_dup(m_mdCtx.get()));
  if (!outClone.m_mdCtx) return false;

  outClone.m_mdSize = m_mdSize;
  outClone.m_algo = m_algo;
  return true;
}

bool Hasher::updateImpl(const void* data, size_t dataSize) noexcept {
  return m_mdCtx && EVP_DigestUpdate(m_mdCtx.get(), data, dataSize) == 1;
}

unsigned int Hasher::finalImpl(uint8_t* outHash,
                               unsigned int outHashSize) noexcept {
  if (outHashSize < getHashSize()) return 0;
  unsigned int mdSize = outHashSize;
  return m_mdCtx && EVP_DigestFinal_ex(m_mdCtx.get(), outHash, &mdSize) == 1
             ? mdSize
             : 0;
}

void Hasher::resetImpl() noexcept {
  if (m_mdCtx) EVP_MD_CTX_reset(m_mdCtx.get());
}

std::pair<const EVP_MD*, unsigned int> Hasher::getOpensslMd(
    HashAlgo algorithm) noexcept {
  switch (algorithm) {
    case HashAlgo::Sha1:
      return std::make_pair(EVP_sha1(), SHA1_HASH_SIZE);
    case HashAlgo::Sha256:
      return std::make_pair(EVP_sha256(), SHA256_HASH_SIZE);
    case HashAlgo::Sha384:
      return std::make_pair(EVP_sha384(), SHA384_HASH_SIZE);
    case HashAlgo::Sha3_256:
      return std::make_pair(EVP_sha3_256(), SHA3_256_HASH_SIZE);
    case HashAlgo::Sha3_384:
      return std::make_pair(EVP_sha3_384(), SHA3_384_HASH_SIZE);
    case HashAlgo::Sha3_512:
      return std::make_pair(EVP_sha3_512(), SHA3_512_HASH_SIZE);
    case HashAlgo::Sha512:
      return std::make_pair(EVP_sha512(), SHA512_HASH_SIZE);
    case HashAlgo::Blake2b512:
      return std::make_pair(EVP_blake2b512(), BLAKE2B512_HASH_SIZE);
    case HashAlgo::Blake2s256:
      return std::make_pair(EVP_blake2s256(), BLAKE2S256_HASH_SIZE);
    default:
      return {nullptr, 0};
  }
}

}  // namespace ikea400::crypto