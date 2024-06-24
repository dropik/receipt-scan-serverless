#pragma once

#include <memory>

#include <mariadb/conncpp/PreparedStatement.hpp>
#include <utility>

#include "base_query.hpp"
#include "configurations/repository_configuration.hpp"

namespace repository {

template <typename T>
class selector : public base_query {
 public:
  selector(std::shared_ptr<sql::PreparedStatement> stmt,
           const configurations::repository_configuration<T>& configuration)
      : m_configuration(configuration), base_query(std::move(stmt)) {}

  auto& with_param(int t) {
    this->set_param(t);
    return *this;
  }

  auto& with_param(double t) {
    this->set_param(t);
    return *this;
  }

  auto& with_param(long t) {
    this->set_param(t);
    return *this;
  }

  auto& with_param(const std::string &t) {
    this->set_param(t);
    return *this;
  }

  auto& with_param(long double t) {
    this->set_param(t);
    return *this;
  }

  std::shared_ptr<T> first_or_default() {
    std::unique_ptr<sql::ResultSet> result(get_stmt()->executeQuery());
    if (result->next()) {
      return std::move(m_configuration.get_entity(result.get()));
    }
    return nullptr;
  }

  std::shared_ptr<std::vector<std::shared_ptr<T>>> all() {
    std::unique_ptr<sql::ResultSet> result(get_stmt()->executeQuery());
    auto entities = std::make_shared<std::vector<std::shared_ptr<T>>>();
    while (result->next()) {
      entities->push_back(std::move(m_configuration.get_entity(result.get())));
    }
    return entities;
  }

 private:
  configurations::repository_configuration<T> m_configuration;
};

}  // namespace repository