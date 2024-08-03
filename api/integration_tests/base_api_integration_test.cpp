//
// Created by Daniil Ryzhkov on 03/08/2024.
//

#include "base_api_integration_test.hpp"
#include "../src/api.hpp"
#include "mocks/factories.hpp"

using namespace api::integration_tests;

std::shared_ptr<sql::Connection> base_api_integration_test::get_connection() {
  return services.get<repository::t_client>()->get_connection();
}
void base_api_integration_test::SetUp() {
  base_repository_integration_test::SetUp();
  api = create_api(services);
}
