#include "ResponseBuilder.h"

#include <drogon/Attribute.h>
#include <drogon/HttpRequest.h>
#include <drogon/HttpResponse.h>
#include <drogon/HttpTypes.h>
#include <dto/response_base.h>
#include <trantor/utils/Logger.h>

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <utility>

using namespace drogon;
using namespace ikea400;

ikea400::ResponseBuilder::ResponseBuilder(
    const drogon::HttpRequestPtr& request) {
  m_logName = request->getPath();
}

ResponseBuilder::~ResponseBuilder() noexcept {
  LOG_ERROR_IF(!m_responseBuilt)
      << "ResponseBuilder '" << m_logName << "' did not build a response";
}

HttpResponsePtr ResponseBuilder::internalServerError(
    std::string reason, const std::function<void()>& errorCallback) {
  return failure(std::move(reason), k500InternalServerError, errorCallback);
}

HttpResponsePtr ResponseBuilder::badRequest(
    std::string reason, const std::function<void()>& errorCallback) {
  return failure(std::move(reason), k400BadRequest, errorCallback);
}

HttpResponsePtr ResponseBuilder::unauthorized(
    std::string reason, const std::function<void()>& errorCallback) {
  return failure(std::move(reason), k401Unauthorized, errorCallback);
}

HttpResponsePtr ResponseBuilder::forbidden(
    std::string reason, const std::function<void()>& errorCallback) {
  return failure(std::move(reason), k403Forbidden, errorCallback);
}

HttpResponsePtr ResponseBuilder::notFound(
    std::string reason, const std::function<void()>& errorCallback) {
  return failure(std::move(reason), k404NotFound, errorCallback);
}

HttpResponsePtr ResponseBuilder::failure(
    std::string reason, HttpStatusCode status,
    const std::function<void()>& errorCallback) {
  ResponseMessage<void> message = {.error = std::move(reason),
                                   .http_status = static_cast<int32_t>(status),
                                   .success = false};

  return createResponse(message, status, errorCallback);
}

HttpResponsePtr ResponseBuilder::success(HttpStatusCode status,
                                         const std::function<void()>& errorCallback) {
  ResponseMessage<void> message = {.http_status = static_cast<int32_t>(status),
                                   .success = true};
  return createResponse(message, status, errorCallback);
}

ResponseBuilderPtr ResponseBuilder::get(const HttpRequestPtr& request) {
  const AttributesPtr& attributes = request->getAttributes();
  if (const auto& builder =
          attributes->get<ResponseBuilderPtr>(kAttributeName)) {
    return builder;
  }

  ResponseBuilderPtr builder = std::make_shared<ResponseBuilder>(request);
  attributes->insert(kAttributeName, builder);
  return builder;
}

HttpResponsePtr ResponseBuilder::createResponse(
    std::string&& body, HttpStatusCode status,
    const std::function<void()>&) {
  if (m_responseBuilt) {
    LOG_ERROR << "ResponseBuilder '" << m_logName
              << "' attempted to build multiple responses";
  }

  auto response = HttpResponse::newHttpResponse();
  response->setStatusCode(status);
  response->setBody(std::move(body));
  response->setContentTypeCode(ContentType::CT_APPLICATION_JSON);

  m_responseBuilt = true;

  return response;
}
