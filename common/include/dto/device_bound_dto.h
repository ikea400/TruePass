#pragma once

#include <optional>
#include <string>

#include "../utils/BinArray.h"
#include "../utils/BinVector.h"

namespace ikea400::dto {

using DevicePublicKey = BinVector<91, 158>;

using DeviceBoundChallenge = BinArray<32>;

using DeviceBoundProof = BinVector<64, 72>;

}  // namespace ikea400::dto