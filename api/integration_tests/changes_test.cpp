//
// Created by Daniil Ryzhkov on 06/08/2024.
//

#include "base_api_integration_test.hpp"

#define DEVICE_ID "d394a832-4011-7023-c519-afe3adaf0233"

namespace api {
namespace integration_tests {

class changes_test : public base_api_integration_test {
 protected:
  void create_device() {
    auto repo = services.get<repository::t_client>();
    auto d = repository::models::user_device{
        .id = DEVICE_ID,
        .user_id = USER_ID,
    };
    repo->create<repository::models::user_device>(d);
  }
};

static aws::lambda_runtime::invocation_request create_request(const std::string &device_id = DEVICE_ID) {
  return create_request("POST", "/v1/changes", R"({"device_id":")" + device_id + R"("})");
}

TEST_F(changes_test, should_return_receipts_list) {
  init_user();
  create_device();
  create_receipt();

  auto response = (*api)(create_request());
  assert_response(response, "200", R"({"budgets":[], "categories": [], "receipts": [
{
  "action": "create",
  "body": {
    "category":"",
    "currency":"EUR",
    "date":"2024-08-04",
    "id": ")" TEST_RECEIPT R"(",
    "imageName":"image",
    "items":[],
    "state":"done",
    "storeName":"store",
    "totalAmount":100,
    "version":0
  },
  "id": ")" TEST_RECEIPT R"("
}])");

  // events should be cleared
  auto repo = services.get<repository::t_client>();
  auto entity_events = repo->select<repository::models::entity_event>("select * from entity_events").all();
  ASSERT_EQ(entity_events->size(), 0);
}

}
}
