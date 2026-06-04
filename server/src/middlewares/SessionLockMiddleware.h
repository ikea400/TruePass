#pragma once
#include <drogon/DrClassMap.h>
#include <drogon/HttpFilter.h>
#include <drogon/HttpMiddleware.h>
#include <drogon/HttpResponse.h>
#include <drogon/HttpTypes.h>
#include <drogon/RequestStream.h>
#include <drogon/drogon_callbacks.h>
#include <drogon/utils/FunctionTraits.h>
#include <json/value.h>
#include <trantor/utils/Logger.h>

#include <atomic>
#include <chrono>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

#include "../utils/ResponseBuilder.h"

class Test {};

using T = Test;

template <class T>
class SessionLockMiddleware
    : public drogon::HttpMiddleware<SessionLockMiddleware<T>> {
 public:
  SessionLockMiddleware() {}

  void invoke(const drogon::HttpRequestPtr& req,
              drogon::MiddlewareNextCallback&& nextCb,
              drogon::MiddlewareCallback&& mcb) override;

  static const std::string &className() {
    static std::string className =
        drogon::DrClassMap::demangle(typeid(SessionLockMiddleware<T>).name());
    return className;
  }

 private:
  struct LockData {
    LockData() = default;
    std::chrono::system_clock::time_point timeout{};
    std::atomic_size_t ownerId{};
  };
  using LockDataPtr = std::shared_ptr<LockData>;

  static constexpr std::string_view getTypeName() { return typeid(T).name(); }

  static bool onLocked(const LockDataPtr& lockData,
                       std::chrono::system_clock::time_point now,
                       size_t ownerId, size_t currOwnerId);

  static void onLockRelease(const LockDataPtr& lockData, size_t ownerId);

  static size_t getNextOwnerId() { return ++sessionLockCount; }

  static bool tryLock(const LockDataPtr &lockData, size_t &expectedOwner,
                             const size_t ownerId) {
    return lockData->ownerId.compare_exchange_strong(expectedOwner, ownerId);
  }

 private:
  static inline const std::string m_name = "session_lock_" + className();

  static inline std::atomic_size_t sessionLockCount = 0;

  static constexpr size_t kInvalidOwnerId = 0;
  static constexpr std::chrono::seconds kLockTimeout{30};
};

#define INSTANTIATE_SESSION_LOCK(TagName)                       \
  class TagName {};                                             \
  using SessionLock_##TagName = SessionLockMiddleware<TagName>; \
  static const SessionLock_##TagName filter_instance_##TagName;

INSTANTIATE_SESSION_LOCK(Auth)

template <class T>
inline void SessionLockMiddleware<T>::invoke(
    const drogon::HttpRequestPtr& req, drogon::MiddlewareNextCallback&& nextCb,
    drogon::MiddlewareCallback&& mcb) {
  LockDataPtr lockData;
  req->getSession()->modify<LockDataPtr>(m_name, [&](LockDataPtr& ld) {
    if (!ld) {
      ld = std::make_shared<LockData>();
    }
    lockData = ld;
  });

  size_t ownerId = getNextOwnerId();

  auto now = std::chrono::system_clock::now();
  std::atomic_int test;
  size_t currentOwnerId = kInvalidOwnerId;
  if (!tryLock(lockData, currentOwnerId, ownerId)) {
    bool allowedToProceed = onLocked(lockData, now, ownerId, currentOwnerId);

    if (!allowedToProceed) {
      // Session is already locked by another request, return an
      // error response

      return mcb(ikea400::ResponseBuilder::get(req)->failure(
          "Session is locked by another request, please try again later",
          drogon::k423Locked));
    }
  }

  lockData->timeout = now + kLockTimeout;

  nextCb([mcb = std::move(mcb), lockData,
          ownerId](const drogon::HttpResponsePtr& resp) {
    mcb(resp);

    onLockRelease(lockData, ownerId);
  });
}

template <class T>
inline bool SessionLockMiddleware<T>::onLocked(
    const LockDataPtr& lockData, std::chrono::system_clock::time_point now,
    size_t ownerId, size_t currOwnerId) {
  if (now > lockData->timeout) {
    if (!tryLock(lockData, currOwnerId, ownerId)) {
      LOG_ERROR << "Session lock was timeout for "
                << std::chrono::duration_cast<std::chrono::seconds>(
                       now - lockData->timeout)
                       .count()
                << "s, but failed to acquire lock for request by "
                << getTypeName()
                << ", current lock owner: " << lockData->ownerId;
      return false;
    }

    if (currOwnerId == ownerId) {
      LOG_ERROR << "Session lock was timeout for "
                << std::chrono::duration_cast<std::chrono::seconds>(
                       now - lockData->timeout)
                       .count()
                << "s by " << getTypeName() << ", resetting lock";
    } else {
      LOG_ERROR << "Session lock was timout for "
                << std::chrono::duration_cast<std::chrono::seconds>(
                       now - lockData->timeout)
                       .count()
                << "s";
    }

    return true;
  }

  return false;
}

template <class T>
inline void SessionLockMiddleware<T>::onLockRelease(const LockDataPtr& lockData,
                                                    size_t ownerId) {
  if (!lockData) {
    LOG_ERROR << "Session lock data for " << getTypeName()
              << " is NULL when releasing lock";
  } else if (!tryLock(lockData, ownerId, kInvalidOwnerId)) {
    LOG_ERROR << "Session lock owner mismatch when releasing lock, expected: "
              << ownerId << ", actual: " << lockData->ownerId;
  } else {
    lockData->timeout = {};
  }
}
