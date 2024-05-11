#pragma once

#include <memory>
#include <string>

#include <conncpp/PreparedStatement.hpp>

namespace scanner {
namespace repository {
namespace common {

template<typename T>
class id_configuration {
 public:
  typedef const std::string& (*id_selector_t)(const T&);
  
  id_configuration(const id_selector_t& id_selector)
      : m_id_selector(id_selector) {}

  id_configuration& with_column_name(const std::string& column_name) {
    m_column_name = column_name;
    return *this;
  }

  const std::string& get_column_name() const { return m_column_name; }

  void configure_statement(int p_number, const T& t,
                           std::shared_ptr<sql::PreparedStatement>& stmt) {
    stmt->setString(p_number, m_id_selector(t));
  }

 private:
  id_selector_t m_id_selector;
  std::string m_column_name;
};

}  // namespace common
}  // namespace repository
}  // namespace scanner