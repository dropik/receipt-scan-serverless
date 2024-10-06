//
// Created by Daniil Ryzhkov on 04/06/2024.
//

#pragma once

#include <rest/api_root.hpp>

#include "config.h"

#include "identity.hpp"
#include "http_request.hpp"
#include "model_types.hpp"

#include "services/user_service.hpp"
#include "services/file_service.hpp"
#include "services/receipt_service.hpp"
#include "services/category_service.hpp"
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

    if (request.path == "/v1/user") return next(request);
    auto repo = c.template get<repository::t_client>();
    auto users = repo->template select<repository::models::user>("select * from users where id = ?")
        .with_param(user_id)
        .all();
    if (users->size() == 0) {
      return rest::bad_request(rest::api_exception(user_not_initialized, "User is not initialized"));
    }

    return next(request);
  });

  // Logging
  api->use_logging();

  // Version
  api->use([](const auto &request, const auto &next) {
    lambda::log.info("App Version: %s", APP_VERSION);
    return next(request);
  });

  // Http request storage
  api->use([&c](const auto &request, const auto &next) {
    auto r = c.template get<http_request>();
    r->current = request;
    return next(request);
  });

  // Error handling
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

  // CORS
  api->use([&c](const auto &request, const auto &next) {
    auto response = next(request);

    auto allowed_origins = std::vector<std::string>{"https://speza.it", "http://localhost:5173"};
    auto origin = request.headers.find("origin");
    if (origin == request.headers.end()) {
      origin = request.headers.find("Origin");
    }
    if (origin != request.headers.end() && std::find(allowed_origins.begin(), allowed_origins.end(), origin->second) != allowed_origins.end()) {
      response.headers["Access-Control-Allow-Headers"] = "Content-Type,X-Amz-Date,Authorization,X-Api-Key,X-Amz-Security-Token";
      response.headers["Access-Control-Allow-Methods"] = "OPTIONS,GET,POST,PUT,DELETE,PATCH";
      response.headers["Access-Control-Allow-Origin"] = origin->second;
      return response;
    }

    return response;
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
      user.del("/")([&c]() {
        return c.template get<services::t_user_service>()->delete_user();
      });
    });

    v1.any("/budgets")([&c](api_resource &budgets) {
      budgets.get("/")([&c]() {
        return c.template get<services::t_budget_service>()->get_budgets();
      });
      budgets.put<parameters::put_budget>("/")([&c](const auto &request) {
        return c.template get<services::t_budget_service>()->store_budget(request);
      });
      budgets.get("/changes")([&c]() {
        auto request = c.template get<http_request>()->current;
        return c.template get<services::t_budget_service>()->get_changes(request.query_string_parameters["from"]);
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
      categories.get("/changes")([&c]() {
        auto request = c.template get<http_request>()->current;
        return c.template get<services::t_category_service>()->get_changes(request.query_string_parameters["from"]);
      });
    });

    v1.any("/receipts")([&c](api_resource &receipts) {
      receipts.any("/years")([&c](api_resource &years) {
        years.any<int>()([&c](const int &y, api_resource &year) {
          year.any("/months")([&c, &y](api_resource &months) {
            months.get<int>()([&c, &y](const int &m) {
              return c.template get<services::t_receipt_service>()->get_receipts(y, m);
            });
          });
        });
      });
      receipts.any<guid_t>()([&c](const guid_t &receipt_id, api_resource &receipt) {
        receipt.get("/image")([&c, &receipt_id]() {
          return c.template get<services::t_receipt_service>()->get_receipt_get_image_url(receipt_id);
        });
        receipt.post("/image")([&c, &receipt_id]() {
          return c.template get<services::t_receipt_service>()->get_receipt_put_image_url(receipt_id);
        });
      });
      receipts.put<parameters::put_receipt>("/")([&c](const auto &request) {
        return c.template get<services::t_receipt_service>()->put_receipt(request);
      });
      receipts.del<guid_t>()([&c](const guid_t &receipt_id) {
        return c.template get<services::t_receipt_service>()->delete_receipt(receipt_id);
      });
      receipts.get("/changes")([&c]() {
        auto request = c.template get<http_request>()->current;
        return c.template get<services::t_receipt_service>()->get_changes(request.query_string_parameters["from"]);
      });
    });
  });

  return std::move(api);
}

}
