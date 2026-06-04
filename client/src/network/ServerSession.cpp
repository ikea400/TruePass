#include "ServerSession.h"

#include <QNetworkReply>

ServerSession::ServerSession(QObject* parent)
    : QObject(parent),
      m_networkManager(std::make_unique<QNetworkAccessManager>()),
      m_authService(std::make_unique<ClientAuth>(this)) {
  connect(m_authService.get(), &ClientAuth::authenticationRefreshed, this,
          &ServerSession::onAuthTokenRefreshed);
  connect(m_authService.get(), &ClientAuth::authenticationRefreshFailed, this,
          &ServerSession::onAuthTokenRefreshFailed);
}

ServerSession::~ServerSession() {}

void ServerSession::connectToServer(const QUrl& url) { m_serverUrl = url; }

void ServerSession::postRequest(
    const QString& endpoint, const QByteArray& postData,
    std::function<void(RequestError, const QByteArray&)> callback,
    RequestFlags flags) {
  QUrl url = m_serverUrl;
  url.setPath(endpoint);

  QNetworkRequest request(url);
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

  QNetworkReply* reply = m_networkManager->post(request, postData);
  if (!reply) {
    qWarning() << "Failed to create QNetworkReply in postRequest";
    callback(RequestError::NetworkError, {});
    return;
  }

  connect(reply, &QNetworkReply::finished, this,
          [this, reply, cb = std::move(callback), postData, flags]() {
            onResponseMessage(reply, postData, HttpMethod::Post, cb, flags);
          });
}

void ServerSession::putRequest(
    const QString& endpoint, const QByteArray& putData,
    std::function<void(RequestError, const QByteArray&)> callback,
    RequestFlags flags) {
  QUrl url = m_serverUrl;
  url.setPath(endpoint);

  QNetworkRequest request(url);
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

  QNetworkReply* reply = m_networkManager->put(request, putData);
  if (!reply) {
    qWarning() << "Failed to create QNetworkReply in putRequest";
    callback(RequestError::NetworkError, {});
    return;
  }

  connect(reply, &QNetworkReply::finished, this,
          [this, reply, cb = std::move(callback), putData, flags]() {
            onResponseMessage(reply, putData, HttpMethod::Put, cb, flags);
          });
}

void ServerSession::getRequest(
    const QString& endpoint,
    std::function<void(RequestError, const QByteArray&)> callback,
    RequestFlags flags) {
  QUrl url = m_serverUrl;
  url.setPath(endpoint);

  QNetworkRequest request(url);
  QNetworkReply* reply = m_networkManager->get(request);
  if (!reply) {
    qWarning() << "Failed to create QNetworkReply in getRequest";
    callback(RequestError::NetworkError, {});
    return;
  }

  connect(reply, &QNetworkReply::finished, this,
          [this, reply, cb = std::move(callback), flags]() {
            onResponseMessage(reply, {}, HttpMethod::Get, cb, flags);
          });
}

void ServerSession::onResponseMessage(
    QNetworkReply* reply, const QByteArray& data, HttpMethod method,
    const std::function<void(RequestError, const QByteArray&)>& callback,
    RequestFlags flags) {
  reply->deleteLater();

  QNetworkReply::NetworkError networkError = reply->error();
  RequestError requestError = mapNetworkError(networkError);
  if (networkError != QNetworkReply::NoError &&
      networkError < QNetworkReply::UnknownProxyError) {
    qWarning() << "Network error:" << reply->errorString();
    callback(requestError, {});
    return;
  }

  if (networkError == QNetworkReply::AuthenticationRequiredError &&
      m_authService->isLoggedIn() && !flags.authRefreshAttempted) {
    m_pendingRequests.emplace_back(
        PendingRequest{.callback = callback,
                       .endpoint = reply->request().url().path(),
                       .data = data,
                       .flags = flags,
                       .method = method});

    if (!m_authService->isRefreshing()) {
      qInfo() << "Authentication required. Attempting to refresh token.";
      m_authService->refreshAuthentication();
    } else {
      qInfo() << "Authentication token refresh already in progress. Waiting "
                 "for result.";
    }

    return;
  }

  const QByteArray responseData = reply->readAll();
  callback(requestError, responseData);
}

void ServerSession::onAuthTokenRefreshed() {
  qInfo() << "Authentication token refreshed successfully.";

  for (auto&& [callback, endpoint, postData, flags, method] :
       m_pendingRequests) {
    switch (method) {
      case HttpMethod::Post:
        postRequest(endpoint, postData, std::move(callback), flags);
        break;
      case HttpMethod::Put:
        putRequest(endpoint, postData, std::move(callback), flags);
        break;
      case HttpMethod::Get:
        getRequest(endpoint, std::move(callback), flags);
        break;
    }
  }
  m_pendingRequests.clear();
}

void ServerSession::onAuthTokenRefreshFailed(const QString& error) {
  qWarning() << "Authentication token refresh failed:" << error;

  for (const auto& [callback, endpoint, postData, flags, method] :
       m_pendingRequests) {
    callback(RequestError::Unauthorized, {});
  }
  m_pendingRequests.clear();
}
