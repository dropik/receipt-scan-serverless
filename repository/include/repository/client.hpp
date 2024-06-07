#pragma once

#include <iostream>
#include <map>
#include <memory>
#include <string>

#include <mariadb/conncpp/Connection.hpp>
#include <mariadb/conncpp/PreparedStatement.hpp>

#include <aws-lambda-cpp/common/logger.hpp>

#include <aws/core/client/ClientConfiguration.h>

#include "repository/configurations/repository_configuration.hpp"
#include "selector.hpp"

#include "repository/configurations/category_configuration.hpp"
#include "repository/configurations/receipt_configuration.hpp"
#include "repository/configurations/receipt_item_configuration.hpp"
#include "repository/configurations/user_configuration.hpp"
#include "statement.hpp"

namespace repository {

std::string get_connection_string(const std::string& stage, const Aws::Client::ClientConfiguration& config);

class client {
 public:
  client(const std::string& connection_string,
         std::shared_ptr<aws_lambda_cpp::common::logger> logger);
  ~client();

  template <typename T>
  void create(const T& entity) {
    auto& configuration = get_configuration<T>();
    m_logger->info("Inserting in %s...", configuration.get_table_name().c_str());
    try {
      auto& stmt = configuration.get_insert_statement(entity, m_connection);
      if (!stmt) {
        throw std::runtime_error("Unable to create prepared statement!");
      }
      stmt->executeUpdate();
    } catch (std::exception& e) {
      m_logger->error("Error occurred while creating entity in the database: %s",
                      e.what());
      throw;
    }
  }

  template <typename T>
  std::shared_ptr<T> get(const models::guid& id) {
    auto& configuration = get_configuration<T>();
    m_logger->info("Getting entity from %s...", configuration.get_table_name().c_str());
    try {
      auto& stmt = configuration.get_select_statement(id, m_connection);
      if (!stmt) {
        throw std::runtime_error("Unable to create prepared statement!");
      }
      auto result = stmt->executeQuery();
      if (result->next()) {
        return std::move(configuration.get_entity(result));
      }
      throw std::runtime_error("Entity not found!");
    } catch (std::exception& e) {
      m_logger->error(
          "Error occurred while getting entity from the database: %s", e.what());
      throw;
    }
  }

  template <typename T>
  void update(const T& entity) {
    auto& configuration = get_configuration<T>();
    m_logger->info("Updating in %s...", configuration.get_table_name().c_str());
    try {
      auto& stmt = configuration.get_update_statement(entity, m_connection);
      if (!stmt) {
        throw std::runtime_error("Unable to create prepared statement!");
      }
      stmt->executeUpdate();
    } catch (std::exception& e) {
      m_logger->error("Error occurred while updating entity in the database: %s",
                      e.what());
      throw;
    }
  }

  template <typename T>
  void drop(const models::guid& id) {
    auto& configuration = get_configuration<T>();
    m_logger->info("Deleting from %s...", configuration.get_table_name().c_str());
    try {
      auto& stmt = configuration.get_delete_statement(id, m_connection);
      if (!stmt) {
        throw std::runtime_error("Unable to create prepared statement!");
      }
      stmt->executeUpdate();
    } catch (std::exception& e) {
      m_logger->error("Error occurred while deleting entity in the database: %s",
                      e.what());
      throw;
    }
  }

  template <typename T>
  selector<T> select(const std::string& query) {
    auto& configuration = get_configuration<T>();
    m_logger->info("Executing query: %s", query.c_str());
    try {
      std::shared_ptr<sql::PreparedStatement> stmt(m_connection->prepareStatement(query));
      stmt->closeOnCompletion();
      if (!stmt) {
        throw std::runtime_error("Unable to create prepared statement!");
      }
      return selector<T>(stmt, configuration);
    } catch (std::exception& e) {
      m_logger->error(
          "Error occurred while preparing query: %s",
          e.what());
      throw;
    }
  }

  statement execute(const std::string& query);

 private:
  template <typename T>
  configurations::repository_configuration<T>& get_configuration() {
    static configurations::repository_configuration<T> configuration;
    return configuration;
  }

  std::shared_ptr<sql::Connection> m_connection;
  std::shared_ptr<aws_lambda_cpp::common::logger> m_logger;
};

}  // namespace client
