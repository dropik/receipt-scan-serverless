#include "repository/client.hpp"

#include <utility>

#include <mariadb/conncpp/DriverManager.hpp>
#include <aws/ssm/SSMClient.h>

#include <lambda/string_utils.hpp>
#include <aws/ssm/model/GetParameterRequest.h>

using namespace repository;
using namespace lambda;

client::client(
    const std::string& connection_string,
    std::shared_ptr<lambda::logger> logger)
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

statement client::execute(const std::string &query) {
  m_logger->info("Executing query: %s", query.c_str());
  try {
    std::shared_ptr<sql::PreparedStatement> stmt(m_connection->prepareStatement(query));
    stmt->closeOnCompletion();
    if (!stmt) {
      throw std::runtime_error("Unable to create prepared statement!");
    }
    return statement(stmt);
  } catch (std::exception& e) {
    m_logger->error(
        "Error occurred while preparing query: %s",
        e.what());
    throw;
  }
}

std::string repository::get_connection_string(const std::string &stage, const Aws::Client::ClientConfiguration &config) {
  auto conn_env = getenv("DB_CONNECTION_STRING");
  if (conn_env != nullptr) {
    return conn_env;
  }

  std::string ssmPrefix = string::format("/receipt-scan/%s", stage.c_str());
  Aws::SSM::SSMClient ssmClient(config);
  Aws::SSM::Model::GetParameterRequest connStrReq;
  connStrReq
      .WithName(string::format("%s/db-connection-string", ssmPrefix.c_str()))
      .WithWithDecryption(true);
  Aws::SSM::Model::GetParameterOutcome outcome =
      ssmClient.GetParameter(connStrReq);
  if (!outcome.IsSuccess()) {
    throw std::runtime_error(
        string::format("Error occurred while obtaining parameter from ssm: %s",
                   outcome.GetError().GetMessage().c_str()));
  }
  return outcome.GetResult().GetParameter().GetValue();
}
