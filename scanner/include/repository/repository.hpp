#pragma once

#include <memory>
#include <string>
#include <map>
#include <functional>
#include <iostream>

#include <conncpp/Connection.hpp>
#include <conncpp/PreparedStatement.hpp>

#include <aws-lambda-cpp/common/logger.hpp>

#include "repository_configuration.hpp"
#include "receipt_configuration.hpp"

namespace scanner {
namespace repository {

class repository {
 public:
  repository(const std::string& connection_string,
             const std::shared_ptr<aws_lambda_cpp::common::logger>& logger);

  template <typename T>
  void create(const T& entity) {
    auto& configuration = get_configuration<T>();
    try {
      auto& stmt = configuration.get_insert_statement(entity, m_connection);
      if (!stmt) {
        m_logger->error("Unable to create prepared statement!");
        throw std::runtime_error("Unable to create prepared statement!");
      }
      m_logger->info("Executing prepared statement...");
      stmt->executeUpdate();
    } catch (std::exception& e) {
      m_logger->error("Error occured while creating entity in the database: %s",
                      e.what());
      throw;
    }
  }

  template <typename T>
  std::shared_ptr<T> get(const std::string& id) {
    auto& configuration = get_configuration<T>();
    try {
      auto& stmt = configuration.get_select_statement(id, m_connection);
      if (!stmt) {
        m_logger->error("Unable to create prepared statement!");
        throw std::runtime_error("Unable to create prepared statement!");
      }
      m_logger->info("Executing prepared statement...");
      auto result = stmt->executeQuery();
      if (result->next()) {
        return std::move(configuration.get_entity(result));
      }
      throw std::runtime_error("Entity not found!");
    } catch (std::exception& e) {
      m_logger->error(
          "Error occured while getting entity from the database: %s", e.what());
      throw;
    }
  }

  template <typename T>
  void update(const T& entity) {
    auto& configuration = get_configuration<T>();
    try {
      auto& stmt = configuration.get_update_statement(entity, m_connection);
      if (!stmt) {
        m_logger->error("Unable to create prepared statement!");
        throw std::runtime_error("Unable to create prepared statement!");
      }
      m_logger->info("Executing prepared statement...");
      stmt->executeUpdate();
    } catch (std::exception& e) {
      m_logger->error("Error occured while updating entity in the database: %s",
                      e.what());
      throw;
    }
  }

  template <typename T>
  void drop(const std::string& id) {
    auto& configuration = get_configuration<T>();
    try {
      auto& stmt = configuration.get_delete_statement(id, m_connection);
      if (!stmt) {
        m_logger->error("Unable to create prepared statement!");
        throw std::runtime_error("Unable to create prepared statement!");
      }
      m_logger->info("Executing prepared statement...");
      stmt->executeUpdate();
    } catch (std::exception& e) {
      m_logger->error("Error occured while deleting entity in the database: %s",
                      e.what());
      throw;
    }
  }

 private:
  template <typename T>
  repository_configuration<T>& get_configuration() {
    static repository_configuration<T> configuration;
    return configuration;
  }

  std::shared_ptr<sql::Connection> m_connection;
  std::shared_ptr<aws_lambda_cpp::common::logger> m_logger;
};

}  // namespace repository
}  // namespace scanner