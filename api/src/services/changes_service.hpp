//
// Created by Daniil Ryzhkov on 07/08/2024.
//

#pragma once

#include "repository/client.hpp"
#include "../identity.hpp"
#include "../responses/changes.hpp"

namespace api {
namespace services {

struct t_changes_service {};

template<
    typename TRepository = repository::t_client,
    typename TIdentity = identity>
class changes_service {
 public:
  changes_service(TRepository repository, TIdentity identity)
      : m_repository(std::move(repository)),
        m_identity(std::move(identity)) {}



 private:
  TRepository m_repository;
  TIdentity m_identity;
};

}
}
