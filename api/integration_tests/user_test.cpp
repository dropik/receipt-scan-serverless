//
// Created by Daniil Ryzhkov on 03/08/2024.
//

#include "base_api_integration_test.hpp"
#include "repository/models/user.hpp"

using namespace repository;

namespace api {
namespace integration_tests {

class user_test : public base_api_integration_test {};

TEST_F(user_test, post_users) {
  // should initialize user
  auto response = (*api)(create_request("POST", "/v1/user", "null"));
  assert_response(response, "200", "");

  // user should be in the database
  auto repo = services.get<repository::t_client>();
  auto users = repo->select<::models::user>("select * from users").all();
  ASSERT_EQ(users->size(), 1);
  ASSERT_EQ(users->at(0)->id, "d394a832-4011-7023-c519-afe3adaf0233");

  // initializing same user once again is no-op
  response = (*api)(create_request("POST", "/v1/user", "null"));
  assert_response(response, "200", "");

  users = repo->select<::models::user>("select * from users").all();
  ASSERT_EQ(users->size(), 1);
  ASSERT_EQ(users->at(0)->id, "d394a832-4011-7023-c519-afe3adaf0233");
}

TEST_F(user_test, get_users) {
  // should return 404 if user does not exist
  auto response = (*api)(create_request("GET", "/v1/user", "null"));
  assert_response(response, "404", "");

  (*api)(create_request("POST", "/v1/user", "null"));

  // should return user
  response = (*api)(create_request("GET", "/v1/user", "null"));
  assert_response(response, "200", R"({"id":"d394a832-4011-7023-c519-afe3adaf0233"})");
}

}
}
