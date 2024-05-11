#pragma once

#include <functional>
#include <string>

#include "base_property_configuration.hpp"

namespace scanner {
namespace repository {
namespace common {

template <typename T, typename TProperty>
class property_configuration : public base_property_configuration<T> {
 public:
  typedef const TProperty& (*property_selector_t)(const T&);

  property_configuration(const property_selector_t& property_selector)
      : m_property_selector(property_selector) {}

  void configure_statement(int p_number, const T& t,
                           std::shared_ptr<sql::PreparedStatement>& stmt) {
    stmt->setString(p_number, m_property_selector(t));
  }

 private:
  property_selector_t m_property_selector;
};

template <typename T>
class property_configuration<T, int> : public base_property_configuration<T> {
 public:
  typedef int (*property_selector_t)(const T&);

  property_configuration(const property_selector_t& property_selector)
      : m_property_selector(property_selector) {}

  void configure_statement(int p_number, const T& t,
                           std::shared_ptr<sql::PreparedStatement>& stmt) {
    stmt->setInt(p_number, m_property_selector(t));
  }

 private:
  property_selector_t m_property_selector;
};

template <typename T>
class property_configuration<T, long> : public base_property_configuration<T> {
 public:
  typedef long (*property_selector_t)(const T&);

  property_configuration(const property_selector_t& property_selector)
      : m_property_selector(property_selector) {}

  void configure_statement(int p_number, const T& t,
                           std::shared_ptr<sql::PreparedStatement>& stmt) {
    stmt->setInt64(p_number, m_property_selector(t));
  }

 private:
  property_selector_t m_property_selector;
};

template <typename T>
class property_configuration<T, double>
    : public base_property_configuration<T> {
 public:
  typedef double (*property_selector_t)(const T&);

  property_configuration(const property_selector_t& property_selector)
      : m_property_selector(property_selector) {}

  void configure_statement(int p_number, const T& t,
                           std::shared_ptr<sql::PreparedStatement>& stmt) {
    stmt->setDouble(p_number, m_property_selector(t));
  }

 private:
  property_selector_t m_property_selector;
};

template <typename T>
class property_configuration<T, long double>
    : public base_property_configuration<T> {
 public:
  typedef long double (*property_selector_t)(const T&);

  property_configuration(const property_selector_t& property_selector)
      : m_property_selector(property_selector) {}

  void configure_statement(int p_number, const T& t,
                           std::shared_ptr<sql::PreparedStatement>& stmt) {
    stmt->setDouble(p_number, m_property_selector(t));
  }

 private:
  property_selector_t m_property_selector;
};

template <typename T>
class property_configuration<T, bool> : public base_property_configuration<T> {
 public:
  typedef bool (*property_selector_t)(const T&);

  property_configuration(const property_selector_t& property_selector)
      : m_property_selector(property_selector) {}

  void configure_statement(int p_number, const T& t,
                           std::shared_ptr<sql::PreparedStatement>& stmt) {
    stmt->setBoolean(p_number, m_property_selector(t));
  }

 private:
  property_selector_t m_property_selector;
};

}  // namespace common
}  // namespace repository
}  // namespace scanner