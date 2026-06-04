#include "AuthController.h"

#include <drogon/CacheMap.h>
#include <drogon/HttpAppFramework.h>
#include <drogon/HttpResponse.h>
#include <drogon/HttpTypes.h>
#include <drogon/RequestStream.h>
#include <drogon/Session.h>
#include <drogon/utils/FunctionTraits.h>
#include <drogon/utils/coroutine.h>
#include <dto/auth_dto.h>
#include <dto/response_base.h>
#include <json/value.h>
#include <trantor/utils/Logger.h>
#include <utils/BinArray.h>
#include <utils/validation.h>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <exception>
#include <memory>
#include <span>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "../dao/UserDAO.h"
#include "../services/OpaqueService.h"
#include "../services/SessionService.h"
#include "../utils/RequestReader.h"
#include "../utils/ResponseBuilder.h"

using namespace drogon;
using namespace ikea400;

api::v1::Auth::Auth()
    : m_registrationCache(drogon::app().getLoop(), 1.f, 2, 100),
      m_loginCache(drogon::app().getLoop(), 1.f, 2, 100) {}

Task<HttpResponsePtr> api::v1::Auth::startRegister(HttpRequestPtr req) {
  LOG_TRACE << "StartRegister called";

  ResponseBuilderPtr builder = ikea400::ResponseBuilder::get(req);

  LOG_TRACE << req->getBody();

  try {
    const auto& session = req->session();
    removeRegistrationContext(session);

    auto registrationData = RequestReader::read<StartRegisterRequest>(*req);
    if (!registrationData) {
      LOG_ERROR << "Reading request data";
      co_return builder->badRequest("Invalid request data");
    }

    if (!validation::validateUsername(registrationData->username).isValid() ||
        !validation::validateEmail(registrationData->email).isValid()) {
      LOG_ERROR << "Validating request data";
      co_return builder->badRequest("Invalid username or email");
    }

    RegistrationContextPtr registrationContext = createRegistrationContext(
        session, registrationData->username, registrationData->email);

    std::vector<uint8_t> registrationResponse =
        registrationContext->opaque.startRegistration(
            registrationData->username,
            registrationData->registrationStart.data);

    if (registrationResponse.empty()) {
      LOG_ERROR << "Empty registration response from Opaque";
      co_return builder->badRequest("Invalid registration data");
    }

    ResponseMessage<StartRegisterResponse> response = {
        .data =
            StartRegisterResponse{
                .registrationResponse =
                    std::span<const uint8_t>(registrationResponse),
            },
        .success = true};

    co_return builder->success(response);
  } catch (const std::exception& e) {
    LOG_ERROR << "Error in StartRegister: " << e.what();
  }

  co_return builder->internalServerError();
}

Task<HttpResponsePtr> api::v1::Auth::finishRegister(HttpRequestPtr req) {
  ResponseBuilderPtr builder = ikea400::ResponseBuilder::get(req);

  LOG_TRACE << req->getBody();

  try {
    const auto& session = req->session();
    RegistrationContextPtr registrationContext =
        takeRegistrationContext(session);

    if (!registrationContext) {
      LOG_ERROR << "No registration context found for session";
      co_return builder->badRequest("No registration in progress");
    }

    auto request = RequestReader::read<FinishRegisterRequest>(*req);
    if (!request) {
      LOG_ERROR << "Failed to Reading request data";
      co_return builder->badRequest("Invalid request data");
    }

    std::vector<uint8_t> passwordFile =
        registrationContext->opaque.finishRegistration(
            request->registrationRecord.data);

    if (passwordFile.empty()) {
      LOG_ERROR << "Empty password file from Opaque";
      co_return builder->badRequest("Invalid registration data");
    }

    bool success = co_await dao::UserDao::instance()->createUser(
        registrationContext->username, registrationContext->email, passwordFile,
        request->protectedAccountKey.data);

    if (!success) {
      co_return builder->failure("Failed to create user", k409Conflict);
    }

    co_return builder->success();

  } catch (const std::exception& e) {
    LOG_ERROR << "Error in FinishRegister: " << e.what();
  }

  co_return builder->internalServerError();
}

Task<HttpResponsePtr> api::v1::Auth::startLogin(HttpRequestPtr req) {
  ResponseBuilderPtr builder = ikea400::ResponseBuilder::get(req);

  LOG_TRACE << req->getBody();

  try {
    const auto& session = req->session();
    removeLoginContext(session);

    auto request = RequestReader::read<StartLoginRequest>(*req);
    if (!request) {
      LOG_ERROR << "Failed to read login request";
      co_return builder->badRequest("Invalid request data");
    }

    std::optional<dao::AuthCredentials> authCredentials =
        co_await dao::UserDao::instance()->getAuthCredentialsByUsername(
            request->identifier);

    if (!authCredentials) {
      // To prevent username enumeration, we tread it as if the user exists but
      // the registration record is invalid.
      LOG_ERROR << "User not found: " << request->identifier;
      authCredentials = dao::AuthCredentials{.username = request->identifier,
                                             .passwordFile = {},
                                             .id = uuid::null()};
    }

    LoginContextPtr loginContext = createLoginContext(
        session, authCredentials->username, authCredentials->id);

    if (request->devicePublicKey.has_value()) {
      try {
        loginContext->deviceBound.emplace(request->devicePublicKey->data);
      } catch (const std::exception& e) {
        LOG_ERROR << "Failed to create device context: " << e.what();
        co_return builder->badRequest("Invalid device public key");
      }
    }

    std::vector<uint8_t> loginResponse = loginContext->opaque.startLogin(
        authCredentials->username, authCredentials->passwordFile,
        request->loginRequest.data);

    if (loginResponse.empty()) {
      LOG_ERROR << "Empty login response from Opaque";
      removeLoginContext(session);
      co_return builder->badRequest("Invalid login data");
    }

    ResponseMessage<StartLoginResponse> response = {
        .data =
            StartLoginResponse{
                .loginResponse = std::span<const uint8_t>(loginResponse),
                .deviceChallenge =
                    loginContext->deviceBound
                        ? std::optional<dto::DeviceBoundChallenge>(
                              loginContext->deviceBound->getChallenge())
                        : std::nullopt,
            },
        .success = true};

    co_return builder->success(response);

  } catch (const std::exception& e) {
    LOG_ERROR << "Error in StartLogin: " << e.what();
  }

  co_return builder->internalServerError();
}

Task<HttpResponsePtr> api::v1::Auth::finishLogin(HttpRequestPtr req) {
  ResponseBuilderPtr builder = ikea400::ResponseBuilder::get(req);

  LOG_TRACE << req->getBody();

  try {
    const auto& session = req->session();
    LoginContextPtr loginContext = takeLoginContext(session);
    if (!loginContext) {
      LOG_INFO << "No login context found for session";
      co_return builder->badRequest("No login in progress");
    }

    if (loginContext->userId.isNull()) {
      LOG_ERROR << "User ID is null for login context";
      co_return builder->internalServerError("Failed to retrieve user data");
    }

    auto request = RequestReader::read<FinishLoginRequest>(*req);
    if (!request) {
      LOG_INFO << "Failed to read login request";
      co_return builder->badRequest("Invalid request data");
    }

    if (!loginContext->opaque.finishLogin(request->loginRequest.data) ||
        !loginContext->isRealLogin) {
      LOG_INFO << "Login failed in Opaque";
      // Client should have see that the login failed and not  have sent the
      // finish login request. But we treat it as an unauthorized error instead
      // of a bad request to prevent login flow enumeration.
      co_return builder->unauthorized();
    }

    if (loginContext->deviceBound) {
      if (!request->deviceProof.has_value()) {
        LOG_INFO << "Device response missing for device bound login";
        co_return builder->badRequest(
            "Device response required for device bound login");
      }
      if (!loginContext->deviceBound->consumeChallenge(
              request->deviceProof->data)) {
        LOG_INFO << "Device response verification failed";
        co_return builder->unauthorized("Device verification failed");
      }
    }

    std::vector<uint8_t> protectedAccountKey =
        co_await dao::UserDao::instance()->getProtectedAccountKeyByUserId(
            loginContext->userId);

    if (protectedAccountKey.empty()) {
      LOG_ERROR << "Protected account key not found for user: "
                << loginContext->userId.toString();
      co_return builder->internalServerError("Failed to retrieve account data");
    }

    drogon::Cookie cookie = SessionService::instance().authenticate(
        session, loginContext->userId, loginContext->opaque.getSessionKey(),
        std::move(loginContext->deviceBound));

    ResponseMessage<FinishLoginResponse> response = {
        .data =
            FinishLoginResponse{
                .protectedAccountKey =
                    std::span<const uint8_t>(protectedAccountKey),
                .userId = loginContext->userId,
            },
        .success = true};
    auto httpResponse = builder->success(response);
    httpResponse->addCookie(std::move(cookie));
    co_return httpResponse;
  } catch (const std::exception& e) {
    LOG_ERROR << "Error in FinishLogin: " << e.what();
  }

  co_return builder->internalServerError();
}

void api::v1::Auth::logout(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback) {
  ResponseBuilderPtr builder = ikea400::ResponseBuilder::get(req);

  try {
    const auto& session = req->session();
    removeRegistrationContext(session);
    removeLoginContext(session);

    SessionService::instance().deauthenticate(session);
    callback(builder->success());
  } catch (const std::exception& e) {
    LOG_ERROR << "Error in Logout: " << e.what();
    callback(builder->internalServerError());
  }
}

void api::v1::Auth::getChallenge(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback) {
  ResponseBuilderPtr builder = ikea400::ResponseBuilder::get(req);

  try {
    auto challenge = SessionService::instance().getNextChallenge(req);
    if (!challenge) {
      callback(builder->badRequest("Failed to generate challenge"));
      return;
    }

    ResponseMessage<GetChallengeResponse> response = {
        .data = challenge->asArraySpan(), .success = true};

    callback(builder->success(response));
  } catch (const std::exception& e) {
    LOG_ERROR << "Error in GetChallenge: " << e.what();
    callback(builder->internalServerError());
  }
}

void api::v1::Auth::refresh(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback) {
  ResponseBuilderPtr builder = ikea400::ResponseBuilder::get(req);

  try {
    auto request = RequestReader::read<RefreshRequest>(*req);
    if (!request) {
      LOG_INFO << "Failed to read refresh request";
      callback(builder->badRequest("Invalid request data"));
      return;
    }

    std::span<const uint8_t> deviceProof;
    if (request->deviceProof.has_value()) {
      deviceProof = request->deviceProof->data;
    }

    auto cookieOpt =
        SessionService::instance().refreshAuthentication(req, deviceProof);
    if (!cookieOpt) {
      LOG_INFO << "Authentication refresh failed";
      callback(builder->unauthorized("Authentication refresh failed"));
      return;
    }

    const auto response = builder->success();
    response->addCookie(*std::move(cookieOpt));

    callback(response);

  } catch (const std::exception& e) {
    LOG_ERROR << "Error in Refresh: " << e.what();
    callback(builder->internalServerError());
  }
}

RegistrationContextPtr api::v1::Auth::createRegistrationContext(
    const SessionPtr& session, const std::string& username,
    const std::string& email) {
  RegistrationContextPtr context = std::make_shared<RegistrationContext>(
      RegistrationContext{.opaque = OpaqueService::instance().createContext(),
                          .username = username,
                          .email = email});

  m_registrationCache.modify(
      session->sessionId(),
      [&context](RegistrationContextPtr& ptr) { ptr = context; },
      kContextTimeoutSeconds);

  return context;
}

RegistrationContextPtr api::v1::Auth::takeRegistrationContext(
    const SessionPtr& session) {
  RegistrationContextPtr context;
  if (m_registrationCache.findAndFetch(session->sessionId(), context))
    removeRegistrationContext(session);

  return context;
}

void api::v1::Auth::removeRegistrationContext(const SessionPtr& session) {
  m_registrationCache.erase(session->sessionId());
}

LoginContextPtr api::v1::Auth::createLoginContext(const SessionPtr& session,
                                                  const std::string& username,
                                                  const uuid& userId) {
  LoginContextPtr context = std::make_shared<LoginContext>(
      LoginContext{.opaque = OpaqueService::instance().createContext(),
                   .username = username,
                   .userId = userId,
                   .isRealLogin = true});

  m_loginCache.modify(
      session->sessionId(), [&context](LoginContextPtr& ptr) { ptr = context; },
      kContextTimeoutSeconds);

  return context;
}

LoginContextPtr api::v1::Auth::takeLoginContext(const SessionPtr& session) {
  LoginContextPtr context;
  if (m_loginCache.findAndFetch(session->sessionId(), context))
    removeLoginContext(session);

  return context;
}

void api::v1::Auth::removeLoginContext(const SessionPtr& session) {
  m_loginCache.erase(session->sessionId());
}
