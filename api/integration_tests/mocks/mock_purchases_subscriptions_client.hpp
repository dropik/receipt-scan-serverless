//
// Created by Daniil Ryzhkov on 23/10/2024.
//

#pragma once

#include "../../src/services/google_api/purchases_subscriptions/purchases_subscriptions_client.hpp"

namespace api::integration_tests::mocks {

template<typename TAuthProvider = services::google_api::google_api_auth_provider>
class mock_purchases_subscriptions_client {
 public:
  explicit mock_purchases_subscriptions_client(TAuthProvider auth) : m_auth(std::move(auth)) {}

  bool was_acknowledged = false;

  services::outcome<services::no_result> acknowledge(const std::string &package_name,
                                                     const std::string &subscription_id,
                                                     const std::string &token,
                                                     const services::google_api::purchases_subscriptions::models::purchases_subscriptions_acknowledge_request &request) {
    m_auth->get_access_token();
    was_acknowledged = true;
    return services::outcome<services::no_result>(services::no_result{});
  }

 private:
  TAuthProvider m_auth;
};

}
