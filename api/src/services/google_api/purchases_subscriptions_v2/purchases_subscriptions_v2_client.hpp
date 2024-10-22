//
// Created by Daniil Ryzhkov on 19/10/2024.
//

#pragma once

#include <lambda/string_utils.hpp>
#include "../google_api_auth_provider.hpp"
#include "models/purchases_subscriptions_v2_revoke_request.hpp"
#include "models/subscription_purchase_v2.hpp"
#include "../base_google_api_client.hpp"

namespace api::services::google_api::purchases_subscriptions_v2 {

struct t_purchases_subscriptions_v2_client {};

template<typename TAuthProvider = google_api_auth_provider>
class purchases_subscriptions_v2_client : protected base_google_api_client {
 public:
  explicit purchases_subscriptions_v2_client(TAuthProvider auth) : base_google_api_client(auth) {}

  outcome<models::subscription_purchase_v2> get(const std::string &package_name, const std::string &token) {
    auto url = lambda::string::format(get_url_format, package_name.c_str(), token.c_str());
    return base_google_api_client::get<models::subscription_purchase_v2>(url);
  }

  outcome<no_result> revoke(const std::string &package_name,
                            const std::string &token,
                            const models::purchases_subscriptions_v2_revoke_request &request) {
    auto url = lambda::string::format(revoke_url_format, package_name.c_str(), token.c_str());
    auto body = lambda::json::serialize(request, true);
    return post<no_result>(url, body);
  }

 private:
  static const inline std::string get_url_format =
      "https://androidpublisher.googleapis.com/androidpublisher/v3/applications/%s/purchases/subscriptionsv2/tokens/%s";
  static const inline std::string revoke_url_format =
      "https://androidpublisher.googleapis.com/androidpublisher/v3/applications/%s/purchases/subscriptionsv2/tokens/%s:revoke";
};

}
