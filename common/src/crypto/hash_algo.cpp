#include "../../include/crypto/hash_algo.h"

#include <openssl/obj_mac.h>

namespace ikea400::crypto {
const char* getHashProviderName(
    HashAlgo algo) noexcept {
  switch (algo) {
    case HashAlgo::Sha1:
      return SN_sha1;
    case HashAlgo::Sha256:
      return SN_sha256;
    case HashAlgo::Sha384:
      return SN_sha384;
    case HashAlgo::Sha512:
      return SN_sha512;
    case HashAlgo::Sha3_256:
      return SN_sha3_256;
    case HashAlgo::Sha3_384:
      return SN_sha3_384;
    case HashAlgo::Sha3_512:
      return SN_sha3_512;
    case HashAlgo::Blake2b512:
      return SN_blake2b512;
    case HashAlgo::Blake2s256:
      return SN_blake2s256;
    default:
      return nullptr;
  }
}
}  // namespace ikea400::crypto