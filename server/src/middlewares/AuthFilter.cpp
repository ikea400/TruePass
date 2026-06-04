#include "AuthFilter.h"

#include <drogon/HttpTypes.h>
#include <drogon/drogon_callbacks.h>
#include <utils/uuid.h>

#include "../services/SessionService.h"
#include "../utils/ResponseBuilder.h"

using namespace drogon;
using namespace ikea400;

void AuthFilter::doFilter(const HttpRequestPtr& req, FilterCallback&& fcb,
                          FilterChainCallback&& fccb) {
  uuid userId = SessionService::instance().getAuthenticatedUserId(req);
  if (userId.isNull()) {
    fcb(ResponseBuilder::get(req)->failure(
        "Unauthorized", k401Unauthorized));  // User is not authenticated,
                                             // return 401 response
    return;
  }

  req->attributes()->insert("user_id", userId);
  fccb();  // User is authenticated, proceed to the next filter or handler
}
