//
// Created by Daniil Ryzhkov on 31/07/2024.
//

#include <string>
#include <memory>
#include <mariadb/conncpp/PreparedStatement.hpp>

#pragma once

namespace repository {
namespace configurations {
namespace common {

template<typename T>
class version_configuration {
 public:
  typedef const int &(*version_selector_t)(const T &);
  typedef void (*version_setter_t)(T &, const int &);

  version_configuration(const version_selector_t &version_selector,
                        const version_setter_t &version_setter)
      : m_version_selector(version_selector), m_version_setter(version_setter) {}

  version_configuration &with_column_name(const std::string &column_name) {
    m_column_name = column_name;
    return *this;
  }

  const std::string &get_column_name() const { return m_column_name; }

  void configure_statement(int p_number, int version,
                           std::shared_ptr<sql::PreparedStatement> &stmt) {
    stmt->setInt(p_number, version);
  }

  void set_version(T &t, sql::ResultSet *res) {
    m_version_setter(t, res->getInt(m_column_name));
  }

  int get_version(const T &t) {
    return m_version_selector(t);
  }

 private:
  version_selector_t m_version_selector;
  version_setter_t m_version_setter;
  std::string m_column_name;
};

}  // namespace common
}  // namespace configurations
}  // namespace repository
