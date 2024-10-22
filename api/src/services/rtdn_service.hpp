//
// Created by Daniil Ryzhkov on 19/10/2024.
//

#pragma once

#include "../parameters/rtdn.hpp"
#include "../parameters/gp_notification.hpp"
#include "../http_request.hpp"
#include "lambda/log.hpp"
#include "google_api/purchases_subscriptions_v2/purchases_subscriptions_v2_client.hpp"

namespace api::services {

struct t_rtdn_service {};

template<typename TSubscriptionsV2Client = google_api::purchases_subscriptions_v2::t_purchases_subscriptions_v2_client>
class rtdn_service {
 public:
  explicit rtdn_service(TSubscriptionsV2Client subscriptions_v2_client)
      : m_subscriptions_v2_client(std::move(subscriptions_v2_client)) {}

  void process_message(const parameters::rtdn<parameters::gp_notification> &message) {
    auto notification = message.get_message();
    if (notification.test_notification.has_value()) {
      lambda::log.info("Received test notification");
      return;
    }
  }

 private:
  TSubscriptionsV2Client m_subscriptions_v2_client;

  using subscription_purchase_v2 = google_api::purchases_subscriptions_v2::models::subscription_purchase_v2;

  outcome<subscription_purchase_v2> get_subscription_state_outcome(const std::string &package_name, const std::string &token) {
    return m_subscriptions_v2_client.get(package_name, token);
  }
};

}
