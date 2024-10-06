//
// Created by Daniil Ryzhkov on 06/10/2024.
//

#include "base_api_integration_test.hpp"

using namespace api::integration_tests;

class cors_test : public base_api_integration_test {};

TEST_F(cors_test, should_not_add_header_if_origin_missing) {
  auto response = (*api)(create_request("OPTIONS", "/v1/user", "", false));
  assert_response(response, "405", "", false);
}
