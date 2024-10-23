//
// Created by Daniil Ryzhkov on 22/10/2024.
//

#include "base_api_integration_test.hpp"

#define ENDPOINT "/v1/rtdn"
#define PURCHASE_TOKEN "purchase_token"

using namespace api::integration_tests;
using namespace api::parameters;
using namespace api::services;
using namespace api::services::google_api::purchases_subscriptions;
using namespace api::services::google_api::purchases_subscriptions_v2;
using namespace api::services::google_api::purchases_subscriptions::models;
using namespace api::services::google_api::purchases_subscriptions_v2::models;

Aws::Utils::Base64::Base64 base64;

class rtdn_test : public base_api_integration_test {
 protected:
  aws::lambda_runtime::invocation_response send_request(const gp_notification &notification) {
    auto base64_str = base64.Encode(lambda::json::serialize(notification));

    auto message = rtdn<gp_notification>{
        .message = rtdn_message<gp_notification>{
            .data = base64_str,
        },
    };

    return (*this->api)(create_request("POST", ENDPOINT, lambda::json::serialize(message)));
  }
};


gp_notification create_notification(bool is_test = false) {
  return gp_notification{
      .version = "1.0",
      .package_name = "com.package",
      .event_time_millis = 1634870000000,
      .subscription_notification = !is_test ? subscription_notification{
          .version = "1.0",
          .notification_type = 0,
          .purchase_token = PURCHASE_TOKEN,
          .subscription_id = "subscription_id",
      } : lambda::nullable<subscription_notification>{},
      .test_notification = is_test ? test_notification{
          .version = "1.0",
      } : lambda::nullable<test_notification>{},
  };
}

outcome<subscription_purchase_v2> create_subscription_outcome(bool is_test = false,
                                                              const std::optional<std::string> &expiry_time = {}) {
  return outcome<subscription_purchase_v2>(subscription_purchase_v2{
      .kind = "androidpublisher#subscriptionPurchase",
      .region_code = "IT",
      .line_items = !is_test ? std::vector<subscription_purchase_line_item>{
          subscription_purchase_line_item{
              .product_id = "speza.subscription.base",
              .expiry_time = expiry_time.value_or("2024-11-01T00:00:00Z"),
              .auto_renewing_plan = auto_renewing_plan{
                  .auto_renew_enabled = true,
              },
          }
      } : std::vector<subscription_purchase_line_item>{},
      .subscription_state = subscription_state::active,
      .test_purchase = is_test ? test_purchase{} : lambda::nullable<test_purchase>{},
      .acknowledgement_state = acknowledgement_state::pending,
  });
}

TEST_F(rtdn_test, should_handle_test_event) {
  auto notification = create_notification(true);
  auto response = send_request(notification);
  assert_response(response, "200", "");
}

TEST_F(rtdn_test, should_handle_test_subscription) {
  auto notification = create_notification();
  auto subscriptions_v2_client = services.get<t_purchases_subscriptions_v2_client>();
  subscriptions_v2_client->response = create_subscription_outcome(true);
  auto response = send_request(notification);
  assert_response(response, "200", "");
}

TEST_F(rtdn_test, should_handle_new_subscriptions) {
  init_user(false, PURCHASE_TOKEN);
  std::string expiry_time = "2024-11-01T00:00:00Z";
  auto notification = create_notification();
  auto subscriptions_v2_client = services.get<t_purchases_subscriptions_v2_client>();
  subscriptions_v2_client->response = create_subscription_outcome(false, expiry_time);
  auto response = send_request(notification);
  assert_response(response, "200", "");

  // should grant access to service
  auto client = services.get<repository::t_client>();
  auto user = client->get<repository::models::user>(USER_ID);
  ASSERT_EQ(user->has_subscription, true);
  ASSERT_EQ(user->purchase_token.get_value(), PURCHASE_TOKEN);
  ASSERT_EQ(user->subscription_expiry_time.get_value(), "2024-11-01 00:00:00");

  // should acknowledge subscription
  auto subscriptions_client = services.get<t_purchases_subscriptions_client>();
  ASSERT_TRUE(subscriptions_client->was_acknowledged);
}
