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
  ASSERT_EQ(users->at(0)->has_subscription, false);

  repo->execute("update users set has_subscription = 1").go();

  // initializing same user once again is no-op, and should not change has_subscription
  response = (*api)(create_request("POST", ENDPOINT, ""));
  assert_response(response, "200", "");

  users = repo->select<::models::user>("select * from users").all();
  ASSERT_EQ(users->size(), 1);
  ASSERT_EQ(users->at(0)->id, USER_ID);
  ASSERT_EQ(users->at(0)->has_subscription, true);
}

TEST_F(user_test, get_user) {
  // should return 404 if user does not exist
  auto response = (*api)(create_request("GET", ENDPOINT, ""));
  assert_response(response, "404", "");

  (*api)(create_request("POST", ENDPOINT, ""));

  // should return user
  response = (*api)(create_request("GET", ENDPOINT, ""));
  assert_response(response, "200", R"({"hasSubscription": false, "id":")" USER_ID R"("})");
}

TEST_F(user_test, delete_user) {
  (*api)(create_request("POST", ENDPOINT, ""));
  create_budget();
  create_category();
  create_receipt();
  create_receipt_item(0);

  auto repo = services.get<repository::t_client>();
  auto user = repo->get<::models::user>(USER_ID);
  user->has_subscription = true;
  user->purchase_token = "token";
  user->payment_account_email = "email";
  user->subscription_expiry_time = "2024-08-03 00:00:00";
  repo->update(*user);

  // should delete user
  auto response = (*api)(create_request("DELETE", ENDPOINT, ""));
  assert_response(response, "200", "");

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

  auto subscriptions_client = services.get<services::google_api::purchases_subscriptions_v2::t_purchases_subscriptions_v2_client>();
  ASSERT_TRUE(subscriptions_client->was_revoked);

  // should be idempotent
  response = (*api)(create_request("DELETE", ENDPOINT, ""));
  assert_response(response, "200", "");
}

}
