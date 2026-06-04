#include "SessionService.h"

#include <drogon/Cookie.h>
#include <drogon/HttpAppFramework.h>
#include <drogon/RequestStream.h>
#include <drogon/Session.h>
#include <json/value.h>
#include <trantor/utils/Logger.h>
#include <utils/SecureArray.h>
#include <utils/utils.h>
#include <utils/uuid.h>

#include <algorithm>
#include <cstdint>
#include <memory>
#include <mutex>
#include <optional>
#include <span>
#include <stdexcept>
#include <utility>

#include "../models/DeviceBoundContext.h"
#include "../models/UserSession.h"

using namespace ikea400;

namespace {
constexpr std::int64_t kDefaultCookieTimeoutSeconds = 900;
constexpr std::int64_t kMinCookieTimeoutSeconds = 1;
constexpr std::int64_t kMaxCookieTimeoutSeconds = 30 * 24 * 60 * 60;

// Clamp untrusted configuration into a safe operational window.
[[nodiscard]] std::int64_t sanitizeTimeout(std::int64_t raw) noexcept {
  return std::clamp(raw, kMinCookieTimeoutSeconds, kMaxCookieTimeoutSeconds);
}

// Convert a timeout in seconds into CacheMap wheel/bucket sizing.
[[nodiscard]] std::pair<size_t, size_t> computeCookieCacheLayout(
    std::int64_t timeoutSeconds) noexcept {
  timeoutSeconds = sanitizeTimeout(timeoutSeconds);

  // Small timeouts get one wheel and a bucket per second.
  if (timeoutSeconds < 500) {
    return {1, static_cast<size_t>(timeoutSeconds + 1)};
  }

  // Larger timeouts are spread across multiple wheels to avoid huge bucket
  // counts.
  size_t wheelNum = 1;
  std::int64_t remaining = timeoutSeconds;
  while (remaining > 100) {
    ++wheelNum;
    remaining /= 100;
  }

  return {wheelNum, 100};
}

// Read the auth_cookie subtree safely and apply defaults.
[[nodiscard]] Json::Value getAuthCookieConfig() {
  const Json::Value& customConfig = drogon::app().getCustomConfig();
  return customConfig.get("auth_cookie", Json::Value{Json::objectValue});
}
}  // namespace

uuid ikea400::SessionService::getAuthenticatedUserId(
    const drogon::HttpRequestPtr& req) {
  if (!req) {
    return uuid::null();
  }

  UserSessionPtr userSession =
      req->session()->get<UserSessionPtr>(kAuthSessionKey);
  if (!userSession) {
    return uuid::null();
  }

  std::scoped_lock lock(userSession->mutex);

  const uuid cookieId = uuid::fromString<false>(req->getCookie(m_cookieKey));
  if (cookieId.isNull()) {
    return uuid::null();
  }

  if (cookieId != userSession->cookieId) {
    return uuid::null();
  }

  AuthCookieData cookieData;
  if (!m_authCookieCache->findAndFetch(cookieId, cookieData)) {
    return uuid::null();
  }

  if (cookieData.userId.isNull() || cookieData.userId != userSession->userId) {
    return uuid::null();
  }

  if (cookieData.expireTime <= utils::unix_seconds()) {
    return uuid::null();
  }

  return userSession->userId;
}

void SessionService::deauthenticate(const drogon::SessionPtr& session) {
  if (!session) {
    return;
  }

  UserSessionPtr userSession = session->get<UserSessionPtr>(kAuthSessionKey);
  if (userSession) {
    std::scoped_lock lock(userSession->mutex);

    m_authCookieCache->erase(userSession->cookieId);

    userSession->userId = uuid::null();
    userSession->cookieId = uuid::null();
    userSession->sessionKey.clear();
    userSession->deviceContext.reset();
  }

  session->erase(kAuthSessionKey);
}

drogon::Cookie SessionService::authenticate(
    const drogon::SessionPtr& session, uuid userId,
    const std::span<const uint8_t> sessionKey,
    std::optional<ikea400::DeviceBoundContext> deviceContext) {
  if (!session) {
    throw std::invalid_argument("Session cannot be null");
  }

  if (userId.isNull()) {
    throw std::invalid_argument("User ID cannot be null");
  }

  UserSessionPtr userSession = std::make_shared<UserSession>();
  userSession->userId = userId;
  userSession->deviceContext = std::move(deviceContext);

  if (sessionKey.size() != userSession->sessionKey.size()) {
    throw std::invalid_argument("Invalid session key size");
  }

  std::copy_n(sessionKey.data(), sessionKey.size(),
              userSession->sessionKey.data());

  const uuid cookieId = uuid::rand();
  userSession->cookieId = cookieId;

  session->modify<UserSessionPtr>(kAuthSessionKey,
                                  [&userSession](UserSessionPtr& sessionData) {
                                    sessionData = userSession;
                                  });

  return insertAuthCookie(userId, cookieId);
}

std::optional<drogon::Cookie> ikea400::SessionService::refreshAuthentication(
    const drogon::HttpRequestPtr& req, std::span<const uint8_t> deviceProof) {
  if (!req) {
    return std::nullopt;
  }

  UserSessionPtr userSession =
      req->session()->get<UserSessionPtr>(kAuthSessionKey);
  if (!userSession) {
    return std::nullopt;
  }

  std::scoped_lock lock(userSession->mutex);

  // Device-bound sessions require proof-of-possession before rotation.
  if (userSession->deviceContext) {
    if (!userSession->deviceContext->consumeChallenge(deviceProof)) {
      LOG_INFO << "RefreshAuthentication failed: Invalid device proof";
      return std::nullopt;
    }
  }

  m_authCookieCache->erase(userSession->cookieId);

  userSession->cookieId = uuid::rand();
  return insertAuthCookie(userSession->userId, userSession->cookieId);
}

std::optional<SecureArray<uint8_t, 32>>
ikea400::SessionService::getNextChallenge(const drogon::HttpRequestPtr& req) {
  if (!req) {
    return std::nullopt;
  }

  UserSessionPtr userSession =
      req->session()->get<UserSessionPtr>(kAuthSessionKey);
  if (!userSession) {
    return std::nullopt;
  }

  std::scoped_lock lock(userSession->mutex);

  if (!userSession->deviceContext) {
    return std::nullopt;
  }

  userSession->deviceContext->refreshChallenge();
  return userSession->deviceContext->getChallenge();
}

void SessionService::onBeginning() noexcept {
  const Json::Value config = getAuthCookieConfig();

  m_cookieKey = config.get("cookie_key", "auth_cookie").asString();
  m_cookieTimeout = sanitizeTimeout(
      config.get("timeout", kDefaultCookieTimeoutSeconds).asInt64());
  m_cookieSameSite = drogon::Cookie::convertString2SameSite(
      config.get("same_site", "Null").asString());

  const auto [wheelNum, bucketNum] = computeCookieCacheLayout(m_cookieTimeout);
  m_authCookieCache.emplace(drogon::app().getLoop(), 1.f, wheelNum, bucketNum);
}

drogon::Cookie ikea400::SessionService::insertAuthCookie(uuid userId,
                                                         uuid cookieId) {
  if (userId.isNull()) {
    throw std::invalid_argument("User ID cannot be null");
  }

  if (cookieId.isNull()) {
    throw std::runtime_error("Failed to generate a valid cookie ID");
  }

  const AuthCookieData cookieData{
      .userId = userId,
      .expireTime = utils::unix_seconds() + m_cookieTimeout,
  };

  m_authCookieCache->insert(cookieId, cookieData, m_cookieTimeout);

  drogon::Cookie cookie{m_cookieKey, cookieId.toString()};
  cookie.setHttpOnly(true);
  cookie.setSecure(true);
  cookie.setSameSite(m_cookieSameSite);
  cookie.setPath("/");

  return cookie;
}
