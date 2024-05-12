#pragma once

#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <conncpp/Connection.hpp>
#include <conncpp/PreparedStatement.hpp>

#include "id_configuration.hpp"
#include "property_configuration.hpp"
#include "table_configuration.hpp"

#define HAS_TABLE(table_name) has_table(table_name)
#define HAS_ID(id_field) \
  has_id([](const entity_t& entity) -> const std::string& { return entity.id_field; })
#define WITH_COLUMN(column_name) .with_column_name(column_name)
#define HAS_STRING(field) \
  has_property<std::string>([](const entity_t& entity) -> const std::string& { return entity.field; })
#define HAS_INT(field) has_property<int>([](const entity_t& entity) -> int { return entity.field; })
#define HAS_DOUBLE(field) \
  has_property<double>([](const entity_t& entity) -> double { return entity.field; })
#define HAS_BOOL(field) \
  has_property<bool>([](const entity_t& entity) -> bool { return entity.field; })
#define HAS_DECIMAL(field) \
  has_property<long double>([](const entity_t& entity) -> long double { return entity.field; })

namespace scanner {
namespace repository {
namespace common {

template <typename T>
class base_repository_configuration {
 public:
  std::shared_ptr<sql::PreparedStatement> get_insert_statement(
      const T& entity, std::shared_ptr<sql::Connection>& connection) {
    if (!m_insert_statement) {
      if (!m_table) {
        throw std::runtime_error("Table is not configured!");
      }
      if (!m_id) {
        throw std::runtime_error("Id is not configured!");
      }

      std::string query =
          "insert into " + m_table->get_name() + " (" + m_id->get_column_name();
      for (const auto& property : m_properties) {
        if (!property) continue;
        query += ", " + property->get_column_name();
      }
      query += ") values (?";
      for (size_t i = 0; i < m_properties.size(); i++) {
        query += ", ?";
      }
      query += ")";
      std::cout << "Insert query: " << query << std::endl;
      std::shared_ptr<sql::PreparedStatement> stmt(
          connection->prepareStatement(query));
      if (!stmt) {
        throw std::runtime_error("Unable to create prepared statement!");
      }
      m_insert_statement = std::move(stmt);
    }

    m_id->configure_statement(1, entity, m_insert_statement);
    for (size_t i = 0; i < m_properties.size(); i++) {
      if (!m_properties[i]) continue;
      m_properties[i]->configure_statement(i + 2, entity, m_insert_statement);
    }

    return m_insert_statement;
  }

 protected:
  typedef T entity_t;
  
  table_configuration& has_table(const std::string& table_name) {
    m_table = std::make_shared<table_configuration>(table_name);
    return *m_table;
  }

  id_configuration<T>& has_id(
      const typename id_configuration<T>::id_selector_t& id_selector) {
    m_id = std::make_shared<id_configuration<T>>(id_selector);
    return *m_id;
  }

  template <typename TProperty>
  base_property_configuration<T>& has_property(
      const typename property_configuration<T, TProperty>::property_selector_t&
          property_selector) {
    std::shared_ptr<property_configuration<T, TProperty>> property =
        std::make_shared<property_configuration<T, TProperty>>(
            property_selector);
    m_properties.push_back(std::move(property));
    return *m_properties.back();
  }

 private:
  std::shared_ptr<sql::PreparedStatement> m_insert_statement;
  std::shared_ptr<table_configuration> m_table;
  std::shared_ptr<id_configuration<T>> m_id;
  std::vector<std::shared_ptr<base_property_configuration<T>>> m_properties;
};

}  // namespace common
}  // namespace repository
}  // namespace scanner