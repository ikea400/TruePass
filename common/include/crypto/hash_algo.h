#pragma once

namespace ikea400::crypto {
enum class HashAlgo {
  None,
  Sha1,
  Sha256,
  Sha384,
  Sha512,
  Sha3_256,
  Sha3_384,
  Sha3_512,
  Blake2b512,
  Blake2s256,
};

enum HashSize {
  SHA1_HASH_SIZE = 20,
  SHA256_HASH_SIZE = 32,
  SHA384_HASH_SIZE = 48,
  SHA512_HASH_SIZE = 64,
  SHA3_256_HASH_SIZE = 32,
  SHA3_384_HASH_SIZE = 48,
  SHA3_512_HASH_SIZE = 64,
  BLAKE2B512_HASH_SIZE = 64,
  BLAKE2S256_HASH_SIZE = 32,
  MAX_HASH_SIZE = 64
};

const char* getHashProviderName(HashAlgo algo) noexcept;

}  // namespace ikea400::crypto