#pragma once
#include <opaque++.h>
#include <utils/SecureArray.h>
#include <utils/uuid.h>

#include <cstdint>
#include <memory>
#include <mutex>
#include <optional>
#include <string>

#include "DeviceBoundContext.h"

namespace ikea400 {
struct UserSession {
  std::mutex mutex;
  uuid userId;
  uuid cookieId;
  SecureArray<uint8_t, opaque::kSessionKeySize> sessionKey;
  std::optional<DeviceBoundContext> deviceContext;
};

using UserSessionPtr = std::shared_ptr<UserSession>;
}  // namespace ikea400