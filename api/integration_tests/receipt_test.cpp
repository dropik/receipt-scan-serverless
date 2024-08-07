//
// Created by Daniil Ryzhkov on 04/08/2024.
//

#include "base_api_integration_test.hpp"
#include "repository/models/receipt.hpp"

#define ENDPOINT "/v1/receipts"

using namespace repository;

namespace api {
namespace integration_tests {

class receipt_test : public base_api_integration_test {
 protected:

  const std::string receipt_done = ::models::receipt::done;
};

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
  ASSERT_EQ(r->state, receipt_done);
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
  "version": 0,
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
  ASSERT_EQ(r->state, receipt_done);
  ASSERT_EQ(r->image_name, "image");
  ASSERT_EQ(r->version, 1);

  auto items = repo->select<::models::receipt_item>("select * from receipt_items").all();
  ASSERT_EQ(items->size(), 0);
}

TEST_F(receipt_test, put_receipt_update_conflict) {
  init_user();
  auto r1 = create_receipt(1);
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
  "url": "https://s3.amazonaws.com/test-bucket/users/)" USER_ID R"(/receipts/image"
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
  r->is_deleted = true;
  repo->update(*r);

  auto response = (*api)(create_request("GET", ENDPOINT "/" TEST_RECEIPT "/image", ""));
  assert_response(response, "404", "");
}

TEST_F(receipt_test, post_receipt_image) {
  init_user();
  create_receipt();
  create_receipt_item(0);

  auto response = (*api)(create_request("POST", ENDPOINT "/" TEST_RECEIPT "/image", ""));
  assert_response(response, "200", R"(
{
    "url": "https://s3.amazonaws.com/test-bucket/users/)" USER_ID R"(/receipts/image"
}
)");
}

TEST_F(receipt_test, post_receipt_image_not_found) {
  init_user();
  create_receipt();
  create_receipt_item(0);

  auto response = (*api)(create_request("POST", ENDPOINT "/" TEST_RECEIPT "asdf/image", ""));
  assert_response(response, "404", "");
}

}
}
