#pragma once

#include <string>

#include "base_property_configuration.hpp"

namespace repository {
namespace configurations {
namespace common {

template <typename T, typename TProperty>
class property_configuration : public base_property_configuration<T> {
 public:
  typedef const TProperty& (*property_selector_t)(const T&);
  typedef void (*property_setter_t)(T&, const TProperty&);

  property_configuration(const property_selector_t& property_selector,
                         const property_setter_t& property_setter)
      : m_property_selector(property_selector),
        m_property_setter(property_setter) {}

  void configure_statement(int p_number, const T& t,
                           std::shared_ptr<sql::PreparedStatement>& stmt) {
    stmt->setString(p_number, m_property_selector(t));
  }

  void set_entity_property(T& t, sql::ResultSet* res) {
    m_property_setter(t, res->getString(this->get_column_name()).c_str());
  }

 private:
  property_selector_t m_property_selector;
  property_setter_t m_property_setter;
};

template <typename T>
class property_configuration<T, int> : public base_property_configuration<T> {
 public:
  typedef int (*property_selector_t)(const T&);
  typedef void (*property_setter_t)(T&, int);

  property_configuration(const property_selector_t& property_selector,
                         const property_setter_t& property_setter)
      : m_property_selector(property_selector),
        m_property_setter(property_setter) {}

  void configure_statement(int p_number, const T& t,
                           std::shared_ptr<sql::PreparedStatement>& stmt) {
    stmt->setInt(p_number, m_property_selector(t));
  }

  void set_entity_property(T& t, sql::ResultSet* res) {
    m_property_setter(t, res->getInt(this->get_column_name()));
  }

 private:
  property_selector_t m_property_selector;
  property_setter_t m_property_setter;
};

template <typename T>
class property_configuration<T, long> : public base_property_configuration<T> {
 public:
  typedef long (*property_selector_t)(const T&);
  typedef void (*property_setter_t)(T&, long);

  property_configuration(const property_selector_t& property_selector,
                         const property_setter_t& property_setter)
      : m_property_selector(property_selector),
        m_property_setter(property_setter) {}

  void configure_statement(int p_number, const T& t,
                           std::shared_ptr<sql::PreparedStatement>& stmt) {
    stmt->setInt64(p_number, m_property_selector(t));
  }

  void set_entity_property(T& t, sql::ResultSet* res) {
    m_property_setter(t, res->getInt64(this->get_column_name()));
  }

 private:
  property_selector_t m_property_selector;
  property_setter_t m_property_setter;
};

template <typename T>
class property_configuration<T, double>
    : public base_property_configuration<T> {
 public:
  typedef double (*property_selector_t)(const T&);
  typedef void (*property_setter_t)(T&, double);

  property_configuration(const property_selector_t& property_selector,
                         const property_setter_t& property_setter)
      : m_property_selector(property_selector),
        m_property_setter(property_setter) {}

  void configure_statement(int p_number, const T& t,
                           std::shared_ptr<sql::PreparedStatement>& stmt) {
    stmt->setDouble(p_number, m_property_selector(t));
  }

  void set_entity_property(T& t, sql::ResultSet* res) {
    m_property_setter(t, res->getDouble(this->get_column_name()));
  }

 private:
  property_selector_t m_property_selector;
  property_setter_t m_property_setter;
};

template <typename T>
class property_configuration<T, long double>
    : public base_property_configuration<T> {
 public:
  typedef long double (*property_selector_t)(const T&);
  typedef void (*property_setter_t)(T&, long double);

  property_configuration(const property_selector_t& property_selector,
                         const property_setter_t& property_setter)
      : m_property_selector(property_selector),
        m_property_setter(property_setter) {}

  void configure_statement(int p_number, const T& t,
                           std::shared_ptr<sql::PreparedStatement>& stmt) {
    stmt->setDouble(p_number, m_property_selector(t));
  }

  void set_entity_property(T& t, sql::ResultSet* res) {
    m_property_setter(t, res->getDouble(this->get_column_name()));
  }

 private:
  property_selector_t m_property_selector;
  property_setter_t m_property_setter;
};

template <typename T>
class property_configuration<T, bool> : public base_property_configuration<T> {
 public:
  typedef bool (*property_selector_t)(const T&);
  typedef void (*property_setter_t)(T&, bool);

  property_configuration(const property_selector_t& property_selector,
                         const property_setter_t& property_setter)
      : m_property_selector(property_selector),
        m_property_setter(property_setter) {}

  void configure_statement(int p_number, const T& t,
                           std::shared_ptr<sql::PreparedStatement>& stmt) {
    stmt->setBoolean(p_number, m_property_selector(t));
  }

  void set_entity_property(T& t, sql::ResultSet* res) {
    m_property_setter(t, res->getBoolean(this->get_column_name()));
  }

 private:
  property_selector_t m_property_selector;
  property_setter_t m_property_setter;
};

}  // namespace common
}  // namespace configurations
}  // namespace repository