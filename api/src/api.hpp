//
// Created by Daniil Ryzhkov on 04/06/2024.
//

#pragma once

#include <rest/api_root.hpp>

#include "config.h"

#include "identity.hpp"
#include "model_types.hpp"
#include "parameters/put_file.hpp"

#include "services/user_service.hpp"
#include "services/file_service.hpp"
#include "services/receipt_service.hpp"
#include "services/category_service.hpp"
#include "services/device_service.hpp"
#include "services/budget_service.hpp"

namespace api {

using api_root = rest::api_root;
using api_resource = rest::api_resource;

template<typename TServiceContainer>
std::unique_ptr<api_root> create_api(TServiceContainer &c) {
  std::unique_ptr<api_root> api = std::make_unique<api_root>();

  // Middleware

  // Identity
  api->use([&c](const auto &request, const auto &next) {
    auto auth = request.request_context.authorizer;
    auto user_id = auth.claims["sub"];
    if (user_id.empty()) {
      return rest::unauthorized();
    }

    auto i = c.template get<identity>();
    i->user_id = user_id;
    return next(request);
  });

  api->use_logging();

  // Version
  api->use([](const auto &request, const auto &next) {
    lambda::log.info("App Version: %s", APP_VERSION);
    return next(request);
  });

  api->use([&c](const auto &request, const auto &next) {
    if (request.path == "/v1/user") return next(request);

    auto repo = c.template get<repository::t_client>();
    auto id = c.template get<identity>();
    auto users = repo->template select<repository::models::user>("select * from users where id = ?")
        .with_param(id->user_id)
        .all();
    if (users->size() == 0) {
      throw rest::api_exception(user_not_initialized, "User is not initialized");
    }

    return next(request);
  });

  api->use([](const auto &request, const auto &next) {
    try {
      return next(request);
    } catch (rest::api_exception &e) {
      lambda::log.info("API Exception: %d %s", e.error, e.message.c_str());
      if (e.error == not_found) {
        return rest::not_found();
      }
      return bad_request(e);
    } catch (repository::entity_not_found_exception &e) {
      return rest::not_found();
    } catch (repository::concurrency_exception &e) {
      return rest::conflict();
    } catch (std::exception &e) {
      lambda::log.error("Internal error: %s", e.what());
      return rest::internal_server_error();
    }
  });

  // Routes

  api->any("/v1")([&c](api_resource &v1) {
    v1.any("/user")([&c](api_resource &user) {
      user.post("/")([&c]() {
        return c.template get<services::t_user_service>()->init_user();
      });
      user.get("/")([&c]() {
        return c.template get<services::t_user_service>()->get_user();
      });
    });

    v1.put<parameters::put_device>("/devices")([&c](const auto &request) {
      return c.template get<services::t_device_service>()->register_device(request);
    });

    v1.any("/budgets")([&c](api_resource &budgets) {
      budgets.get("/")([&c]() {
        return c.template get<services::t_budget_service>()->get_budgets();
      });
      budgets.put<parameters::put_budget>("/")([&c](const auto &request) {
        return c.template get<services::t_budget_service>()->store_budget(request);
      });
    });

    v1.any("/categories")([&c](api_resource &categories) {
      categories.get("/")([&c]() {
        return c.template get<services::t_category_service>()->get_categories();
      });
      categories.put<parameters::put_category>("/")([&c](const auto &request) {
        return c.template get<services::t_category_service>()->put_category(request);
      });
      categories.del<guid_t>()([&c](const guid_t &category_id) {
        return c.template get<services::t_category_service>()->delete_category(category_id);
      });
    });

    v1.any("/files")([&c](api_resource &files) {
      files.post<parameters::put_file>("/")([&c](const auto &request) {
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
      receipts.put<parameters::put_receipt>("/")([&c](const auto &request) {
        return c.template get<services::t_receipt_service>()->put_receipt(request);
      });
      receipts.del<guid_t>()([&c](const guid_t &receipt_id) {
        return c.template get<services::t_receipt_service>()->delete_receipt(receipt_id);
      });
    });
  });

  return std::move(api);
}

}
