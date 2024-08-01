//
// Created by Daniil Ryzhkov on 07/06/2024.
//

#pragma once

#include <memory>

#include <repository/client.hpp>
#include "../models/identity.hpp"

namespace api {
namespace services {

struct t_user_service {};

template<
    typename TRepository = repository::t_client,
    typename TIdentity = const models::identity>
class user_service {
  using user = repository::models::user;

 public:
  user_service(TRepository repository, TIdentity identity)
      : m_repository(std::move(repository)), m_identity(std::move(identity)) {}

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

 private:
  TRepository m_repository;
  TIdentity m_identity;
};

}
}
