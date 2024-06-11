//
// Created by Daniil Ryzhkov on 07/06/2024.
//

#pragma once

#include <memory>

#include <repository/client.hpp>
#include "../models/identity.hpp"

namespace api {
namespace services {

class user_service {
 public:
  user_service(std::shared_ptr<repository::client> repository, const models::identity &identity);

  void init_user();

 private:
  std::shared_ptr<repository::client> m_repository;
  const models::identity &m_identity;
};

}
}