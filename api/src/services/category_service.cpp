//
// Created by Daniil Ryzhkov on 09/06/2024.
//

#include "category_service.hpp"
#include "../api_errors.hpp"
#include "rest/api_exception.hpp"

using namespace api::models;

namespace api {
namespace services {

category_service::category_service(std::shared_ptr<repository::client> repository, const models::identity &identity)
    : m_repository(std::move(repository)), m_identity(identity) {}

std::vector<models::category> category_service::get_categories() {
  auto categories = m_repository->select<repository::models::category>(
          "select * from categories where user_id = ? "
          "order by name asc")
      .with_param(m_identity.user_id)
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

void category_service::put_category(const models::category &category) {
  repository::models::category c;
  c.id = category.id;
  c.user_id = m_identity.user_id;
  c.name = category.name;

  auto existing = try_get_category(c.id);

  if (existing) {
    if (existing->user_id != m_identity.user_id) {
      throw rest::api_exception(forbidden, "Access denied");
    }
    m_repository->update(c);
  } else {
    m_repository->create(c);
  }
}

void category_service::delete_category(const models::guid_t &category_id) {
  auto existing = try_get_category(category_id);

  if (!existing) {
    throw rest::api_exception(not_found, "Category not found");
  }
  if (existing->user_id != m_identity.user_id) {
    throw rest::api_exception(forbidden, "Access denied");
  }

  m_repository->drop(existing);
}

std::shared_ptr<repository::models::category> category_service::try_get_category(const guid_t &category_id) {
  return m_repository->select<repository::models::category>(
          "select * from categories where id = ?")
      .with_param(category_id)
      .first_or_default();
}

}
}
