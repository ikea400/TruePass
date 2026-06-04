#pragma once
#include <drogon/HttpFilter.h>
#include <drogon/RequestStream.h>
#include <drogon/drogon_callbacks.h>

class AuthFilter : public drogon::HttpFilter<AuthFilter> {
 public:
  void doFilter(const drogon::HttpRequestPtr &req, drogon::FilterCallback &&fcb,
                drogon::FilterChainCallback &&fccb) override;
};