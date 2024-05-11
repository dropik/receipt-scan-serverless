#include "repository/repository.hpp"

#include <conncpp/DriverManager.hpp>

using namespace scanner::repository;

repository::repository(
    const std::string& connection_string,
    const std::shared_ptr<aws_lambda_cpp::common::logger>& logger)
    : m_logger(logger) {
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
        "Error occured while establishing connection with the database: %s",
        e.what());
    throw;
  }
}
