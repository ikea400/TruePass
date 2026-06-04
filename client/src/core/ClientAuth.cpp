#include "ClientAuth.h"

#include <dto/auth_dto.h>
#include <dto/response_base.h>
#include <opaque++.h>
#include <qdebug.h>
#include <qfuturewatcher.h>
#include <qlogging.h>
#include <qobject.h>
#include <qstring.h>
#include <qtconcurrentrun.h>
#include <qtmetamacros.h>
#include <utils/BinEncoding.h>

#include <cstdint>
#include <exception>
#include <functional>
#include <iostream>
#include <memory>
#include <optional>
#include <span>
#include <utility>
#include <vector>

#include "../network/RequestError.h"
#include "../network/ServerSession.h"
#include "DeviceBinder.h"

using namespace ikea400;
using namespace opaque;

ClientAuth::ClientAuth(ServerSession* serverSession)
    : m_serverSession(serverSession) {}

void ClientAuth::registerUser(const QString& username, const QString& email,
                              const QString& password) noexcept {
  qDebug("Attempting to register user: %s", qPrintable(username));

  if (m_opaqueClient) {
    qWarning() << "Registration/Login already in progress.";
    emit registrationFailed("Registration/Login already in progress.");
    return;
  }

  m_registrationRequestData =
      std::make_unique<RegistrationRequestData>(RegistrationRequestData{
          .username = username.toStdString(), .email = email.toStdString()});

  m_opaqueClient = std::make_unique<OpaqueClient>(
      password.toStdString(), m_registrationRequestData->username, "", "");

  startRegistration();
}

void ClientAuth::loginUser(const QString& identifier,
                           const QString& password) noexcept {
  qDebug("Attempting to log in user: %s", qPrintable(identifier));

  if (m_opaqueClient) {
    qWarning() << "Registration/Login already in progress.";
    emit loginFailed("Registration/Login already in progress.");
    return;
  }

  m_loginRequestData = std::make_unique<LoginRequestData>(
      LoginRequestData{.identifier = identifier.toStdString()});

  m_opaqueClient = std::make_unique<OpaqueClient>(
      password.toStdString(), m_loginRequestData->identifier, "", "");

  m_deviceBinder =
      DeviceBinder::create(identifier.toStdString()).value_or(nullptr);
  if (!m_deviceBinder) {
    qWarning() << "Device binding is not available. Login may fail if server "
                  "requires it.";
  }

  startLogin();
}

void ClientAuth::refreshAuthentication() noexcept {
  qDebug("Refreshing authentication for user.");
  if (!m_isLoggedIn) {
    onAuthenticationRefreshFailed("User is not logged in.");
    return;
  }

  m_isRefreshing = true;

  // Device binder mean we needs to grab a new challenge from the server and
  // sign it to refresh the session
  if (m_deviceBinder) {
    m_serverSession->getRequest<GetChallengeResponse>(
        "/api/v1/auth/challenge",
        std::bind(&ClientAuth::onGetChallengeResponse, this,
                  std::placeholders::_1, std::placeholders::_2));
  } else {
    onRefreshComputed(std::nullopt);
  }
}

void ClientAuth::startRegistration() {
  auto future = QtConcurrent::run(
      [this]() { return m_opaqueClient->startRegistration(); });

  auto watcher = new QFutureWatcher<std::vector<uint8_t>>(this);

  connect(watcher, &QFutureWatcher<std::vector<uint8_t>>::finished, this,
          [this, watcher]() {
            try {
              std::vector<uint8_t> resultData = watcher->result();
              onRegisterStartComputed(resultData);
            } catch (const std::exception& e) {
              onRegisterFailed("Registration Start Error:" +
                               QString::fromStdString(e.what()));
            }

            watcher->deleteLater();  // Clean up
          });

  watcher->setFuture(future);
}

void ClientAuth::startLogin() {
  auto future = QtConcurrent::run([this]() {
    return StartLoginCompute{
        m_opaqueClient->startLogin(),
        m_deviceBinder
            ? std::optional(m_deviceBinder->getPublicKey().value_or({}))
            : std::nullopt};
  });

  auto watcher = new QFutureWatcher<StartLoginCompute>(this);

  connect(watcher, &QFutureWatcher<StartLoginCompute>::finished, this,
          [this, watcher]() {
            try {
              StartLoginCompute result = watcher->result();
              onLoginStartComputed(result);
            } catch (const std::exception& e) {
              onLoginFailed("Login Start Compute Error:" +
                            QString::fromStdString(e.what()));
            }
            watcher->deleteLater();  // Clean up
          });

  watcher->setFuture(future);
}

void ClientAuth::onRegisterStartComputed(
    const std::vector<uint8_t>& registrationData) {
  StartRegisterRequest request{
      .username = m_registrationRequestData->username,
      .email = m_registrationRequestData->email,
      .registrationStart = std::span(registrationData)};

  m_serverSession->postRequest<StartRegisterResponse>(
      "/api/v1/auth/register/start", request,
      std::bind(&ClientAuth::onRegistrationStartReponse, this,
                std::placeholders::_1, std::placeholders::_2));
}

void ClientAuth::onLoginStartComputed(const StartLoginCompute& data) {
  StartLoginRequest request{.identifier = m_loginRequestData->identifier,
                            .loginRequest = std::span(data.loginData),
                            .devicePublicKey = data.devicePublicKey};
  m_serverSession->postRequest<StartLoginResponse>(
      "/api/v1/auth/login/start", request,
      std::bind(&ClientAuth::onLoginStartResponse, this, std::placeholders::_1,
                std::placeholders::_2));
}

void ClientAuth::onRegistrationStartReponse(
    RequestError error, const ResponseMessage<StartRegisterResponse>& result) {
  if (error != RequestError::None) {
    onRegisterFailed(QString::fromStdString(
        result.error.value_or(requestErrorToString(error))));
    return;
  }

  if (!result.data.has_value()) {
    onRegisterFailed("Failed to start registration: No data in response");
    return;
  }

  std::vector<uint8_t> registrationResponse;
  registrationResponse.assign_range(result.data->registrationResponse.value());

  auto future =
      QtConcurrent::run([this, data = std::move(registrationResponse)]() {
        return m_opaqueClient->finishRegistration(data);
      });

  auto watcher = new QFutureWatcher<std::vector<uint8_t>>(this);
  connect(watcher, &QFutureWatcher<std::vector<uint8_t>>::finished, this,
          [this, watcher]() {
            try {
              // result() will re-throw the exception if the worker failed
              std::vector<uint8_t> resultData = watcher->result();

              onRegisterFinishComputed(resultData);

            } catch (const std::exception& e) {
              // Handle the error (e.g., show a message box)
              qWarning() << "Registration Start Error:" << e.what();

              onRegisterFailed("Registration Start Error:" +
                               QString::fromStdString(e.what()));
            }

            watcher->deleteLater();  // Clean up
          });

  watcher->setFuture(future);
}

void ClientAuth::onLoginStartResponse(
    RequestError error,
    const ikea400::ResponseMessage<ikea400::StartLoginResponse>& result) {
  if (error != RequestError::None) {
    onLoginFailed(QString::fromStdString(
        result.error.value_or(requestErrorToString(error))));
    return;
  }

  if (!result.data.has_value()) {
    onLoginFailed("Failed to start login: No data in response");
    return;
  }
  std::vector<uint8_t> loginResponse;
  loginResponse.assign_range(result.data->loginResponse.value());

  auto future = QtConcurrent::run([this, data = std::move(loginResponse),
                                   challenge = result.data->deviceChallenge]() {
    try {
      return FinishLoginCompute{
          m_opaqueClient->finishLogin(data),
          m_deviceBinder && challenge.has_value()
              ? std::optional(
                    m_deviceBinder->signChallenge(challenge.value().data)
                        .value_or({}))
              : std::nullopt};
    } catch (const opaque::InvalidLoginException&) {
      return FinishLoginCompute{};
    }
  });

  auto watcher = new QFutureWatcher<FinishLoginCompute>(this);
  connect(watcher, &QFutureWatcher<FinishLoginCompute>::finished, this,
          [this, watcher]() {
            try {
              // result() will re-throw the exception if the worker failed
              FinishLoginCompute resultData = watcher->result();
              if (resultData.loginData.empty()) {
                qWarning() << "Invalid username/password";
                onLoginFailed("Invalid username/password");
                return;
              }
              onLoginFinishComputed(resultData);
            } catch (const std::exception& e) {
              // Handle the error (e.g., show a message box)
              qWarning() << "Login Finish Error:" << e.what();
              onLoginFailed("Login Finish Error:" +
                            QString::fromStdString(e.what()));
            }
            watcher->deleteLater();  // Clean up
          });
  watcher->setFuture(future);
}

void ClientAuth::onRegisterFinishComputed(
    const std::vector<uint8_t>& registrationData) {
  std::cout << bin::hex::encode(registrationData) << "\n";

  const std::span<const uint8_t> exportKey = m_opaqueClient->getExportKey();

  if (!generateAccountKey(exportKey)) {
    onRegisterFailed("Failed to generate account key");
    return;
  }

  FinishRegisterRequest request{
      .registrationRecord = std::span(registrationData),
      .protectedAccountKey = m_masterKeyManager.exportMasterKey()};

  if (request.protectedAccountKey.data.empty()) {
    onRegisterFailed("Failed to export master key");
    return;
  }

  m_serverSession->postRequest<void>(
      "/api/v1/auth/register/finish", request,
      std::bind(&ClientAuth::onRegisterFinishResponse, this,
                std::placeholders::_1, std::placeholders::_2));
}

void ClientAuth::onLoginFinishComputed(const FinishLoginCompute& data) {
  FinishLoginRequest request{.loginRequest = std::span(data.loginData),
                             .deviceProof = data.deviceSignature};

  m_serverSession->postRequest<FinishLoginResponse>(
      "/api/v1/auth/login/finish", request,
      std::bind(&ClientAuth::onLoginFinishResponse, this, std::placeholders::_1,
                std::placeholders::_2));
}

void ClientAuth::onRegisterFinishResponse(
    RequestError error, const ikea400::ResponseMessage<void>& result) {
  if (error != RequestError::None) {
    onRegisterFailed(QString::fromStdString(
        result.error.value_or(requestErrorToString(error))));
    return;
  }

  qInfo(
      "Registration successful for user: %s",
      qPrintable(QString::fromStdString(m_registrationRequestData->username)));

  m_opaqueClient.reset();
  m_registrationRequestData.reset();

  emit registrationSuccess();
}

void ClientAuth::onLoginFinishResponse(
    RequestError error,
    const ikea400::ResponseMessage<ikea400::FinishLoginResponse>& result) {
  if (error != RequestError::None) {
    onLoginFailed(QString::fromStdString(
        result.error.value_or(requestErrorToString(error))));
    return;
  }

  if (!result.data.has_value()) {
    onLoginFailed("Failed to finish login: No data in response");
    return;
  }

  const FinishLoginResponse& loginResult = result.data.value();
  if (!loadAccountKey(m_opaqueClient->getExportKey(),
                      loginResult.protectedAccountKey.data)) {
    qDebug(
        "Failed to load account key with provided export key and protected "
        "account key");
    onLoginFailed("Failed to load account key");
    return;
  }

  onLoginSuccess();
}

void ClientAuth::onGetChallengeResponse(
    RequestError error,
    const ikea400::ResponseMessage<ikea400::GetChallengeResponse>& result) {
  if (error != RequestError::None) {
    onAuthenticationRefreshFailed(QString::fromStdString(
        result.error.value_or(requestErrorToString(error))));
    return;
  }

  if (!result.data.has_value()) {
    onAuthenticationRefreshFailed(
        "Failed to get challenge: No data in response");
    return;
  }

  auto future =
      QtConcurrent::run([this, challenge = result.data.value().data]() {
        std::optional<std::vector<uint8_t>> deviceSignature;
        if (m_deviceBinder) {
          if (auto result = m_deviceBinder->signChallenge(challenge)) {
            deviceSignature =
                std::move(result).value();
          } else {
            qDebug() << "Failed to sign challenge for authentication refresh:"
                     << (int)result.error();
          }
        }
        return deviceSignature;
      });

  auto watcher = new QFutureWatcher<std::optional<std::vector<uint8_t>>>(this);
  connect(
      watcher, &QFutureWatcher<std::optional<std::vector<uint8_t>>>::finished,
      this, [this, watcher]() {
        std::optional<std::vector<uint8_t>> deviceSignature = watcher->result();
        if (deviceSignature && (!deviceSignature || deviceSignature->empty())) {
          qDebug()
              << "Device binder failed to sign challenge for authentication "
                 "refresh";
          onAuthenticationRefreshFailed(
              "Device binder failed to sign challenge for authentication "
              "refresh");
        }
        onRefreshComputed(deviceSignature);
        watcher->deleteLater();  // Clean up
      });

  watcher->setFuture(future);
}

void ClientAuth::onRefreshComputed(
    std::optional<std::vector<uint8_t>> deviceSignature) {
  qDebug() << "Proof :" << bin::hex::encode(deviceSignature.value_or({}));

  m_serverSession->postRequest<void>(
      "/api/v1/auth/refresh", RefreshRequest{.deviceProof = deviceSignature},
      std::bind(&ClientAuth::onRefreshResponse, this, std::placeholders::_1,
                std::placeholders::_2),
      RequestFlags{.authRefreshAttempted = true});
}

void ClientAuth::onRefreshResponse(
    RequestError error, const ikea400::ResponseMessage<void>& result) {
  if (error != RequestError::None) {
    qDebug("Authentication refresh failed with error: %s",
           requestErrorToString(error).c_str());
    onAuthenticationRefreshFailed(QString::fromStdString(
        result.error.value_or(requestErrorToString(error))));
    return;
  }

  onAuthenticationRefreshed();
}

bool ClientAuth::generateAccountKey(const std::span<const uint8_t> exportKey) {
  if (exportKey.size() != opaque::kExportKeySize) {
    qDebug("Invalid export key size: %d", static_cast<int>(exportKey.size()));
    return false;
  }

  if (!m_masterKeyManager.deriveMasterKey(
          m_opaqueClient->getExportKey().subspan<0, opaque::kExportKeySize>(),
          m_registrationRequestData->username)) {
    qDebug("Failed to derive master key");
    return false;
  }

  if (!m_masterKeyManager.generateNewAccountKey()) {
    qDebug("Failed to generate new account key");
    return false;
  }
  return true;
}

bool ClientAuth::loadAccountKey(
    const std::span<const uint8_t> exportKey,
    const std::span<const uint8_t> protectedAccountKey) {
  if (exportKey.size() != opaque::kExportKeySize) {
    qDebug("Invalid export key size: %d", static_cast<int>(exportKey.size()));
    return false;
  }

  if (!m_masterKeyManager.deriveMasterKey(
          m_opaqueClient->getExportKey().subspan<0, opaque::kExportKeySize>(),
          m_loginRequestData->identifier)) {
    qDebug("Failed to derive master key");
    return false;
  }

  return m_masterKeyManager.loadAccountKeyFromBlob(protectedAccountKey);
}
