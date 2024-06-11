//
// Created by Daniil Ryzhkov on 04/06/2024.
//

#include "config.h"
#include "rest_api.hpp"
#include "di.hpp"

using namespace api;
using namespace rest;
using namespace services;
using namespace models;

api_root api::create_api() {
  api_root api;

  // Middleware

  // Identity
  api.use([](const auto &request, const auto &next) {
    auto auth = request.request_context.authorizer;
    auto user_id = auth.claims["sub"];
    if (user_id.empty()) {
      return unauthorized();
    }
    models::current_identity.user_id = user_id;
    return next(request);
  });

  api.use_logging(di<lambda::logger>::get());

  // Version
  api.use([](const auto &request, const auto &next) {
    di<lambda::logger>::get()->info("App Version: %s", APP_VERSION);
    return next(request);
  });

  api.use_exception_filter();

  // Routes

  api.any("/v1")([](api_resource &v1) {
    v1.post("/user")([]() {
      return di<user_service>::get()->init_user();
    });

    v1.any("/files")([](api_resource &files) {
      files.post<upload_file_params>("/")([](const auto &request) {
        return di<file_service>::get()->get_upload_file_url(request);
      });
    });

    v1.any("/receipts")([](api_resource &receipts) {
      receipts.get("/")([]() {
        return di<receipt_service>::get()->get_receipts();
      });
      receipts.any<guid_t>()([](const guid_t &receipt_id, api_resource &receipt) {
        receipt.get("/")([receipt_id = std::forward<const guid_t>(receipt_id)]() {
          return di<receipt_service>::get()->get_receipt(receipt_id);
        });
        receipt.get("/file")([receipt_id = std::forward<const guid_t>(receipt_id)]() {
          return di<receipt_service>::get()->get_receipt_file(receipt_id);
        });
      });
      receipts.put<receipt_put_params>("/")([](const auto &request) {
        return di<receipt_service>::get()->put_receipt(request);
      });
      receipts.del<guid_t>()([](const guid_t &receipt_id) {
        return di<receipt_service>::get()->delete_receipt(receipt_id);
      });
    });

    v1.any("/categories")([](api_resource &categories) {
      categories.get("/")([]() {
        return di<category_service>::get()->get_categories();
      });
      categories.put<category>("/")([](const auto &request) {
        return di<category_service>::get()->put_category(request);
      });
      categories.del<guid_t>()([](const guid_t &category_id) {
        return di<category_service>::get()->delete_category(category_id);
      });
    });
  });

  return api;
}
