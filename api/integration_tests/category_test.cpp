//
// Created by Daniil Ryzhkov on 03/08/2024.
//

#include "base_api_integration_test.hpp"
#include "repository/models/category.hpp"

#define ENDPOINT "/v1/categories"
#define TEST_CATEGORY "d394a832-4011-7023-c519-afe3adaf0233"

using namespace repository;

namespace api {
namespace integration_tests {

class category_test : public base_api_integration_test {
 protected:
  ::models::category create_category(const lambda::nullable<int>& version = lambda::nullable<int>()) {
    auto repo = services.get<repository::t_client>();
    auto c = ::models::category{
        .id = TEST_CATEGORY,
        .user_id = USER_ID,
        .name = "category",
        .color = 29,
        .version = version.has_value() ? version.get_value() : 0,
    };
    repo->create<::models::category>(c);
    return c;
  }
};

TEST_F(category_test, put_category) {
  // should not be created until user is created
  auto response = (*api)(create_request("PUT", ENDPOINT, R"(
{
  "id": ")" TEST_CATEGORY R"(",
  "name": "category",
  "color": 29,
  "version": 0
})"));
  assert_response(response, "400", R"({"error":4,"message":"User is not initialized"})");

  auto repo = services.get<repository::t_client>();
  auto categories = repo->select<::models::category>("select * from categories").all();

  ASSERT_EQ(categories->size(), 0);

  init_user();

  // should create category
  response = (*api)(create_request("PUT", ENDPOINT, R"(
{
  "id": ")" TEST_CATEGORY R"(",
  "name": "category",
  "color": 29,
  "version": 0
})"));
  assert_response(response, "200", "");

  categories = repo->select<::models::category>("select * from categories").all();
  ASSERT_EQ(categories->size(), 1);

  auto c = categories->at(0);
  ASSERT_EQ(c->id, TEST_CATEGORY);
  ASSERT_EQ(c->user_id, USER_ID);
  ASSERT_EQ(c->name, "category");
  ASSERT_EQ(c->color, 29);
  ASSERT_EQ(c->version, 0);
}

TEST_F(category_test, put_category_update) {
  init_user();
  create_category();

  // should update category
  auto response = (*api)(create_request("PUT", ENDPOINT, R"(
{
  "id": ")" TEST_CATEGORY R"(",
  "name": "category2",
  "color": 30,
  "version": 0
})"));
  assert_response(response, "200", "");

  auto repo = services.get<repository::t_client>();
  auto categories = repo->select<::models::category>("select * from categories").all();
  ASSERT_EQ(categories->size(), 1);

  auto c = categories->at(0);
  ASSERT_EQ(c->id, TEST_CATEGORY);
  ASSERT_EQ(c->user_id, USER_ID);
  ASSERT_EQ(c->name, "category2");
  ASSERT_EQ(c->color, 30);
  ASSERT_EQ(c->version, 1);
}

TEST_F(category_test, put_category_update_conflict) {
  init_user();
  auto c1 = create_category(1);

  // should not update category
  auto response = (*api)(create_request("PUT", ENDPOINT, R"(
{
  "id": ")" TEST_CATEGORY R"(",
  "name": "category3",
  "color": 31,
  "version": 0
})"));
  assert_response(response, "409", R"("Optimistic concurrency error")");

  auto repo = services.get<repository::t_client>();
  auto categories = repo->select<::models::category>("select * from categories").all();
  ASSERT_EQ(categories->size(), 1);

  auto c = categories->at(0);
  ASSERT_EQ(c->id, c1.id);
  ASSERT_EQ(c->user_id, c1.user_id);
  ASSERT_EQ(c->name, c1.name);
  ASSERT_EQ(c->color, c1.color);
  ASSERT_EQ(c->version, c1.version);
}

TEST_F(category_test, get_categories) {
  init_user();
  create_category();

  // should get categories
  auto response = (*api)(create_request("GET", ENDPOINT, ""));
  assert_response(response, "200", R"(
[
  {
    "color": 29,
    "id": ")" TEST_CATEGORY R"(",
    "name": "category",
    "version": 0
  }
])");
}

TEST_F(category_test, delete_category) {
  init_user();
  create_category();

  // should delete category
  auto response = (*api)(create_request("DELETE", ENDPOINT "/" TEST_CATEGORY, ""));
  assert_response(response, "200", "");

  auto repo = services.get<repository::t_client>();
  auto categories = repo->select<::models::category>("select * from categories").all();
  ASSERT_EQ(categories->size(), 0);
}

TEST_F(category_test, delete_category_not_found) {
  init_user();

  // should not delete category
  auto response = (*api)(create_request("DELETE", ENDPOINT "/" TEST_CATEGORY, ""));
  assert_response(response, "404", "");
}

}
}
