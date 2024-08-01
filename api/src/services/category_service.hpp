//
// Created by Daniil Ryzhkov on 09/06/2024.
//

#pragma once

#include <repository/client.hpp>
#include "../models/identity.hpp"
#include "../models/category.hpp"

namespace api {
namespace services {

class t_category_service {};

template<
    typename TRepository = repository::t_client,
    typename TIdentity = const models::identity>
class category_service {
 public:
  using category = models::category;

  category_service(TRepository repository, TIdentity identity)
      : m_repository(std::move(repository)),
        m_identity(std::move(identity)) {}

  std::vector<models::category> get_categories() {
    auto categories = m_repository->template select<repository::models::category>(
            "select * from categories where user_id = ? "
            "order by name asc")
        .with_param(m_identity->user_id)
        .all();

    std::vector<category> response;
    for (const auto &c : *categories) {
      category item;
      item.id = c->id;
      item.name = c->name;
      response.push_back(item);
    }

    return response;
  }

  void put_category(const models::category &category) {
    repository::models::category c;
    c.id = category.id;
    c.user_id = m_identity->user_id;
    c.name = category.name;

    auto existing = try_get_category(c.id);

    if (existing) {
      if (existing->user_id != m_identity->user_id) {
        throw rest::api_exception(forbidden, "Access denied");
      }
      m_repository->update(c);
    } else {
      m_repository->create(c);
    }
  }

  void delete_category(const models::guid_t &category_id) {
    auto existing = try_get_category(category_id);

    if (!existing) {
      throw rest::api_exception(not_found, "Category not found");
    }
    if (existing->user_id != m_identity->user_id) {
      throw rest::api_exception(forbidden, "Access denied");
    }

    m_repository->drop(*existing);
  }

 private:
  TRepository m_repository;
  TIdentity m_identity;

  std::shared_ptr<repository::models::category> try_get_category(const models::guid_t &category_id) {
    return m_repository->template select<repository::models::category>(
            "select * from categories where id = ?")
        .with_param(category_id)
        .first_or_default();
  }
};

}
}
