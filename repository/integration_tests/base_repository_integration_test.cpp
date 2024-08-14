//
// Created by Daniil Ryzhkov on 03/08/2024.
//

#include "base_repository_integration_test.hpp"

void base_repository_integration_test::SetUp() {
  repository_integration_test::SetUp();
  auto connection = get_connection();
  std::unique_ptr<sql::Statement>(connection->createStatement())->execute("insert into users (id) values ('" DEFAULT_USER_ID "')");
}
