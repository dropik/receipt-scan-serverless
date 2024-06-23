//
// Created by Daniil Ryzhkov on 23/06/2024.
//

#include "base_repository_integration_test.hpp"

void base_repository_integration_test::SetUp() {
  auto connection = get_connection();
  std::unique_ptr<sql::Statement>(connection->createStatement())->execute("drop database if exists receipt_scan_test");
  std::unique_ptr<sql::Statement>(connection->createStatement())->execute("create database receipt_scan_test");
  std::unique_ptr<sql::Statement>(connection->createStatement())->execute("use receipt_scan_test");

  // get working directory
  std::string pwd = getenv("PWD");
  if (pwd.empty()) {
    throw std::runtime_error("Failed to get working directory");
  }
  lambda::log.info("Working directory: %s", pwd.c_str());

  std::ifstream file("database/v1.0.0.sql");
  if (!file.is_open()) {
    throw std::runtime_error("Failed to open database migration file");
  }
  std::string line;
  std::string sql;
  while (std::getline(file, line)) {
    sql += line + "\n";
  }

  std::vector<std::string> statements;
  std::string delimiter = "DELIMITER //";
  std::string switch_delimiter = "DELIMITER ;";
  std::string normal_delimiter = ";";
  std::string alt_delimiter = "//";
  size_t pos = 0;
  while ((pos = sql.find(delimiter)) != std::string::npos) {
    std::string normal_delimiter_sql = sql.substr(0, pos);
    size_t delimiter_pos = 0;
    while ((delimiter_pos = normal_delimiter_sql.find(normal_delimiter)) != std::string::npos) {
      statements.push_back(normal_delimiter_sql.substr(0, delimiter_pos));
      normal_delimiter_sql.erase(0, delimiter_pos + normal_delimiter.length());
    }
    sql.erase(0, pos + delimiter.length());

    size_t switch_pos = sql.find(switch_delimiter);
    std::string alt_delimiter_sql = sql.substr(0, switch_pos);
    sql.erase(0, switch_pos + switch_delimiter.length());

    size_t alt_pos = 0;
    while ((alt_pos = alt_delimiter_sql.find(alt_delimiter)) != std::string::npos) {
      statements.push_back(alt_delimiter_sql.substr(0, alt_pos));
      alt_delimiter_sql.erase(0, alt_pos + alt_delimiter.length());
    }
  }
  while((pos = sql.find(normal_delimiter)) != std::string::npos) {
    statements.push_back(sql.substr(0, pos));
    sql.erase(0, pos + normal_delimiter.length());
  }

  for (const auto &statement : statements) {
    std::unique_ptr<sql::Statement> stmt(connection->createStatement());
    std::string trimmed_statement = statement;
    trimmed_statement.erase(0, trimmed_statement.find_first_not_of(" \n\r\t"));
    trimmed_statement.erase(trimmed_statement.find_last_not_of(" \n\r\t") + 1);
    stmt->execute(trimmed_statement);
    while (stmt->getMoreResults()) {
      // do nothing
    }
  }

  std::unique_ptr<sql::Statement>(connection->createStatement())->execute("insert into users (id) values ('" DEFAULT_USER_ID "')");
}

void base_repository_integration_test::TearDown() {
  auto connection = get_connection();
  std::unique_ptr<sql::Statement>(connection->createStatement())->execute("drop database if exists receipt_scan_test");
}
