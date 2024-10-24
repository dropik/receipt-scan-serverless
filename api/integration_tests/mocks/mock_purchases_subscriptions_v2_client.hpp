//
// Created by Daniil Ryzhkov on 22/10/2024.
//

#pragma once

#include "../../src/services/google_api/purchases_subscriptions_v2/purchases_subscriptions_v2_client.hpp"

namespace api::integration_tests::mocks {

template<typename TAuthProvider = services::google_api::google_api_auth_provider>
class mock_purchases_subscriptions_v2_client {
 public:
  explicit mock_purchases_subscriptions_v2_client(TAuthProvider auth) : m_auth(std::move(auth)), was_revoked(false) {}

  services::outcome<services::google_api::purchases_subscriptions_v2::models::subscription_purchase_v2> get(const std::string &package_name,
                                                                                                            const std::string &token) {
    m_auth->get_access_token();
    return response;
  }
  services::outcome<services::no_result> revoke(const std::string &package_name,
                                                const std::string &token,
                                                const services::google_api::purchases_subscriptions_v2::models::purchases_subscriptions_v2_revoke_request &request) {
    m_auth->get_access_token();
    was_revoked = true;
    return services::outcome<services::no_result>(services::no_result{});
  }

  services::outcome<services::google_api::purchases_subscriptions_v2::models::subscription_purchase_v2> response;
  bool was_revoked = false;

 private:
  TAuthProvider m_auth;
};

}
