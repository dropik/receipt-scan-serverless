//
// Created by Daniil Ryzhkov on 03/08/2024.
//

#pragma once

#include "repository/client.hpp"
#include "repository/models/user.hpp"
#include "../identity.hpp"
#include "../parameters/put_device.hpp"
#include "../api_errors.hpp"
#include "rest/api_exception.hpp"

namespace api {
namespace services {

struct t_device_service {};

template<
    typename TRepository = repository::t_client,
    typename TIdentity = const identity>
class device_service {
 public:
  using user_device = repository::models::user_device;

  device_service(TRepository repository, TIdentity identity)
      : m_repository(std::move(repository)), m_identity(std::move(identity)) {}

  void register_device(const parameters::put_device &params) {
    if (params.id.empty()) {
      throw rest::api_exception(invalid_argument, "Id is required");
    }

    auto devices = m_repository->template select<user_device>("select * from user_devices where id = ?")
        .with_param(params.id)
        .all();
    if (devices->size() == 0) {
      m_repository->template create<user_device>(user_device{
          .id = params.id,
          .user_id = m_identity->user_id
      });
    }
  }

 private:
  TRepository m_repository;
  TIdentity m_identity;
};

}
}
