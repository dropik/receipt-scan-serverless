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
    auto res = m_repository->template select<models::category>("select * from categories where user_id = ?")
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

    if (existing_category) {
      m_repository->template update(category);
    } else {
      m_repository->template create(category);
    }
  }

 private:
  TRepository m_repository;

  using category = models::category;
};

}
