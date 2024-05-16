#pragma once

#include <memory>
#include <string>

#include <conncpp/PreparedStatement.hpp>

#include "models/common.hpp"

namespace scanner {
namespace repository {
namespace common {

template <typename T>
class id_configuration {
 public:
  typedef const models::guid& (*id_selector_t)(const T&);
  typedef void (*id_setter_t)(T&, const models::guid&);

  id_configuration(const id_selector_t& id_selector,
                   const id_setter_t& id_setter)
      : m_id_selector(id_selector), m_id_setter(id_setter) {}

  id_configuration& with_column_name(const std::string& column_name) {
    m_column_name = column_name;
    return *this;
  }

  const std::string& get_column_name() const { return m_column_name; }

  void configure_statement(int p_number, const T& t,
                           std::shared_ptr<sql::PreparedStatement>& stmt) {
    stmt->setString(p_number, m_id_selector(t));
  }

  void set_id(T& t, sql::ResultSet* res) {
    m_id_setter(t, res->getString(m_column_name).c_str());
  }

 private:
  id_selector_t m_id_selector;
  id_setter_t m_id_setter;
  std::string m_column_name;
};

}  // namespace common
}  // namespace repository
}  // namespace scanner