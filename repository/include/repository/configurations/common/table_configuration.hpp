#pragma once

#include <string>
#include <utility>

namespace repository {
namespace configurations {
namespace common {

class table_configuration {
 public:
  explicit table_configuration(std::string name) : m_name(std::move(name)) {}

  const std::string& get_name() const { return m_name; }

 private:
  std::string m_name;
};

}  // namespace common
}  // namespace configurations
}  // namespace repository