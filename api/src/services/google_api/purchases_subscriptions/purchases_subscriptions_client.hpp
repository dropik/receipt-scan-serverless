//
// Created by Daniil Ryzhkov on 20/10/2024.
//

#pragma once

#include <lambda/string_utils.hpp>
#include "../../http_client.hpp"
#include "models/purchases_subscriptions_acknowledge_request.hpp"
#include "../google_api_auth_provider.hpp"
#include "../base_google_api_client.hpp"

namespace api::services::google_api::purchases_subscriptions {

struct t_purchases_subscriptions_client {};

template<typename TAuthProvider = google_api_auth_provider>
class purchases_subscriptions_client : protected base_google_api_client {
 public:
  explicit purchases_subscriptions_client(TAuthProvider auth) : base_google_api_client(std::move(auth)) {}

  outcome<no_result> acknowledge(const std::string &package_name,
                                 const std::string &subscription_id,
                                 const std::string &token,
                                 const models::purchases_subscriptions_acknowledge_request &request) {
    auto url = lambda::string::format(acknowledge_url_format, package_name, subscription_id, token);
    auto body = lambda::json::serialize(request);
    return post<no_result>(url, body);
  }

 private:
  static inline const std::string acknowledge_url_format = "https://androidpublisher.googleapis.com/androidpublisher/v3/applications/%s/purchases/subscriptions/%s/tokens/%s:acknowledge";
};

}
