//
// Created by Daniil Ryzhkov on 07/06/2024.
//

#include <repository/base_query.hpp>

namespace repository {

base_query::base_query(std::shared_ptr<sql::PreparedStatement> stmt) : m_stmt(std::move(stmt)) {}

void base_query::set_param(int t) {
  m_stmt->setInt(m_param_index++, t);
}

void base_query::set_param(double t) {
  m_stmt->setDouble(m_param_index++, t);
}

void base_query::set_param(long t) {
  m_stmt->setInt64(m_param_index++, t);
}

void base_query::set_param(const std::string &t) {
  m_stmt->setString(m_param_index++, t);
}

void base_query::set_param(long double t) {
  m_stmt->setDouble(m_param_index++, (double) t);
}

std::shared_ptr<sql::PreparedStatement> base_query::get_stmt() {
  return m_stmt;
}

}
