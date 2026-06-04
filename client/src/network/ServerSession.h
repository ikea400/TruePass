#pragma once
#include <dto/response_base.h>

#include <QNetworkAccessManager>
#include <QObject>
#include <QUrl>
#include <algorithm>
#include <cstdio>
#include <functional>
#include <glaze/glaze.hpp>
#include <glaze/json/generic.hpp>
#include <glaze/json/prettify.hpp>
#include <glaze/json/read.hpp>
#include <glaze/json/write.hpp>
#include <memory>
#include <string>
#include <utility>

#include "../core/ClientAuth.h"
#include "RequestError.h"

struct RequestFlags {
  bool authRefreshAttempted = false;
};

class ServerSession : public QObject {
  Q_OBJECT

  enum class HttpMethod {
    Get,
    Post,
    Put,
  };

 public:
  ServerSession(QObject* parent = nullptr);
  ~ServerSession();

  void connectToServer(const QUrl& url);

  ClientAuth* authService() const { return m_authService.get(); }

  void postRequest(
      const QString& endpoint, const QByteArray& postData,
      std::function<void(RequestError, const QByteArray&)> callback,
      RequestFlags flags);

  void putRequest(const QString& endpoint, const QByteArray& putData,
                  std::function<void(RequestError, const QByteArray&)> callback,
                  RequestFlags flags);

  void getRequest(const QString& endpoint,
                  std::function<void(RequestError, const QByteArray&)> callback,
                  RequestFlags flags);

  template <typename U, typename T>
  void postRequest(
      const QString& endpoint, const T& message,
      std::function<void(RequestError, const ikea400::ResponseMessage<U>&)>
          callback,
      RequestFlags flags = {}) {
    QByteArray byteJsonString;
    auto ec = glz::write_json(message, byteJsonString);
    if (ec) {
      qWarning() << "Failed to serialize message to JSON";
      callback(RequestError::SerializationError, {});
      return;
    }

    postRequest(
        endpoint, byteJsonString,
        [this, cb = std::move(callback), flags](
            RequestError error, const QByteArray& responseData) {
          onResponseMessage(error, responseData, cb);
        },
        flags);
  }

  template <typename U, typename T>
  void putRequest(
      const QString& endpoint, const T& message,
      std::function<void(RequestError, const ikea400::ResponseMessage<U>&)>
          callback,
      RequestFlags flags = {}) {
    QByteArray byteJsonString;
    auto ec = glz::write_json(message, byteJsonString);
    if (ec) {
      qWarning() << "Failed to serialize message to JSON";
      callback(RequestError::SerializationError, {});
      return;
    }
    putRequest(
        endpoint, byteJsonString,
        [this, cb = std::move(callback)](RequestError error,
                                         const QByteArray& responseData) {
          onResponseMessage(error, responseData, cb);
        },
        flags);
  }

  template <typename T>
  void getRequest(
      const QString& endpoint,
      std::function<void(RequestError, const ikea400::ResponseMessage<T>&)>
          callback,
      RequestFlags flags = {}) {
    getRequest(
        endpoint,
        [this, cb = std::move(callback)](RequestError error,
                                         const QByteArray& responseData) {
          onResponseMessage(error, responseData, cb);
        },
        flags);
  }

 private:
  void onResponseMessage(
      QNetworkReply* reply, const QByteArray& data, HttpMethod method,
      const std::function<void(RequestError, const QByteArray&)>& callback,
      RequestFlags flags);

  template <typename T>
  void onResponseMessage(
      RequestError error, const QByteArray& responseData,
      const std::function<void(RequestError,
                               const ikea400::ResponseMessage<T>&)>& callback) {
    if (error != RequestError::None && responseData.isEmpty()) {
      qDebug("Request failed with error: %d", (int)error);
      callback(error, {});
      return;
    }

    const std::string response_str = responseData.toStdString();
    auto result = glz::read_json<ikea400::ResponseMessage<T>>(response_str);
    if (!result) {
      qWarning() << "Failed to parse response JSON";
      printf("%s\n", glz::prettify_json(response_str).c_str());
      qInfo() << (int)result.error().ec;
      qInfo() << result.error().custom_error_message;
      callback(error == RequestError::None ? RequestError::ParseError : error,
               {});
      return;
    }

    if (result->error) {
      qWarning() << "Error in response: "
                 << QString::fromStdString(*result->error);
    }

    callback(error, *result);
  }

 public slots:
  void onAuthTokenRefreshed();
  void onAuthTokenRefreshFailed(const QString& error);

 private:
  struct PendingRequest {
    std::function<void(RequestError, const QByteArray&)> callback;
    QString endpoint;
    QByteArray data;
    RequestFlags flags;
    HttpMethod method;
  };

  QUrl m_serverUrl;
  std::unique_ptr<QNetworkAccessManager> m_networkManager;
  std::unique_ptr<ClientAuth> m_authService;
  std::vector<PendingRequest> m_pendingRequests;
};