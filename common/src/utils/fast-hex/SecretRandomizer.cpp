#include "../../../include/utils/SecretRandomizer.h"

#include <openssl/rand.h>
#include <utils/CryptoRandomizer.h>

using namespace ikea400;

ikea400::SecretRandomizer::SecretRandomizer()
    : CryptoRandomizer(RAND_priv_bytes) {}
