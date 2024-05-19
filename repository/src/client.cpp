#include "repository/client.hpp"

#include <utility>

#include <mariadb/conncpp/DriverManager.hpp>

using namespace repository;

client::client(
    const std::string& connection_string,
    std::shared_ptr<aws_lambda_cpp::common::logger> logger)
    : m_logger(std::move(logger)) {
  try {
    m_logger->info("Establishing connection with the database...");

    sql::SQLString url(connection_string);
    std::unique_ptr<sql::Connection> conn(
        sql::DriverManager::getConnection(url));
    if (conn == nullptr) {
      m_logger->error("Unable to establish connection with database!");
      throw std::runtime_error("Unable to establish connection with database!");
    }
    m_connection = std::move(conn);
  } catch (std::exception& e) {
    m_logger->error(
        "Error occurred while establishing connection with the database: %s",
        e.what());
    throw;
  }
}

client::~client() {
  if (m_connection) {
    m_logger->info("Closing connection with the database...");
    m_connection->close();
  }
}
