#pragma once

#include <string>

namespace scanner {
namespace repository {
namespace common {

class table_configuration {
 public:
  table_configuration(const std::string& name) : m_name(name) {}

  const std::string& get_name() const { return m_name; }

 private:
  std::string m_name;
};

}  // namespace common
}  // namespace repository
}  // namespace scanner