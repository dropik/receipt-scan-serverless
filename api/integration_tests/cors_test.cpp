//
// Created by Daniil Ryzhkov on 06/10/2024.
//

#include "base_api_integration_test.hpp"

using namespace api::integration_tests;

class cors_test : public base_api_integration_test {};

TEST_F(cors_test, should_add_header_if_origin_matches) {
  auto response = (*api)(create_request("OPTIONS", "/v1/user", "", "https://speza.it"));
  assert_response(response, "204", "", true);
}

TEST_F(cors_test, should_not_add_header_if_origin_invalid) {
  auto response = (*api)(create_request("OPTIONS", "/v1/user", "", "http://example.com"));
  assert_response(response, "204", "", false);
}
