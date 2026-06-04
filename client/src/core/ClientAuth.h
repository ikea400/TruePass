#pragma once
#include <dto/auth_dto.h>
#include <dto/response_base.h>
#include <qlogging.h>
#include <qobject.h>
#include <qstring.h>
#include <qtmetamacros.h>

#include <cstdint>
#include <optional>
#include <span>
#include <string>
// #include<opaque++.h>

#include <QObject>
#include <QString>
#include <array>
#include <memory>
#include <vector>

#include "../network/RequestError.h"
#include "DeviceBinder.h"
#include "MasterKeyManager.h"

class ServerSession;

namespace opaque {
class OpaqueClient;
}

class ClientAuth : public QObject {
  Q_OBJECT
 public:
  ClientAuth(ServerSession* serverSession);

  bool isLoggedIn() const { return m_isLoggedIn; }
  bool isRefreshing() const { return m_isRefreshing; }

  void registerUser(const QString& username, const QString& email,
                    const QString& password) noexcept;

  void loginUser(const QString& identifier, const QString& password) noexcept;

  void refreshAuthentication() noexcept;

  MasterKeyManager* getMasterKeyManager() { return &m_masterKeyManager; }

 signals:
  void registrationSuccess();
  void registrationFailed(const QString& errorMessage);
  void loginSuccess();
  void loginFailed(const QString& errorMessage);
  void authenticationRefreshed();
  void authenticationRefreshFailed(const QString& errorMessage);

 private:
  struct RegistrationRequestData {
    std::string username;
    std::string email;
  };

  struct LoginRequestData {
    std::string identifier;
  };

  struct StartLoginCompute {
    std::vector<uint8_t> loginData;
    std::optional<std::vector<uint8_t>> devicePublicKey;
  };

  struct FinishLoginCompute {
    std::vector<uint8_t> loginData;
    std::optional<std::vector<uint8_t>> deviceSignature;
  };

  void startRegistration();
  void startLogin();

  void onRegisterStartComputed(const std::vector<uint8_t>& registrationData);
  void onLoginStartComputed(const StartLoginCompute& data);

  void onRegistrationStartReponse(
      RequestError error,
      const ikea400::ResponseMessage<ikea400::StartRegisterResponse>& result);
  void onLoginStartResponse(
      RequestError error,
      const ikea400::ResponseMessage<ikea400::StartLoginResponse>& result);

  void onRegisterFinishComputed(const std::vector<uint8_t>& registrationData);
  void onLoginFinishComputed(const FinishLoginCompute& data);

  void onRegisterFinishResponse(RequestError error,
                                const ikea400::ResponseMessage<void>& result);
  void onLoginFinishResponse(
      RequestError error,
      const ikea400::ResponseMessage<ikea400::FinishLoginResponse>& result);

  void onGetChallengeResponse(
      RequestError error,
      const ikea400::ResponseMessage<ikea400::GetChallengeResponse>& result);

  void onRefreshComputed(std::optional<std::vector<uint8_t>> deviceSignature);
  void onRefreshResponse(RequestError error,
                         const ikea400::ResponseMessage<void>& result);

  bool generateAccountKey(const std::span<const uint8_t> exportKey);
  bool loadAccountKey(const std::span<const uint8_t> exportKey,
                      const std::span<const uint8_t> protectedAccountKey);

 private:
  void onRegisterFailed(const QString& errorMessage) {
    qWarning() << "Registration failed: " << errorMessage;
    m_opaqueClient.reset();
    m_registrationRequestData.reset();
    emit registrationFailed(errorMessage);
  }

  void onLoginSuccess() {
    qInfo() << "Login successful.";
    m_isLoggedIn = true;
    emit loginSuccess();
  }

  void onLoginFailed(const QString& errorMessage) {
    qWarning() << "Login failed: " << errorMessage;
    m_opaqueClient.reset();
    m_loginRequestData.reset();
    m_deviceBinder.reset();
    emit loginFailed(errorMessage);
  }

  void onAuthenticationRefreshFailed(const QString& errorMessage) {
    qWarning() << "Authentication refresh failed: " << errorMessage;
    m_isRefreshing = false;
    emit authenticationRefreshFailed(errorMessage);
  }

  void onAuthenticationRefreshed() {
    qInfo() << "Authentication refreshed successfully.";
    m_isRefreshing = false;
    emit authenticationRefreshed();
  }

 private:
  MasterKeyManager m_masterKeyManager;
  std::unique_ptr<opaque::OpaqueClient> m_opaqueClient;
  std::unique_ptr<RegistrationRequestData> m_registrationRequestData;
  std::unique_ptr<LoginRequestData> m_loginRequestData;
  std::unique_ptr<DeviceBinder> m_deviceBinder;
  ServerSession* m_serverSession;
  bool m_isLoggedIn = false;
  bool m_isRefreshing = false;
};