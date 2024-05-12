#pragma once

#include <memory>

#include <conncpp/PreparedStatement.hpp>

#include "repository/repository_configuration.hpp"

namespace scanner {
namespace repository {
namespace common {

template <typename T>
class selector {
 public:
  selector(std::shared_ptr<sql::PreparedStatement> stmt,
           const repository_configuration<T>& configuration)
      : m_stmt(stmt), m_configuration(configuration) {}

  template <typename TProperty>
  selector& with_param(const TProperty& t) {
    m_stmt->setString(m_param_index++, t);
    return *this;
  }

  selector& with_param(int t) {
    m_stmt->setInt(m_param_index++, t);
    return *this;
  }

  selector& with_param(double t) {
    m_stmt->setDouble(m_param_index++, t);
    return *this;
  }

  selector& with_param(long t) {
    m_stmt->setInt64(m_param_index++, t);
    return *this;
  }

  selector& with_param(const std::string& t) {
    m_stmt->setString(m_param_index++, t);
    return *this;
  }

  selector& with_param(long double t) {
    m_stmt->setDouble(m_param_index++, t);
    return *this;
  }

  std::shared_ptr<T> first_or_default() {
    auto result = m_stmt->executeQuery();
    if (result->next()) {
      return std::move(m_configuration.get_entity(result));
    }
    return nullptr;
  }

 private:
  int m_param_index = 1;
  std::shared_ptr<sql::PreparedStatement> m_stmt;
  repository_configuration<T> m_configuration;
};

}  // namespace common
}  // namespace repository
}  // namespace scanner