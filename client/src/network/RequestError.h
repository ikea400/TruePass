#pragma once

#include <QNetworkReply>

enum class RequestError {
  None,
  NetworkError,
  InternalServerError,
  BadRequest,
  InvalidResponse,
  ParseError,
  SerializationError,
  Unauthorized,
  Timeout,
  NotFound,
  Unknown,
};

inline static RequestError mapNetworkError(QNetworkReply::NetworkError networkError) {
  switch (networkError) {
    case QNetworkReply::NoError:
      return RequestError::None;
    case QNetworkReply::ProtocolInvalidOperationError:
    case QNetworkReply::ContentOperationNotPermittedError:
    case QNetworkReply::UnknownContentError:
      return RequestError::BadRequest;
    case QNetworkReply::AuthenticationRequiredError:
      return RequestError::Unauthorized;
    case QNetworkReply::ContentAccessDenied:
      return RequestError::Unauthorized;
    case QNetworkReply::ContentNotFoundError:
      return RequestError::NotFound;
    case QNetworkReply::ProxyAuthenticationRequiredError:
    case QNetworkReply::ServiceUnavailableError:
    case QNetworkReply::ProtocolFailure:
    case QNetworkReply::ConnectionRefusedError:
    case QNetworkReply::RemoteHostClosedError:
    case QNetworkReply::HostNotFoundError:
    case QNetworkReply::TimeoutError:
      return RequestError::NetworkError;
    case QNetworkReply::InternalServerError:
    case QNetworkReply::UnknownServerError:
      return RequestError::InternalServerError;
    default:
      return RequestError::Unknown;
  }
}

static inline std::string requestErrorToString(RequestError error) {
  switch (error) {
    case RequestError::None:
      return "No Error";
    case RequestError::NetworkError:
      return "Network Error";
    case RequestError::InternalServerError:
      return "Internal Server Error";
    case RequestError::BadRequest:
      return "Bad Request";
    case RequestError::InvalidResponse:
      return "Invalid Response";
    case RequestError::ParseError:
      return "Parse Error";
    case RequestError::SerializationError:
      return "Serialization Error";
    case RequestError::Unauthorized:
      return "Unauthorized";
    case RequestError::Timeout:
      return "Timeout";
    case RequestError::NotFound:
      return "Not Found";
    case RequestError::Unknown:
    default:
      return "Unknown Error";
  }
}