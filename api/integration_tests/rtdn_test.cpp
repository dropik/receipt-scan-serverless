//
// Created by Daniil Ryzhkov on 22/10/2024.
//

#include "base_api_integration_test.hpp"

#define ENDPOINT "/v1/rtdn"

using namespace api::integration_tests;
using namespace api::parameters;
using namespace api::services;
using namespace api::services::google_api::purchases_subscriptions_v2;

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
          .purchase_token = "purchase_token",
          .subscription_id = "subscription_id",
      } : lambda::nullable<subscription_notification>{},
      .test_notification = is_test ? test_notification{
          .version = "1.0",
      } : lambda::nullable<test_notification>{},
  };
}

outcome<models::subscription_purchase_v2> create_subscription_outcome() {
  return outcome<models::subscription_purchase_v2>(models::subscription_purchase_v2{
      .kind = "androidpublisher#subscriptionPurchase",
      .region_code = "IT",
      .test_purchase = models::test_purchase{},
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
  subscriptions_v2_client->response = create_subscription_outcome();
  auto response = send_request(notification);
  assert_response(response, "200", "");
}
