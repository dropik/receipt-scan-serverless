//
// Created by Daniil Ryzhkov on 04/06/2024.
//

#pragma once

#include <rest/api_root.hpp>

#include "config.h"

#include "models/identity.hpp"
#include "models/model_types.hpp"
#include "models/upload_file_params.hpp"

#include "services/user_service.hpp"
#include "services/file_service.hpp"
#include "services/receipt_service.hpp"
#include "services/category_service.hpp"

namespace api {

using api_root = rest::api_root;
using api_resource = rest::api_resource;
using guid_t = models::guid_t;

template<typename TServiceContainer>
api_root create_api(TServiceContainer &c) {
  api_root api;

  // Middleware

  // Identity
  api.use([&c](const auto &request, const auto &next) {
    auto auth = request.request_context.authorizer;
    auto user_id = auth.claims["sub"];
    if (user_id.empty()) {
      return rest::unauthorized();
    }

    auto identity = c.template get<models::identity>();
    identity->user_id = user_id;
    return next(request);
  });

  api.use_logging(c.template get<lambda::logger>());

  // Version
  api.use([&c](const auto &request, const auto &next) {
    c.template get<lambda::logger>()->info("App Version: %s", APP_VERSION);
    return next(request);
  });

  api.use_exception_filter();

  // Routes

  api.any("/v1")([&c](api_resource &v1) {
    v1.post("/user")([&c]() {
      return c.template get<services::t_user_service>()->init_user();
    });

    v1.any("/files")([&c](api_resource &files) {
      files.post<models::upload_file_params>("/")([&c](const auto &request) {
        return c.template get<services::t_file_service>()->get_upload_file_url(request);
      });
    });

    v1.any("/receipts")([&c](api_resource &receipts) {
      receipts.get("/")([&c]() {
        return c.template get<services::t_receipt_service>()->get_receipts();
      });
      receipts.any<guid_t>()([&c](const guid_t &receipt_id, api_resource &receipt) {
        receipt.get("/")([&c, &receipt_id]() {
          return c.template get<services::t_receipt_service>()->get_receipt(receipt_id);
        });
        receipt.get("/file")([&c, &receipt_id]() {
          return c.template get<services::t_receipt_service>()->get_receipt_file(receipt_id);
        });
      });
      receipts.put<models::receipt_put_params>("/")([&c](const auto &request) {
        return c.template get<services::t_receipt_service>()->put_receipt(request);
      });
      receipts.del<guid_t>()([&c](const guid_t &receipt_id) {
        return c.template get<services::t_receipt_service>()->delete_receipt(receipt_id);
      });
    });

    v1.any("/categories")([&c](api_resource &categories) {
      categories.get("/")([&c]() {
        return c.template get<services::t_category_service>()->get_categories();
      });
      categories.put<models::category>("/")([&c](const auto &request) {
        return c.template get<services::t_category_service>()->put_category(request);
      });
      categories.del<guid_t>()([&c](const guid_t &category_id) {
        return c.template get<services::t_category_service>()->delete_category(category_id);
      });
    });
  });

  return api;
}

}