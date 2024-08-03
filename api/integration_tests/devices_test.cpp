//
// Created by Daniil Ryzhkov on 03/08/2024.
//

#include "base_api_integration_test.hpp"
#include "repository/models/user_device.hpp"

#define ENDPOINT "/v1/devices"
#define TEST_DEVICE "d394a832-4011-7023-c519-afe3adaf0233"

using namespace repository;

namespace api {
namespace integration_tests {

class devices_test : public base_api_integration_test {};

TEST_F(devices_test, put_device) {
  // should not be created until user is created
  auto response = (*api)(create_request("PUT", ENDPOINT, R"({"id":")" TEST_DEVICE R"("})"));
  assert_response(response, "400", R"({"error":1,"message":"User is not initialized"})");

  auto repo = services.get<repository::t_client>();
  auto devices = repo->select<::models::user_device>("select * from user_devices").all();

  ASSERT_EQ(devices->size(), 0);

  init_user();

  // should create device
  response = (*api)(create_request("PUT", ENDPOINT, R"({"id":")" TEST_DEVICE R"("})"));
  assert_response(response, "200", "");

  devices = repo->select<::models::user_device>("select * from user_devices").all();
  ASSERT_EQ(devices->size(), 1);
  ASSERT_EQ(devices->at(0)->id, TEST_DEVICE);

  // put same device should be no-op
  response = (*api)(create_request("PUT", ENDPOINT, R"({"id":")" TEST_DEVICE R"("})"));
  assert_response(response, "200", "");

  devices = repo->select<::models::user_device>("select * from user_devices").all();
  ASSERT_EQ(devices->size(), 1);
  ASSERT_EQ(devices->at(0)->id, TEST_DEVICE);
}

}
}
