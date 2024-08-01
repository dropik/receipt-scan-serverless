//
// Created by Daniil Ryzhkov on 01/08/2024.
//

#pragma once

#include <string>
#include <memory>
#include <mariadb/conncpp/Connection.hpp>
#include <mariadb/conncpp/PreparedStatement.hpp>

#include "repository/models/common.hpp"
#include "repository/models/entity_event.hpp"

namespace repository {
namespace configurations {
namespace common {

template<typename T>
class tracking_configuration {
 public:
  typedef const models::guid &(*id_selector_t)(const T &);

  tracking_configuration(const id_selector_t &id_selector,
                         const id_selector_t &user_id_selector)
      : m_id_selector(id_selector), m_user_id_selector(user_id_selector) {}

  tracking_configuration &with_entity_name(const std::string &entity_name) {
    m_entity_name = entity_name;
    return *this;
  }

  void track_creation(const T &entity, const std::shared_ptr<sql::Connection> &conn) {
    track(entity, conn, create_event);
  }

  void track_update(const T &entity, const std::shared_ptr<sql::Connection> &conn) {
    track(entity, conn, update_event);
  }

  void track_delete(const T &entity, const std::shared_ptr<sql::Connection> &conn) {
    track(entity, conn, delete_event);
  }

 private:
  id_selector_t m_id_selector;
  id_selector_t m_user_id_selector;
  std::string m_entity_name;
  std::unique_ptr<sql::PreparedStatement> m_insert_statement;

  const char *create_event = models::entity_event::create;
  const char *update_event = models::entity_event::update;
  const char *delete_event = models::entity_event::del;

  static constexpr auto insert_query =
      "insert into entity_events (id, device_id, entity_type, entity_id, event_type) "
      "select uuid_v4(), ud.id, ?, ?, ? "
      "from user_devices ud where ud.user_id = ?";

  void prepare_statement(const std::shared_ptr<sql::Connection> &conn) {
    if (!m_insert_statement) {
      std::unique_ptr<sql::PreparedStatement> stmt(conn->prepareStatement(insert_query));
      if (!stmt) {
        throw std::runtime_error("Unable to create prepared statement!");
      }
      m_insert_statement = std::move(stmt);
    }
  }

  void track(const T &entity, const std::shared_ptr<sql::Connection> &conn, const char *event_type) {
    auto id = m_id_selector(entity);
    auto user_id = m_user_id_selector(entity);

    prepare_statement(conn);

    m_insert_statement->setString(1, m_entity_name);
    m_insert_statement->setString(2, id);
    m_insert_statement->setString(3, event_type);
    m_insert_statement->setString(4, user_id);
    m_insert_statement->execute();
  }
};

}  // namespace common
}  // namespace configurations
}  // namespace repository
