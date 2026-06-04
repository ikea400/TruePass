#pragma once
#include <drogon/HttpRequest.h>
#include <drogon/HttpTypes.h>

#include <algorithm>
#include <glaze/json/generic.hpp>
#include <glaze/json/read.hpp>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

namespace ikea400 {
class RequestReader {
 public:
  template <class T>
  static std::optional<T> read(const drogon::HttpRequest& req) {
    std::optional<std::string_view> jsonString = getJsonString(req);
    if (!jsonString.has_value()) return std::nullopt;

    auto result = glz::read_json<T>(*jsonString);
    if (!result) return std::nullopt;

    return std::make_optional<T>(std::move(*result));
  }

  template <class T>
  std::shared_ptr<T> readPtr(const drogon::HttpRequest& req) {
    std::optional<std::string_view> jsonString = getJsonString(req);
    if (!jsonString.has_value()) return nullptr;

    auto result = glz::read_json<T>(*jsonString);
    if (!result) return nullptr;

    return std::make_shared<T>(std::move(*result));
  }

 private:
  static bool isJson(const drogon::HttpRequest& req) {
    return req.getContentType() == drogon::CT_APPLICATION_JSON ||
           req.getHeader("content-type").find("application/json") !=
               std::string::npos;
  }

  static std::optional<std::string_view> getJsonString(
      const drogon::HttpRequest& req) {
    if (!isJson(req)) return std::nullopt;
    const auto& body = req.getBody();
    if (body.empty()) return std::nullopt;
    return std::make_optional<std::string_view>(body.data(), body.size());
  }
};
}  // namespace ikea400