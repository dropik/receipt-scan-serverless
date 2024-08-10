//
// Created by Daniil Ryzhkov on 09/06/2024.
//

#pragma once

#include "repository/category_repository.hpp"

#include "../identity.hpp"
#include "../responses/category.hpp"
#include "../parameters/put_category.hpp"
#include "../responses/change.hpp"

namespace api {
namespace services {

class t_category_service {};

template<
    typename TRepository = repository::t_category_repository,
    typename TIdentity = const identity>
class category_service {
 public:
  category_service(TRepository repository, TIdentity identity)
      : m_repository(std::move(repository)),
        m_identity(std::move(identity)) {}

  std::vector<responses::category> get_categories() {
    auto categories = m_repository->get_all(m_identity->user_id);

    std::vector<responses::category> response;
    for (auto c : categories) {
      response.push_back(responses::category::from_repo(c));
    }

    return response;
  }

  void put_category(const parameters::put_category &params) {
    auto c = params.to_repo(m_identity->user_id);
    m_repository->store(c);
  }

  void delete_category(const guid_t &category_id) {
    m_repository->drop(category_id);
  }

  std::vector<responses::change<responses::category>> get_changes(const std::string &since) {
    auto categories = m_repository->get_changed(since);

    std::vector<responses::change<responses::category>> response;
    for (auto c : categories) {
      response.push_back(responses::change<responses::category>{
          .action = c.is_deleted
                    ? responses::change_action::del
                    : (c.version == 0
                       ? responses::change_action::create
                       : responses::change_action::update),
          .id = c.id,
          .body = c.is_deleted
                  ? lambda::nullable<responses::category>{}
                  : responses::category::from_repo(c),
      });
    }

    return response;
  }

 private:
  TRepository m_repository;
  TIdentity m_identity;
};

}
}
