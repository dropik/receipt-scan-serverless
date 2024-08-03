//
// Created by Daniil Ryzhkov on 03/08/2024.
//

#include "base_api_integration_test.hpp"

using namespace api::integration_tests;

std::shared_ptr<sql::Connection> base_api_integration_test::get_connection() {
  return services.get<repository::t_client>()->get_connection();
}
