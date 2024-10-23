//
// Created by Daniil Ryzhkov on 19/10/2024.
//

#pragma once

#include "../parameters/rtdn.hpp"
#include "../parameters/gp_notification.hpp"
#include "../http_request.hpp"
#include "lambda/log.hpp"
#include "google_api/purchases_subscriptions/purchases_subscriptions_client.hpp"
#include "google_api/purchases_subscriptions_v2/purchases_subscriptions_v2_client.hpp"
#include "google_api/purchases_subscriptions_v2/models/enums.hpp"

namespace api::services {

struct t_rtdn_service {};

template<
    typename TSubscriptionsClient = google_api::purchases_subscriptions::t_purchases_subscriptions_client,
    typename TSubscriptionsV2Client = google_api::purchases_subscriptions_v2::t_purchases_subscriptions_v2_client,
    typename TRepository = repository::t_client>
class rtdn_service {
 public:
  explicit rtdn_service(TSubscriptionsClient subscriptions_client, TSubscriptionsV2Client subscriptions_v2_client, TRepository repository)
      : m_subscriptions_client(std::move(subscriptions_client)), m_subscriptions_v2_client(std::move(subscriptions_v2_client)), m_repository(std::move(repository)) {}

  void process_message(const parameters::rtdn<parameters::gp_notification> &message) {
    auto notification = message.get_message();
    if (notification.test_notification.has_value()) {
      lambda::log.info("Received test notification");
      return;
    }

    if (!notification.subscription_notification.has_value()) return;
    auto subscription_notification = notification.subscription_notification.get_value();
    auto outcome = get_subscription_state_outcome(notification.package_name, subscription_notification.purchase_token);
    if (outcome.result.test_purchase.has_value()) {
      lambda::log.info("Received test subscription");
      return;
    }

    auto user = get_user_by_token(subscription_notification.purchase_token);
    if (outcome.result.subscription_state == google_api::purchases_subscriptions_v2::models::subscription_state::active) {
      for (auto &line_item : outcome.result.line_items) {
        if (line_item.product_id == "speza.subscription.base" && line_item.expiry_time.has_value()) {
          user->has_subscription = true;
          auto expiry_time = line_item.expiry_time.get_value();
          lambda::string::replace_all(expiry_time, "T", " ");
          lambda::string::replace_all(expiry_time, "Z", "");
          user->subscription_expiry_time = expiry_time;
          m_repository->update(*user);

          if (outcome.result.acknowledgement_state == google_api::purchases_subscriptions_v2::models::acknowledgement_state::pending) {
            m_subscriptions_client->acknowledge(notification.package_name, subscription_notification.subscription_id, subscription_notification.purchase_token, {});
          }
          return;
        }
      }
    }
  }

 private:
  TSubscriptionsClient m_subscriptions_client;
  TSubscriptionsV2Client m_subscriptions_v2_client;
  TRepository m_repository;

  using subscription_purchase_v2 = google_api::purchases_subscriptions_v2::models::subscription_purchase_v2;

  outcome<subscription_purchase_v2> get_subscription_state_outcome(const std::string &package_name, const std::string &token) {
    return m_subscriptions_v2_client->get(package_name, token);
  }

  std::shared_ptr<repository::models::user> get_user_by_token(const std::string &purchase_token) {
    return m_repository->template select<repository::models::user>("select * from users where purchase_token = ?")
        .with_param(purchase_token)
        .first_or_default();
  }
};

}
