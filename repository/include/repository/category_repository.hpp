//
// Created by Daniil Ryzhkov on 23/06/2024.
//

#pragma once

#include "client.hpp"

namespace repository {

struct t_category_repository {};

template<typename TRepository = t_client>
class category_repository {
 public:
  explicit category_repository(TRepository repository) : m_repository(std::move(repository)) {}

  std::vector<models::category> get_all(const std::string &user_id) const {
    auto res = m_repository->template select<models::category>(
            "select * from categories where user_id = ? and is_deleted = 0 order by name")
        .with_param(user_id)
        .all();
    std::vector<models::category> results;
    for (auto &category : *res) {
      results.push_back(*category);
    }
    return results;
  }

  void store(const models::category &category) {
    auto existing_category = m_repository->template select<models::category>(
            "select * from categories where id = ?")
        .with_param(category.id)
        .first_or_default();

    if (!existing_category) {
      m_repository->template create(category);
    } else {
      if (existing_category->is_deleted) {
        throw concurrency_exception();
      }
      m_repository->template update(category);
    }
  }

  void drop(const models::guid &category_id) {
    auto existing_category = m_repository->template get<models::category>(category_id);

    if (!existing_category) {
      throw entity_not_found_exception();
    }

    existing_category->version++;
    existing_category->is_deleted = true;
    m_repository->template update(*existing_category);
  }

  std::vector<models::category> get_changed(const models::guid &user_id, const std::string &since) {
    auto categories = m_repository->template select<models::category>(
            "select * from categories where user_id = ? and modified_timestamp > ?")
        .with_param(user_id)
        .with_param(since)
        .all();

    std::vector<models::category> result;
    result.reserve(categories->size());
    for (const auto &c : *categories) {
      result.push_back(*c);
    }

    return result;
  }

 private:
  TRepository m_repository;

  using category = models::category;
};

}
