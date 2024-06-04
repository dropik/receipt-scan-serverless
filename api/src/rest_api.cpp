//
// Created by Daniil Ryzhkov on 04/06/2024.
//

#include "config.h"
#include "rest_api.hpp"
#include "di.hpp"

#include "services/file_service.hpp"

using namespace api;
using namespace rest;
using namespace services;

api_root api::create_api() {
  api_root api;

  api.use([](const auto &request, const auto &next) {
    auto auth = request.request_context.authorizer;
    auto user_id = auth.claims["sub"];
    if (user_id.empty()) {
      return unauthorized();
    }
    models::current_identity.user_id = user_id;
    return next(request);
  });

  api.use_logging(di<aws_lambda_cpp::common::logger>::get());
  api.use([](const auto &request, const auto &next) {
    di<aws_lambda_cpp::common::logger>::get()->info("App Version: %s", APP_VERSION);
    return next(request);
  });

  api.use_exception_filter();

  api.any("/v1")([](api_resource &v1) {
    v1.any("/user")([](api_resource &user) {
      user.post<models::upload_file_params>("/files")([](const auto &request) {
        return di<file_service>::get()->get_upload_file_url(request);
      });
    });
  });

  return api;
}
