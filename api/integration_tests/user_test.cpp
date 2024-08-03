//
// Created by Daniil Ryzhkov on 03/08/2024.
//

#include "base_api_integration_test.hpp"
#include "../src/api.hpp"

namespace api {
namespace integration_tests {

class user_test : public base_api_integration_test {};

TEST_F(user_test, post_users) {
  auto response = (*api)(create_request("POST", "/v1/users", "null"));
  assert_response(response, "200", "");
}

}
}
