//
// Created by Daniil Ryzhkov on 22/10/2024.
//

#include "base_api_integration_test.hpp"

#define ENDPOINT "/v1/rtdn"

using namespace api::integration_tests;
using namespace api::parameters;

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


gp_notification create_notification() {
  return gp_notification {
    .version = "1.0",
    .package_name = "com.package",
    .event_time_millis = 1634870000000,
    .test_notification = test_notification {
      .version = "1.0",
    },
  };
}

TEST_F(rtdn_test, should_handle_test_event) {
  auto notification = create_notification();
  auto response = send_request(notification);
  assert_response(response, "200", "");
}
