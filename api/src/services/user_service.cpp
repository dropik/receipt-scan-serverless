//
// Created by Daniil Ryzhkov on 07/06/2024.
//

#include "user_service.hpp"

using namespace repository::models;

namespace api {
namespace services {

user_service::user_service(std::shared_ptr<repository::client> repository,
                           const api::models::identity &identity) : m_repository(std::move(repository)),
                                                                    m_identity(identity) {}
void user_service::init_user() {
  auto user_id = m_identity.user_id;

  auto existing_user =
      m_repository->select<user>("select * from users where id = ?")
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

}
}
