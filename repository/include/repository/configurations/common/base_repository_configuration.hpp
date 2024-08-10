#pragma once

#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <mariadb/conncpp/Connection.hpp>
#include <mariadb/conncpp/PreparedStatement.hpp>

#include <repository/models/common.hpp>

#include "id_configuration.hpp"
#include "property_configuration.hpp"
#include "table_configuration.hpp"
#include "version_configuration.hpp"

#define HAS_TABLE(table_name) has_table(table_name)

#define HAS_ID(id_field) \
  has_id(                                                \
      [](const entity_t& entity) -> const models::guid& { \
        return entity.id_field;                          \
      },                                                 \
      [](entity_t& entity, const models::guid& id) {      \
        entity.id_field = id;                            \
      })

#define HAS_STRING(field) \
  has_property<std::string>(                             \
      [](const entity_t& entity) -> const std::string& { \
        return entity.field;                             \
      },                                                 \
      [](entity_t& entity, const std::string& s) { entity.field = s; })

#define HAS_INT(field) \
  has_property<int>(   \
      [](const entity_t& entity) -> int { return entity.field; }, \
      [](entity_t& entity, int i) { entity.field = i; })

#define HAS_DOUBLE(field) \
  has_property<double>(   \
      [](const entity_t& entity) -> double { return entity.field; }, \
      [](entity_t& entity, double d) { entity.field = d; })

#define HAS_BOOL(field) \
  has_property<bool>(   \
      [](const entity_t& entity) -> bool { return entity.field; }, \
      [](entity_t& entity, bool b) { entity.field = b; })

#define HAS_DECIMAL(field) \
  has_property<long double>( \
      [](const entity_t& entity) -> long double { return entity.field; }, \
      [](entity_t& entity, long double d) { entity.field = d; })

#define HAS_VERSION() has_version([](const entity_t& entity) -> const int & { return entity.version; }, [](entity_t& entity, const int &v) { entity.version = v; }).with_column_name("version")

#define WITH_COLUMN(column_name) .with_column_name(column_name)

namespace repository {
namespace configurations {
namespace common {

template<typename T>
class base_repository_configuration {
 public:
  const std::shared_ptr<sql::PreparedStatement> &get_insert_statement(
      const T &entity, const std::shared_ptr<sql::Connection> &connection) {
    if (!m_insert_statement) {
      if (!m_table) {
        throw std::runtime_error("Table is not configured!");
      }
      if (!m_id) {
        throw std::runtime_error("Id is not configured!");
      }

      std::string query =
          "insert into " + m_table->get_name() + " (" + m_id->get_column_name();
      for (const auto &property : m_properties) {
        if (!property) continue;
        query += ", " + property->get_column_name();
      }
      if (m_version) {
        query += ", " + m_version->get_column_name();
      }
      query += ") values (?";
      for (size_t i = 0; i < m_properties.size(); i++) {
        query += ", ?";
      }
      if (m_version) {
        query += ", ?";
      }
      query += ")";
      std::shared_ptr<sql::PreparedStatement> stmt(
          connection->prepareStatement(query));
      if (!stmt) {
        throw std::runtime_error("Unable to create prepared statement!");
      }
      m_insert_statement = std::move(stmt);
    }

    m_id->configure_statement(1, entity, m_insert_statement);
    int property_index = 2;
    for (size_t i = 0; i < m_properties.size(); i++) {
      if (!m_properties[i]) continue;
      m_properties[i]->configure_statement(property_index, entity,
                                           m_insert_statement);
      property_index++;
    }
    if (m_version) {
      m_version->configure_statement(property_index, m_version->get_version(entity), m_insert_statement);
    }

    return m_insert_statement;
  }

  const std::shared_ptr<sql::PreparedStatement> &get_select_statement(
      const std::string &id,
      const std::shared_ptr<sql::Connection> &connection) {
    if (!m_select_statement) {
      if (!m_table) {
        throw std::runtime_error("Table is not configured!");
      }
      if (!m_id) {
        throw std::runtime_error("Id is not configured!");
      }

      std::string query = "select " + m_id->get_column_name();
      for (const auto &property : m_properties) {
        if (!property) continue;
        query += ", " + property->get_column_name();
      }
      if (m_version) {
        query += ", " + m_version->get_column_name();
      }
      query += " from " + m_table->get_name() + " where " +
          m_id->get_column_name() + " = ?";
      std::shared_ptr<sql::PreparedStatement> stmt(
          connection->prepareStatement(query));
      if (!stmt) {
        throw std::runtime_error("Unable to create prepared statement!");
      }
      m_select_statement = std::move(stmt);
    }

    m_select_statement->setString(1, id);

    return m_select_statement;
  }

  std::shared_ptr<T> get_entity(sql::ResultSet *result) {
    auto entity = std::make_shared<T>();
    m_id->set_id(*entity, result);
    for (size_t i = 0; i < m_properties.size(); i++) {
      if (!m_properties[i]) continue;
      m_properties[i]->set_entity_property(*entity, result);
    }
    if (m_version) {
      m_version->set_version(*entity, result);
    }
    return entity;
  }

  const std::shared_ptr<sql::PreparedStatement> &get_update_statement(
      const T &entity, const std::shared_ptr<sql::Connection> &connection) {
    if (!m_update_statement) {
      if (!m_table) {
        throw std::runtime_error("Table is not configured!");
      }
      if (!m_id) {
        throw std::runtime_error("Id is not configured!");
      }

      std::string query = "update " + m_table->get_name() + " set ";
      for (size_t i = 0; i < m_properties.size(); i++) {
        if (!m_properties[i]) continue;
        query += m_properties[i]->get_column_name() + " = ?";
        if (i < m_properties.size() - 1) {
          query += ", ";
        }
      }
      if (m_version) {
        query += ", " + m_version->get_column_name() + " = ?";
      }
      query += " where " + m_id->get_column_name() + " = ?";
      if (m_version) {
        query += " and " + m_version->get_column_name() + " < ?";
      }
      std::shared_ptr<sql::PreparedStatement> stmt(
          connection->prepareStatement(query));
      if (!stmt) {
        throw std::runtime_error("Unable to create prepared statement!");
      }
      m_update_statement = std::move(stmt);
    }

    int property_index = 1;
    int new_version = 0;
    for (size_t i = 0; i < m_properties.size(); i++) {
      if (!m_properties[i]) continue;
      m_properties[i]->configure_statement(property_index, entity, m_update_statement);
      property_index++;
    }
    if (m_version) {
      new_version = m_version->get_version(entity) + 1;
      m_version->configure_statement(property_index, new_version, m_update_statement);
      property_index++;
    }
    m_id->configure_statement(property_index, entity, m_update_statement);
    property_index++;
    if (m_version) {
      m_update_statement->setInt(property_index, new_version);
    }

    return m_update_statement;
  }

  const std::shared_ptr<sql::PreparedStatement> &get_delete_statement(
      const T &entity, const std::shared_ptr<sql::Connection> &connection) {
    if (!m_delete_statement) {
      if (!m_table) {
        throw std::runtime_error("Table is not configured!");
      }
      if (!m_id) {
        throw std::runtime_error("Id is not configured!");
      }

      std::string query = "delete from " + m_table->get_name() + " where " +
          m_id->get_column_name() + " = ?";
      if (m_version) {
        query += " and " + m_version->get_column_name() + " = ?";
      }
      std::shared_ptr<sql::PreparedStatement> stmt(
          connection->prepareStatement(query));
      if (!stmt) {
        throw std::runtime_error("Unable to create prepared statement!");
      }
      m_delete_statement = std::move(stmt);
    }

    m_id->configure_statement(1, entity, m_delete_statement);
    if (m_version) {
      m_version->configure_statement(2, m_version->get_version(entity), m_delete_statement);
    }

    return m_delete_statement;
  }

  const std::string &get_table_name() const {
    if (!m_table) {
      throw std::runtime_error("Table is not configured!");
    }
    return m_table->get_name();
  }

 protected:
  typedef T entity_t;

  table_configuration &has_table(const std::string &table_name) {
    m_table = std::make_shared<table_configuration>(table_name);
    return *m_table;
  }

  id_configuration<T> &has_id(
      const typename id_configuration<T>::id_selector_t &id_selector,
      const typename id_configuration<T>::id_setter_t &id_setter) {
    m_id = std::make_shared<id_configuration<T>>
        (id_selector, id_setter);
    return *m_id;
  }

  template<typename TProperty>
  base_property_configuration<T> &has_property(
      const typename property_configuration<T, TProperty>::property_selector_t &
      property_selector,
      const typename property_configuration<T, TProperty>::property_setter_t &
      property_setter) {
    std::shared_ptr<property_configuration<T, TProperty>>
        property =
        std::make_shared<property_configuration<T, TProperty>>
            (
                property_selector, property_setter);
    m_properties.push_back(std::move(property));
    return *m_properties.back();
  }

  version_configuration<T> &has_version(
      const typename version_configuration<T>::version_selector_t &version_selector,
      const typename version_configuration<T>::version_setter_t &version_setter
  ) {
    m_version = std::make_shared<version_configuration<T>>(version_selector, version_setter);
    return *m_version;
  }

 private:
  std::shared_ptr<sql::PreparedStatement> m_insert_statement;
  std::shared_ptr<sql::PreparedStatement> m_select_statement;
  std::shared_ptr<sql::PreparedStatement> m_update_statement;
  std::shared_ptr<sql::PreparedStatement> m_delete_statement;
  std::shared_ptr<sql::PreparedStatement> m_put_statement;
  std::shared_ptr<table_configuration> m_table;
  std::shared_ptr<id_configuration<T>> m_id;
  std::vector<std::shared_ptr<base_property_configuration<T>>> m_properties;
  std::shared_ptr<version_configuration<T>> m_version;
};

}  // namespace common
}  // namespace configurations
}  // namespace repository
