#pragma once

#include <string>
#include <memory>

#include <mariadb/conncpp/PreparedStatement.hpp>

namespace repository {
namespace configurations {
namespace common {

template<typename T>
class base_property_configuration;

template<typename T>
class base_property_configuration {
 public:
  virtual void configure_statement(
      int p_number, const T &t,
      std::shared_ptr<sql::PreparedStatement> &stmt) = 0;

  virtual void set_entity_property(T &t, sql::ResultSet *res) = 0;

  const std::string &get_column_name() const { return m_column_name; }

  base_property_configuration<T> &with_column_name(
      const std::string &column_name) {
    m_column_name = column_name;
    return *this;
  }

 private:
  std::string m_column_name;
};

}  // namespace common
}  // namespace configurations
}  // namespace repository
