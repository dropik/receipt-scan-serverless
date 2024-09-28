//
// Created by Daniil Ryzhkov on 07/06/2024.
//

#pragma once

#include <memory>

#include <repository/client.hpp>
#include "../identity.hpp"
#include "../api_errors.hpp"
#include "../responses/user.hpp"
#include "file_service.hpp"

namespace api::services {

struct t_user_service {};

template<
    typename TRepository = repository::t_client,
    typename TIdentity = const identity,
    typename TFileService = t_file_service>
class user_service {
  using user = repository::models::user;

 public:
  user_service(TRepository repository, TIdentity identity, TFileService file_service)
      : m_repository(std::move(repository)), m_identity(std::move(identity)), m_file_service(std::move(file_service)) {}

  void init_user() {
    auto user_id = m_identity->user_id;

    auto existing_user =
        m_repository->template select<user>("select * from users where id = ?")
            .with_param(user_id)
            .first_or_default();

    if (existing_user) return;

    user user{.id = user_id};
    m_repository->create(user);
  }

  responses::user get_user() {
    auto users = m_repository->template select<user>("select * from users where id = ?")
        .with_param(m_identity->user_id)
        .all();
    if (users->empty()) {
      throw rest::api_exception(not_found, "User not found");
    }
    return responses::user::from_repository(*users->at(0));
  }

  void delete_user() {
    delete_data_from_database();
    m_file_service->delete_receipt_images(m_identity->user_id);
  }

 private:
  TRepository m_repository;
  TIdentity m_identity;
  TFileService m_file_service;

  void delete_data_from_database() {
    auto user_id = m_identity->user_id;
    try {
      m_repository->execute("delete from receipts where user_id = ?").with_param(user_id).go();
      m_repository->execute("delete from categories where user_id = ?").with_param(user_id).go();
      m_repository->execute("delete from budgets where user_id = ?").with_param(user_id).go();
      m_repository->execute("delete from users where id = ?").with_param(user_id).go();
    } catch (const std::exception &e) {
      lambda::log.error("Failed to delete user data: %s", e.what());
      throw rest::api_exception(internal, "Failed to delete user data");
    }
  }
};

}
