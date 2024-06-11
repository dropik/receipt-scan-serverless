//
// Created by Daniil Ryzhkov on 07/06/2024.
//

#pragma once

#include <memory>

#include <mariadb/conncpp/PreparedStatement.hpp>

namespace repository {

class base_query {
 public:
  explicit base_query(std::shared_ptr<sql::PreparedStatement> stmt);

  void set_param(int t);
  void set_param(double t);
  void set_param(long t);
  void set_param(const std::string &t);
  void set_param(long double t);

 protected:
  std::shared_ptr<sql::PreparedStatement> get_stmt();

 private:
  int m_param_index = 1;
  std::shared_ptr<sql::PreparedStatement> m_stmt;
};

}
