//
// Created by Daniil Ryzhkov on 03/08/2024.
//

#include "base_api_integration_test.hpp"

#define ENDPOINT "/v1/budgets"
#define TEST_BUDGET "d394a832-4011-7023-c519-afe3adaf0233"

using namespace repository;

namespace api {
namespace integration_tests {

class budget_test : public base_api_integration_test {
 protected:
  ::models::budget create_budget(const lambda::nullable<int>& version = lambda::nullable<int>()) {
    auto repo = services.get<repository::t_client>();
    auto b = ::models::budget{
        .id = TEST_BUDGET,
        .user_id = USER_ID,
        .month = "2024-07-01",
        .amount = 1500.0,
        .version = version.has_value() ? version.get_value() : 0,
    };
    repo->create<::models::budget>(b);
    return b;
  }
};

TEST_F(budget_test, put_budget) {
  // should not be created until user is created
  auto response = (*api)(create_request("PUT", ENDPOINT, R"(
{
  "id": ")" TEST_BUDGET R"(",
  "month": "2024-07-01",
  "amount": 1500.0,
  "version": 0
})"));
  assert_response(response, "400", R"({"error":4,"message":"User is not initialized"})");

  auto repo = services.get<repository::t_client>();
  auto budgets = repo->select<::models::budget>("select * from budgets").all();

  ASSERT_EQ(budgets->size(), 0);

  init_user();

  // should create budget
  response = (*api)(create_request("PUT", ENDPOINT, R"(
{
  "id": ")" TEST_BUDGET R"(",
  "month": "2024-07-01",
  "amount": 1500.0,
  "version": 0
})"));
  assert_response(response, "200", "");

  budgets = repo->select<::models::budget>("select * from budgets").all();
  ASSERT_EQ(budgets->size(), 1);

  auto b = budgets->at(0);
  ASSERT_EQ(b->id, TEST_BUDGET);
  ASSERT_EQ(b->user_id, USER_ID);
  ASSERT_EQ(b->month, "2024-07-01");
  ASSERT_EQ(b->amount, 1500.0);
  ASSERT_EQ(b->version, 0);
}

TEST_F(budget_test, put_budget_update) {
  init_user();
  create_budget();

  auto response = (*api)(create_request("PUT", ENDPOINT, R"(
{
  "id": ")" TEST_BUDGET R"(",
  "month": "2024-07-01",
  "amount": 1200.0,
  "version": 0
})"));
  assert_response(response, "200", "");

  auto repo = services.get<repository::t_client>();
  auto budgets = repo->select<::models::budget>("select * from budgets").all();
  ASSERT_EQ(budgets->size(), 1);

  auto b = budgets->at(0);
  ASSERT_EQ(b->version, 1);
  ASSERT_EQ(b->amount, 1200.0);
}

TEST_F(budget_test, put_budget_update_conflict) {
  init_user();
  auto b1 = create_budget(1);

  auto response = (*api)(create_request("PUT", ENDPOINT, R"(
{
  "id": ")" TEST_BUDGET R"(",
  "month": "2024-07-01",
  "amount": 1200.0,
  "version": 0
})"));
  assert_response(response, "409", R"("Optimistic concurrency error")");

  auto repo = services.get<repository::t_client>();
  auto budgets = repo->select<::models::budget>("select * from budgets").all();
  ASSERT_EQ(budgets->size(), 1);

  auto b = budgets->at(0);
  ASSERT_EQ(b->version, b1.version);
  ASSERT_EQ(b->amount, b1.amount);
}

TEST_F(budget_test, get_budgets) {
  init_user();
  create_budget();

  auto response = (*api)(create_request("GET", ENDPOINT, ""));
  assert_response(response, "200", R"(
[
  {
    "amount": 1500.0,
    "id": ")" TEST_BUDGET R"(",
    "month": "2024-07-01",
    "version":0
  }
])");
}

}
}
