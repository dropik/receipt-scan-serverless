//
// Created by Daniil Ryzhkov on 23/06/2024.
//

#include <aws/core/client/ClientConfiguration.h>
#include <thread>

#include "integration_tests_common/base_repository_integration_test.hpp"
#include "repository/client.hpp"

#include "di/container.hpp"
#include "repository/factories.hpp"

#include "repository/models/receipt.hpp"

using namespace di;
using namespace repository::models;

class client_test : public base_repository_integration_test {
 protected:
  std::shared_ptr<sql::Connection> get_connection() override {
    return services.get<repository::t_client>()->get_connection();
  }

  container<
      singleton<Aws::Client::ClientConfiguration>,
      singleton<repository::connection_settings>,
      scoped<repository::t_client, repository::client<>>
  > services;
};

TEST_F(client_test, should_reconnect_if_connection_is_lost) {
  auto client = services.get<repository::t_client>();
  auto connection = client->get_connection();
  std::unique_ptr<sql::Statement>(connection->createStatement())->execute("set session wait_timeout=1");
  std::unique_ptr<sql::Statement>(connection->createStatement())->execute("set session interactive_timeout=1");
  std::this_thread::sleep_for(std::chrono::seconds(2));
  auto results = client->select<receipt>("select * from receipts").all();
  ASSERT_EQ(results->size(), 0);
}

TEST_F(client_test, should_invalidate_get_statement_if_connection_is_lost) {
  auto client = services.get<repository::t_client>();
  auto u1 = client->get<user>(DEFAULT_USER_ID);
  auto connection = client->get_connection();
  std::unique_ptr<sql::Statement>(connection->createStatement())->execute("set session wait_timeout=1");
  std::unique_ptr<sql::Statement>(connection->createStatement())->execute("set session interactive_timeout=1");
  std::this_thread::sleep_for(std::chrono::seconds(2));
  auto u2 = client->get<user>(DEFAULT_USER_ID);
  ASSERT_EQ(u1->id, u2->id);
}

TEST_F(client_test, should_invalidate_create_statment_if_connection_is_lost) {
  auto client = services.get<repository::t_client>();
  user u1 = { "user1"};
  client->create(u1);
  auto connection = client->get_connection();
  std::unique_ptr<sql::Statement>(connection->createStatement())->execute("set session wait_timeout=1");
  std::unique_ptr<sql::Statement>(connection->createStatement())->execute("set session interactive_timeout=1");
  std::this_thread::sleep_for(std::chrono::seconds(2));
  user u2 = { "user2" };
  client->create(u2);
  ASSERT_TRUE(u1.id != u2.id);
}
