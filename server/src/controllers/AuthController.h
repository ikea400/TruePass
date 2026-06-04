#pragma once
#include <drogon/CacheMap.h>
#include <drogon/HttpController.h>
#include <drogon/HttpTypes.h>
#include <drogon/RequestStream.h>
#include <drogon/Session.h>
#include <drogon/utils/FunctionTraits.h>
#include <drogon/utils/coroutine.h>
#include <utils/uuid.h>

#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>

#include "../middlewares/SessionLockMiddleware.h"
#include "../models/DeviceBoundContext.h"
#include "../services/OpaqueService.h"

struct RegistrationContext {
  ikea400::OpaqueContext opaque;
  std::string username;
  std::string email;
};
using RegistrationContextPtr = std::shared_ptr<RegistrationContext>;

struct LoginContext {
  ikea400::OpaqueContext opaque;
  std::optional<ikea400::DeviceBoundContext> deviceBound;
  std::string username;
  ikea400::uuid userId;
  bool isRealLogin;
};
using LoginContextPtr = std::shared_ptr<LoginContext>;

namespace api::v1 {
using namespace drogon;

class Auth : public HttpController<Auth> {
 public:
  Auth();
  METHOD_LIST_BEGIN
  METHOD_ADD(Auth::startRegister, "/register/start", Post,
             k_sessionLockName);  // path is POST /api/v1/auth/register/start
  METHOD_ADD(Auth::finishRegister, "/register/finish", Post,
             k_sessionLockName);  // path is POST /api/v1/auth/register/finish
  METHOD_ADD(Auth::startLogin, "/login/start", Post,
             k_sessionLockName);  // path is POST /api/v1/auth/login/start
  METHOD_ADD(Auth::finishLogin, "/login/finish", Post,
             k_sessionLockName);  // path is POST /api/v1/auth/login/finish
  METHOD_ADD(Auth::logout, "/logout", Post,
             k_sessionLockName);  // path is POST /api/v1/auth/logout
  METHOD_ADD(Auth::getChallenge, "/challenge", Get,
             k_sessionLockName);  // path is GET /api/v1/auth/challenge
  METHOD_ADD(Auth::refresh, "/refresh", Post,
             k_sessionLockName);  // path is POST /api/v1/auth/refresh
  METHOD_LIST_END
 protected:
  Task<HttpResponsePtr> startRegister(HttpRequestPtr req);
  Task<HttpResponsePtr> finishRegister(HttpRequestPtr req);
  Task<HttpResponsePtr> startLogin(HttpRequestPtr req);
  Task<HttpResponsePtr> finishLogin(HttpRequestPtr req);
  void logout(const HttpRequestPtr& req,
              std::function<void(const HttpResponsePtr&)>&& callback);
  void getChallenge(const HttpRequestPtr& req,
                    std::function<void(const HttpResponsePtr&)>&& callback);
  void refresh(const HttpRequestPtr& req,
               std::function<void(const HttpResponsePtr&)>&& callback);

 private:
  RegistrationContextPtr createRegistrationContext(const SessionPtr& session,
                                                   const std::string& username,
                                                   const std::string& email);
  RegistrationContextPtr takeRegistrationContext(const SessionPtr& session);
  void removeRegistrationContext(const SessionPtr& session);

  LoginContextPtr createLoginContext(const SessionPtr& session,
                                     const std::string& username,
                                     const ikea400::uuid& userId);
  LoginContextPtr takeLoginContext(const SessionPtr& session);
  void removeLoginContext(const SessionPtr& session);

 private:
  drogon::CacheMap<std::string, RegistrationContextPtr> m_registrationCache;
  drogon::CacheMap<std::string, LoginContextPtr> m_loginCache;

  static inline const std::string k_sessionLockName =
      SessionLockMiddleware<Auth>::className();

  static constexpr inline const size_t kContextTimeoutSeconds =
      300;  // 5 minutes
};
}  // namespace api::v1