//
// Created by Daniil Ryzhkov on 03/08/2024.
//

#include "base_api_integration_test.hpp"
#include "repository/models/user.hpp"

#define ENDPOINT "/v1/user"

using namespace repository;

namespace api::integration_tests {

class user_test : public base_api_integration_test {};

TEST_F(user_test, post_user) {
  // should initialize user
  auto response = (*api)(create_request("POST", ENDPOINT, ""));
  assert_response(response, "200", "");

  // user should be in the database
  auto repo = services.get<repository::t_client>();
  auto users = repo->select<::models::user>("select * from users").all();
  ASSERT_EQ(users->size(), 1);
  ASSERT_EQ(users->at(0)->id, USER_ID);

  // initializing same user once again is no-op
  response = (*api)(create_request("POST", ENDPOINT, ""));
  assert_response(response, "200", "");

  users = repo->select<::models::user>("select * from users").all();
  ASSERT_EQ(users->size(), 1);
  ASSERT_EQ(users->at(0)->id, USER_ID);
}

TEST_F(user_test, get_user) {
  // should return 404 if user does not exist
  auto response = (*api)(create_request("GET", ENDPOINT, ""));
  assert_response(response, "404", "");

  (*api)(create_request("POST", ENDPOINT, ""));

  // should return user
  response = (*api)(create_request("GET", ENDPOINT, ""));
  assert_response(response, "200", R"({"id":")" USER_ID R"("})");
}

TEST_F(user_test, delete_user) {
  (*api)(create_request("POST", ENDPOINT, ""));
  create_budget();
  create_category();
  create_receipt();
  create_receipt_item(0);

  // should delete user
  auto response = (*api)(create_request("DELETE", ENDPOINT, ""));
  assert_response(response, "200", "");

  auto repo = services.get<repository::t_client>();
  auto users = repo->select<::models::user>("select * from users").all();
  ASSERT_EQ(users->size(), 0);

  auto budgets = repo->select<::models::budget>("select * from budgets").all();
  ASSERT_EQ(budgets->size(), 0);

  auto categories = repo->select<::models::category>("select * from categories").all();
  ASSERT_EQ(categories->size(), 0);

  auto receipts = repo->select<::models::receipt>("select * from receipts").all();
  ASSERT_EQ(receipts->size(), 0);

  auto receipt_items = repo->select<::models::receipt_item>("select * from receipt_items").all();
  ASSERT_EQ(receipt_items->size(), 0);

  // should be idempotent
  response = (*api)(create_request("DELETE", ENDPOINT, ""));
  assert_response(response, "200", "");
}

}
