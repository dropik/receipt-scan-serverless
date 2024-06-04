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

  api.use_exception_filter();
  api.use_logging(di<aws_lambda_cpp::common::logger>::get());
  api.use([](const auto &request, const auto &next) {
    di<aws_lambda_cpp::common::logger>::get()->info("App Version: %s", APP_VERSION);
    return next(request);
  });

  api.any("/v1")([](api_resource &v1) {
    v1.any("/users")([](api_resource &users) {
      users.any<std::string>()([](std::string &user_id, api_resource &user) {
        user.post<models::upload_file_params>("/files")([user_id](const auto &request) {
          return di<file_service>::get()->get_upload_file_url(user_id, request);
        });
      });
    });
  });

  return api;
}
