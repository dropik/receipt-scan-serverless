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

}
}
