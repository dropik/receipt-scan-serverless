//
// Created by Daniil Ryzhkov on 04/08/2024.
//

#include <chrono>
#include "base_api_integration_test.hpp"
#include "repository/models/receipt.hpp"
#include "lambda/utils.hpp"

#define ENDPOINT "/v1/receipts"

using namespace repository;

namespace api::integration_tests {

class receipt_test : public base_api_integration_test {};

TEST_F(receipt_test, put_receipt) {
  // should not be created until user is created
  auto response = (*api)(create_request("PUT", ENDPOINT, R"(
{
  "id": ")" TEST_RECEIPT R"(",
  "date": "2024-08-04",
  "totalAmount": 100,
  "currency": "EUR",
  "storeName": "store",
  "category": "",
  "state": "done",
  "imageName": "image",
  "version": 0,
  "items": [
    {
      "id": "d394a832-4011-7023-c519-afe3adaf0233",
      "description": "item",
      "amount": 100,
      "category": "supermarket"
    }
  ]
})"));
  assert_response(response, "400", R"({"error":4,"message":"User is not initialized"})");

  auto repo = services.get<repository::t_client>();
  auto receipts = repo->select<::models::receipt>("select * from receipts").all();

  ASSERT_EQ(receipts->size(), 0);

  init_user();

  // should create receipt
  response = (*api)(create_request("PUT", ENDPOINT, R"(
{
  "id": ")" TEST_RECEIPT R"(",
  "date": "2024-08-04",
  "totalAmount": 100,
  "currency": "EUR",
  "storeName": "store",
  "category": "",
  "state": "done",
  "imageName": "image",
  "version": 0,
  "items": [
    {
      "id": "d394a832-4011-7023-c519-afe3adaf0233",
      "description": "item",
      "amount": 100,
      "category": "supermarket"
    }
  ]
})"));
  assert_response(response, "200", "");

  receipts = repo->select<::models::receipt>("select * from receipts").all();
  ASSERT_EQ(receipts->size(), 1);

  auto r = receipts->at(0);
  ASSERT_EQ(r->id, TEST_RECEIPT);
  ASSERT_EQ(r->user_id, USER_ID);
  ASSERT_EQ(r->date, "2024-08-04");
  ASSERT_EQ(r->total_amount, 100);
  ASSERT_EQ(r->currency, "EUR");
  ASSERT_EQ(r->store_name, "store");
  ASSERT_EQ(r->category, "");
  ASSERT_EQ(r->state, ::models::receipt::done);
  ASSERT_EQ(r->image_name, "image");
  ASSERT_EQ(r->version, 0);

  auto items = repo->select<::models::receipt_item>("select * from receipt_items").all();
  ASSERT_EQ(items->size(), 1);

  auto i = items->at(0);
  ASSERT_EQ(i->id, "d394a832-4011-7023-c519-afe3adaf0233");
  ASSERT_EQ(i->receipt_id, TEST_RECEIPT);
  ASSERT_EQ(i->description, "item");
  ASSERT_EQ(i->amount, 100);
  ASSERT_EQ(i->category, "supermarket");
  ASSERT_EQ(i->sort_order, 0);
}

TEST_F(receipt_test, put_receipt_update) {
  init_user();
  create_receipt();
  create_receipt_item(0);

  auto response = (*api)(create_request("PUT", ENDPOINT, R"(
{
  "id": ")" TEST_RECEIPT R"(",
  "date": "2024-08-04",
  "totalAmount": 100,
  "currency": "EUR",
  "storeName": "store",
  "category": "",
  "state": "done",
  "imageName": "image",
  "version": 1,
  "items": []
})"));
  assert_response(response, "200", "");

  auto repo = services.get<repository::t_client>();
  auto receipts = repo->select<::models::receipt>("select * from receipts").all();
  ASSERT_EQ(receipts->size(), 1);

  auto r = receipts->at(0);
  ASSERT_EQ(r->id, TEST_RECEIPT);
  ASSERT_EQ(r->user_id, USER_ID);
  ASSERT_EQ(r->date, "2024-08-04");
  ASSERT_EQ(r->total_amount, 100);
  ASSERT_EQ(r->currency, "EUR");
  ASSERT_EQ(r->store_name, "store");
  ASSERT_EQ(r->category, "");
  ASSERT_EQ(r->state, ::models::receipt::done);
  ASSERT_EQ(r->image_name, "image");
  ASSERT_EQ(r->version, 1);

  auto items = repo->select<::models::receipt_item>("select * from receipt_items").all();
  ASSERT_EQ(items->size(), 0);
}

TEST_F(receipt_test, put_receipt_update_conflict) {
  init_user();
  auto r1 = create_receipt();
  create_receipt_item(0);

  auto response = (*api)(create_request("PUT", ENDPOINT, R"(
{
  "id": ")" TEST_RECEIPT R"(",
  "date": "2024-08-04",
  "totalAmount": 100,
  "currency": "EUR",
  "storeName": "store",
  "category": "",
  "state": "done",
  "imageName": "image",
  "version": 0,
  "items": []
})"));
  assert_response(response, "409", R"("Optimistic concurrency error")");

  auto repo = services.get<repository::t_client>();
  auto items = repo->select<::models::receipt_item>("select * from receipt_items").all();
  ASSERT_EQ(items->size(), 1);
}

TEST_F(receipt_test, get_receipts_by_month) {
  init_user();
  create_receipt();
  auto i = create_receipt_item(0);

  auto response = (*api)(create_request("GET", ENDPOINT "/years/2024/months/8", ""));
  assert_response(response, "200", lambda::string::format(R"([
  {
    "categories": ["supermarket"],
    "currency": "EUR",
    "date": "2024-08-04",
    "id": "d394a832-4011-7023-c519-afe3adaf0233",
    "imageName": "image",
    "items": [
      {
        "amount": 100,
        "category": "supermarket",
        "description": "item",
        "id": "%s"
      }
    ],
    "state": "done",
    "storeName": "store",
    "totalAmount": 100,
    "version": 0
  }
])", i.id.c_str()));

  response = (*api)(create_request("GET", ENDPOINT "/years/2024/months/7", ""));
  assert_response(response, "200", "[]");
}

TEST_F(receipt_test, get_receipts_by_month_deleted) {
  init_user();
  create_receipt();
  create_receipt_item(0);

  auto repo = services.get<repository::t_client>();
  auto receipts = repo->select<::models::receipt>("select * from receipts").all();
  auto r = receipts->at(0);
  r->is_deleted = true;
  r->version++;
  repo->update(*r);

  auto response = (*api)(create_request("GET", ENDPOINT "/years/2024/months/8", ""));
  assert_response(response, "200", "[]");
}

TEST_F(receipt_test, delete_receipt) {
  init_user();
  create_receipt();
  create_receipt_item(0);

  auto response = (*api)(create_request("DELETE", ENDPOINT "/" TEST_RECEIPT, ""));
  assert_response(response, "200", "");

  auto repo = services.get<repository::t_client>();
  auto receipts = repo->select<::models::receipt>("select * from receipts").all();
  ASSERT_EQ(receipts->size(), 1);
  ASSERT_EQ(receipts->at(0)->is_deleted, true);

  auto items = repo->select<::models::receipt_item>("select * from receipt_items").all();
  ASSERT_EQ(items->size(), 1);
}

TEST_F(receipt_test, delete_receipt_not_found) {
  init_user();
  create_receipt();
  create_receipt_item(0);

  auto response = (*api)(create_request("DELETE", ENDPOINT "/" TEST_RECEIPT "asdf", ""));
  assert_response(response, "404", "");

  auto repo = services.get<repository::t_client>();
  auto receipts = repo->select<::models::receipt>("select * from receipts").all();
  ASSERT_EQ(receipts->size(), 1);
}

TEST_F(receipt_test, get_receipt_image) {
  init_user();
  create_receipt();
  create_receipt_item(0);

  auto response = (*api)(create_request("GET", ENDPOINT "/" TEST_RECEIPT "/image", ""));
  assert_response(response, "200", R"(
{
  "auth_url": "https://s3.amazonaws.com/test-bucket/users/)" USER_ID R"(/receipts/image"
}
)");
}

TEST_F(receipt_test, get_receipt_image_not_found) {
  init_user();
  create_receipt();
  create_receipt_item(0);

  auto response = (*api)(create_request("GET", ENDPOINT "/" TEST_RECEIPT "asdf/image", ""));
  assert_response(response, "404", "");
}

TEST_F(receipt_test, get_receipt_image_deleted) {
  init_user();
  create_receipt();
  create_receipt_item(0);

  auto repo = services.get<repository::t_client>();
  auto receipts = repo->select<::models::receipt>("select * from receipts").all();
  auto r = receipts->at(0);
  r->version++;
  r->is_deleted = true;
  repo->update(*r);

  auto response = (*api)(create_request("GET", ENDPOINT "/" TEST_RECEIPT "/image", ""));
  assert_response(response, "404", "");
}

std::string gen_timestamp(int shift) {
  auto now = std::time(nullptr);
  auto now_tm = std::gmtime(&now);
  now_tm->tm_sec += shift;
  std::mktime(now_tm);
  std::stringstream ss;
  ss << std::put_time(now_tm, "%Y-%m-%d %H:%M:%S");
  return ss.str();
}

TEST_F(receipt_test, post_receipt_image) {
  init_user(true, {}, {}, gen_timestamp(60));
  create_receipt();
  create_receipt_item(0);

  auto response = (*api)(create_request("POST", ENDPOINT "/" TEST_RECEIPT "/image", ""));
  assert_response(response, "200", R"(
{
    "auth_url": "https://s3.amazonaws.com/test-bucket/users/)" USER_ID R"(/receipts/image"
}
)");
}

TEST_F(receipt_test, post_receipt_image_forbidden_if_no_subscription) {
  init_user();
  create_receipt();
  create_receipt_item(0);

  // should not grant access if no subscription
  auto response = (*api)(create_request("POST", ENDPOINT "/" TEST_RECEIPT "/image", ""));
  assert_response(response, "403", "");

  auto client = services.get<repository::t_client>();
  auto user = client->get<::models::user>(USER_ID);
  user->has_subscription = true;
  client->update(*user);

  // should not grant access since no expiry time
  response = (*api)(create_request("POST", ENDPOINT "/" TEST_RECEIPT "/image", ""));
  assert_response(response, "403", "");

  user->subscription_expiry_time = gen_timestamp(-60);
  client->update(*user);

  //should not grant access since subscription expired
  response = (*api)(create_request("POST", ENDPOINT "/" TEST_RECEIPT "/image", ""));
  assert_response(response, "403", "");
}

TEST_F(receipt_test, post_receipt_image_not_found) {
  init_user(true, {}, {}, gen_timestamp(60));
  create_receipt();
  create_receipt_item(0);

  auto response = (*api)(create_request("POST", ENDPOINT "/" TEST_RECEIPT "asdf/image", ""));
  assert_response(response, "404", "");
}

TEST_F(receipt_test, get_changes_should_return_empty_list) {
  init_user();

  auto today = lambda::utils::today();
  auto response = (*api)(create_request("GET", (ENDPOINT "/changes?from=") + today + "T00:00:00Z", ""));
  assert_response(response, "200", R"([])");
}

TEST_F(receipt_test, get_changes_should_return_receipts) {
  init_user();
  auto r = create_receipt();
  auto ri = create_receipt_item(0);

  auto today = lambda::utils::today();
  auto response = (*api)(create_request("GET", (ENDPOINT "/changes?from=") + today + "T00:00:00Z", ""));
  assert_response(response, "200", lambda::string::format(R"([
{
  "action": "create",
  "body": {
    "categories":["supermarket"],
    "currency":"EUR",
    "date":"2024-08-04",
    "id": ")" TEST_RECEIPT R"(",
    "imageName":"image",
    "items":[
      {
        "amount":100,
        "category":"supermarket",
        "description":"item",
        "id": "%s"
      }
    ],
    "state":"done",
    "storeName":"store",
    "totalAmount":100,
    "version":0
  },
  "id": ")" TEST_RECEIPT R"("
}])", ri.id.c_str()));
}

TEST_F(receipt_test, get_changes_should_return_update) {
  init_user();
  auto r = create_receipt();
  auto ri = create_receipt_item(0);
  auto repo = services.get<repository::t_client>();
  r.total_amount = 120;
  r.version++;
  repo->update(r);

  auto today = lambda::utils::today();
  auto response = (*api)(create_request("GET", (ENDPOINT "/changes?from=") + today + "T00:00:00Z", ""));
  assert_response(response, "200",  lambda::string::format(R"([
{
  "action": "update",
  "body": {
    "categories":["supermarket"],
    "currency":"EUR",
    "date":"2024-08-04",
    "id": ")" TEST_RECEIPT R"(",
    "imageName":"image",
    "items":[
      {
        "amount":100,
        "category":"supermarket",
        "description":"item",
        "id": "%s"
      }
    ],
    "state":"done",
    "storeName":"store",
    "totalAmount":120,
    "version":1
  },
  "id": ")" TEST_RECEIPT R"("
}])", ri.id.c_str()));
}

TEST_F(receipt_test, get_changes_should_return_delete) {
  init_user();
  auto r = create_receipt();
  create_receipt_item(0);
  auto repo = services.get<repository::t_client>();
  r.total_amount = 120;
  r.version++;
  r.is_deleted = true;
  repo->update(r);

  auto today = lambda::utils::today();
  auto response = (*api)(create_request("GET", (ENDPOINT "/changes?from=") + today + "T00:00:00Z", ""));
  assert_response(response, "200", R"([
{
  "action": "delete",
  "body": null,
  "id": ")" TEST_RECEIPT R"("
}])");
}

TEST_F(receipt_test, get_changes_should_merge_receipt_categories) {
  init_user();
  auto r = create_receipt();
  auto ri1 = create_receipt_item(0);
  auto ri2 = create_receipt_item(1);

  auto today = lambda::utils::today();
  auto response = (*api)(create_request("GET", (ENDPOINT "/changes?from=") + today + "T00:00:00Z", ""));
  assert_response(response, "200", lambda::string::format(R"([
{
  "action": "create",
  "body": {
    "categories":["supermarket"],
    "currency":"EUR",
    "date":"2024-08-04",
    "id": ")" TEST_RECEIPT R"(",
    "imageName":"image",
    "items":[
      {
        "amount":100,
        "category":"supermarket",
        "description":"item",
        "id": "%s"
      },
      {
        "amount":100,
        "category":"supermarket",
        "description":"item",
        "id": "%s"
      }
    ],
    "state":"done",
    "storeName":"store",
    "totalAmount":100,
    "version":0
  },
  "id": ")" TEST_RECEIPT R"("
}])", ri1.id.c_str(), ri2.id.c_str()));
}

}
