//
// Created by Daniil Ryzhkov on 03/08/2024.
//

#pragma once

#include "repository/client.hpp"
#include "../identity.hpp"
#include "../responses/budget.hpp"
#include "../parameters/put_budget.hpp"
#include "../responses/change.hpp"
namespace api {
namespace services {

struct t_budget_service {};

template<
    typename TRepository = repository::t_client,
    typename TIdentity = identity>
class budget_service {
 public:
  budget_service(TRepository repository, TIdentity identity)
      : m_repository(std::move(repository)), m_identity(std::move(identity)) {}

  std::vector<responses::budget> get_budgets() {
    auto budgets = m_repository->template select<repository::models::budget>("select * from budgets where user_id = ?")
        .with_param(m_identity->user_id)
        .all();

    std::vector<responses::budget> result;
    result.reserve(budgets->size());
    for (const auto &b : (*budgets)) {
      result.push_back(responses::budget::from_repo(*b));
    }

    return result;
  }

  void store_budget(const parameters::put_budget &params) {
    auto existing = m_repository->template select<repository::models::budget>("select * from budgets where id = ?")
        .with_param(params.id)
        .first_or_default();
    auto new_budget = params.to_repo(m_identity->user_id);
    if (existing) {
      m_repository->template update<repository::models::budget>(new_budget);
    } else {
      m_repository->template create<repository::models::budget>(new_budget);
    }
  }

  std::vector<responses::change<responses::budget>> get_budget_changes(const std::string &since) {
    auto budgets = m_repository->template select<repository::models::budget>(
            "select * from budgets where modified_timestamp > ?")
        .with_param(since)
        .all();

    std::vector<responses::change<responses::budget>> result;
    result.reserve(budgets->size());
    for (const auto &b : (*budgets)) {
      result.push_back(responses::change<responses::budget>{
          .action = b->version == 0
                    ? responses::change_action::create
                    : responses::change_action::update,
          .id = b->id,
          .body = responses::budget::from_repo(*b),
      });
    }

    return result;
  }

 private:
  TRepository m_repository;
  TIdentity m_identity;
};

}
}
