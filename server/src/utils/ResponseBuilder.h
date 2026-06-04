#pragma once
#include <drogon/HttpRequest.h>
#include <drogon/HttpResponse.h>
#include <drogon/HttpTypes.h>
#include <drogon/Session.h>
#include <dto/response_base.h>
#include <trantor/utils/Logger.h>

#include <algorithm>
#include <cstdint>
#include <functional>
#include <glaze/json/write.hpp>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

namespace ikea400 {
class ResponseBuilder;
using ResponseBuilderPtr = std::shared_ptr<ResponseBuilder>;

class ResponseBuilder {
 public:
  ResponseBuilder(const drogon::HttpRequestPtr& request);
  ~ResponseBuilder() noexcept;

  drogon::HttpResponsePtr internalServerError(
      std::string reason = "Internal Server Error",
      const std::function<void()>& errorCallback = nullptr);

  drogon::HttpResponsePtr badRequest(
      std::string reason = "Bad Request",
      const std::function<void()>& errorCallback = nullptr);

  drogon::HttpResponsePtr unauthorized(
      std::string reason = "Unauthorized",
      const std::function<void()>& errorCallback = nullptr);

  drogon::HttpResponsePtr forbidden(
      std::string reason = "Forbidden",
      const std::function<void()>& errorCallback = nullptr);

  drogon::HttpResponsePtr notFound(
      std::string reason = "Not Found",
      const std::function<void()>& errorCallback = nullptr);

  drogon::HttpResponsePtr failure(
      std::string reason, drogon::HttpStatusCode status,
      const std::function<void()>& errorCallback = nullptr);

  drogon::HttpResponsePtr success(
      drogon::HttpStatusCode status = drogon::k200OK,
      const std::function<void()>& errorCallback = nullptr);

  template <class T>
  drogon::HttpResponsePtr success(
      ResponseMessage<T>& value, drogon::HttpStatusCode status = drogon::k200OK,
      const std::function<void()>& errorCallback = nullptr) {
    value.success = true;
    value.http_status = static_cast<int32_t>(status);
    return createResponse(value, status, errorCallback);
  }

  static ResponseBuilderPtr get(const drogon::HttpRequestPtr& request);

 private:
  drogon::HttpResponsePtr createResponse(
      std::string&& body, drogon::HttpStatusCode status,
      const std::function<void()>& errorCallback);

  template <class T>
  drogon::HttpResponsePtr createResponse(
      const ResponseMessage<T>& message, drogon::HttpStatusCode status,
      const std::function<void()>& errorCallback) {
    std::string json_string;
    bool failed = false;

    auto ec = glz::write_json(message, json_string);
    if (ec) {
      LOG_ERROR << "ResponseBuilder '" << m_logName
                << "' failed to serialize response message. Err:"
                << ec.custom_error_message;
      if (errorCallback) errorCallback();
      // Fallback in case of serialization error
      json_string = kFallbackJsonErrorMessage;
      status = drogon::k500InternalServerError;
      failed = true;
    }

    return createResponse(std::move(json_string), status,
                          failed ? nullptr : errorCallback);
  }

 private:
  std::string m_logName;
  drogon::SessionPtr m_session;
  bool m_responseBuilt = false;
  bool m_error = false;

  static const inline std::string kAttributeName = "response_builder";
  static constexpr const inline std::string_view kFallbackJsonErrorMessage =
      R"({"success":false,"http_status":500,"error":"Internal Server Error"})";
};
}  // namespace ikea400