#pragma once
#include <drogon/CacheMap.h>
#include <drogon/Cookie.h>
#include <drogon/RequestStream.h>
#include <drogon/Session.h>
#include <utils/SecureArray.h>
#include <utils/uuid.h>

#include <cstdint>
#include <optional>
#include <span>
#include <string>

#include "../models/DeviceBoundContext.h"
#include "../utils/StartupEntry.h"

namespace ikea400 {
class SessionService : public StartupEntry<SessionService> {
 public:
  static SessionService& instance() {
    static SessionService inst;
    return inst;
  }

  uuid getAuthenticatedUserId(const drogon::HttpRequestPtr& session);
  void deauthenticate(const drogon::SessionPtr& session);

  [[nodiscard]] drogon::Cookie authenticate(
      const drogon::SessionPtr& session, uuid userId,
      const std::span<const uint8_t> sessionKey,
      std::optional<ikea400::DeviceBoundContext> deviceContext);

  [[nodiscard]] std::optional<drogon::Cookie> refreshAuthentication(
      const drogon::HttpRequestPtr& req, std::span<const uint8_t> deviceProof);

  std::optional<SecureArray<uint8_t, 32>> getNextChallenge(
      const drogon::HttpRequestPtr& req);

 protected:
  void onBeginning() noexcept;

  drogon::Cookie insertAuthCookie(uuid userId, uuid cookieId);

 private:
  SessionService() = default;
  friend void startup_beginning(SessionService*) {
    SessionService::instance().onBeginning();
  }
  struct AuthCookieData {
    uuid userId{};
    std::int64_t expireTime{};
  };

  std::optional<drogon::CacheMap<uuid, AuthCookieData>> m_authCookieCache;
  std::string m_cookieKey;
  std::int64_t m_cookieTimeout;
  drogon::Cookie::SameSite m_cookieSameSite;

  static inline const std::string kAuthSessionKey = "user_auth";
};
}  // namespace ikea400