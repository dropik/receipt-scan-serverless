//
// Created by Daniil Ryzhkov on 07/06/2024.
//

#include <repository/statement.hpp>

namespace repository {

statement::statement(std::shared_ptr<sql::PreparedStatement> stmt) : base_query(std::move(stmt)) {}

statement &statement::with_param(int t) {
  this->set_param(t);
  return *this;
}

statement &statement::with_param(double t) {
  this->set_param(t);
  return *this;
}

statement &statement::with_param(long t) {
  this->set_param(t);
  return *this;
}

statement &statement::with_param(const std::string &t) {
  this->set_param(t);
  return *this;
}

statement &statement::with_param(long double t) {
  this->set_param(t);
  return *this;
}

statement &statement::with_param(std::optional<std::string> &t) {
  this->set_param(t);
  return *this;
}

void statement::go() {
  get_stmt()->execute();
}

}
