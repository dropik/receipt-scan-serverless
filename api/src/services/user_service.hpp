//
// Created by Daniil Ryzhkov on 07/06/2024.
//

#pragma once

#include <memory>

#include <repository/client.hpp>
#include "../models/identity.hpp"

namespace api {
namespace services {

class i_user_service {};

template<
    typename IRepository = repository::i_client,
    typename IIdentity = const models::identity>
class user_service {
  using user = repository::models::user;

 public:
  user_service(IRepository repository, IIdentity identity)
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

    m_repository->execute(
            "insert into categories (id, user_id, name) "
            "select uuid_v4(), ?, name from categories "
            "where user_id is null;")
        .with_param(user_id)
        .go();
  }

 private:
  IRepository m_repository;
  IIdentity m_identity;
};

}
}