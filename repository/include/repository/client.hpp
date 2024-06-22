#pragma once

#include <iostream>
#include <map>
#include <memory>
#include <string>

#include <mariadb/conncpp/Connection.hpp>
#include <mariadb/conncpp/PreparedStatement.hpp>
#include <mariadb/conncpp/DriverManager.hpp>

#include <lambda/log.hpp>

#include <aws/core/client/ClientConfiguration.h>

#include "repository/configurations/repository_configuration.hpp"
#include "selector.hpp"
#include "statement.hpp"
#include "connection_settings.hpp"

#include "repository/configurations/category_configuration.hpp"
#include "repository/configurations/receipt_configuration.hpp"
#include "repository/configurations/receipt_item_configuration.hpp"
#include "repository/configurations/user_configuration.hpp"
#include "repository/configurations/receipt_file_configuration.hpp"

namespace repository {

std::string get_connection_string(const std::string &stage, const Aws::Client::ClientConfiguration &config);

struct t_client {};

template<
    typename TSettings = const connection_settings>
class client {
 public:
  explicit client(TSettings settings) {
    try {
      lambda::log.info("Establishing connection with the database...");

      sql::SQLString url(settings->connection_string);
      std::unique_ptr<sql::Connection> conn(
          sql::DriverManager::getConnection(url));
      if (conn == nullptr) {
        lambda::log.error("Unable to establish connection with database!");
        throw std::runtime_error("Unable to establish connection with database!");
      }
      m_connection = std::move(conn);
    } catch (std::exception &e) {
      lambda::log.error(
          "Error occurred while establishing connection with the database: %s",
          e.what());
      throw;
    }
  }

  ~client() {
    if (m_connection) {
      lambda::log.info("Closing connection with the database...");
      m_connection->close();
    }
  }

  template<typename T>
  void create(const T &entity) {
    auto &configuration = get_configuration<T>();
    lambda::log.info("Inserting in %s...", configuration.get_table_name().c_str());
    try {
      auto &stmt = configuration.get_insert_statement(entity, m_connection);
      if (!stmt) {
        throw std::runtime_error("Unable to create prepared statement!");
      }
      stmt->executeUpdate();
    } catch (std::exception &e) {
      lambda::log.error("Error occurred while creating entity in the database: %s",
                      e.what());
      throw;
    }
  }

  template<typename T>
  std::shared_ptr<T> get(const models::guid &id) {
    auto &configuration = get_configuration<T>();
    lambda::log.info("Getting entity from %s...", configuration.get_table_name().c_str());
    try {
      auto &stmt = configuration.get_select_statement(id, m_connection);
      if (!stmt) {
        throw std::runtime_error("Unable to create prepared statement!");
      }
      auto result = stmt->executeQuery();
      if (result->next()) {
        return std::move(configuration.get_entity(result));
      }
      throw std::runtime_error("Entity not found!");
    } catch (std::exception &e) {
      lambda::log.error(
          "Error occurred while getting entity from the database: %s", e.what());
      throw;
    }
  }

  template<typename T>
  void update(const T &entity) {
    auto &configuration = get_configuration<T>();
    lambda::log.info("Updating in %s...", configuration.get_table_name().c_str());
    try {
      auto &stmt = configuration.get_update_statement(entity, m_connection);
      if (!stmt) {
        throw std::runtime_error("Unable to create prepared statement!");
      }
      stmt->executeUpdate();
    } catch (std::exception &e) {
      lambda::log.error("Error occurred while updating entity in the database: %s",
                      e.what());
      throw;
    }
  }

  template<typename T>
  void drop(const models::guid &id) {
    auto &configuration = get_configuration<T>();
    lambda::log.info("Deleting from %s...", configuration.get_table_name().c_str());
    try {
      auto &stmt = configuration.get_delete_statement(id, m_connection);
      if (!stmt) {
        throw std::runtime_error("Unable to create prepared statement!");
      }
      stmt->executeUpdate();
    } catch (std::exception &e) {
      lambda::log.error("Error occurred while deleting entity in the database: %s",
                      e.what());
      throw;
    }
  }

  template<typename T>
  void drop(const std::shared_ptr<T> &entity) {
    drop<T>(entity->id);
  }

  template<typename T>
  selector<T> select(const std::string &query) {
    auto &configuration = get_configuration<T>();
    lambda::log.info("Executing query: %s", query.c_str());
    try {
      std::shared_ptr<sql::PreparedStatement> stmt(m_connection->prepareStatement(query));
      stmt->closeOnCompletion();
      if (!stmt) {
        throw std::runtime_error("Unable to create prepared statement!");
      }
      return selector<T>(stmt, configuration);
    } catch (std::exception &e) {
      lambda::log.error(
          "Error occurred while preparing query: %s",
          e.what());
      throw;
    }
  }

  statement execute(const std::string &query) {
    lambda::log.info("Executing query: %s", query.c_str());
    try {
      std::shared_ptr<sql::PreparedStatement> stmt(m_connection->prepareStatement(query));
      stmt->closeOnCompletion();
      if (!stmt) {
        throw std::runtime_error("Unable to create prepared statement!");
      }
      return statement(stmt);
    } catch (std::exception &e) {
      lambda::log.error(
          "Error occurred while preparing query: %s",
          e.what());
      throw;
    }
  }

 private:
  template<typename T>
  configurations::repository_configuration<T> &get_configuration() {
    static configurations::repository_configuration<T> configuration;
    return configuration;
  }

  std::shared_ptr<sql::Connection> m_connection;
};

} // namespace client
