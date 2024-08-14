//
// Created by Daniil Ryzhkov on 03/08/2024.
//

#include "put_category.hpp"

namespace api {
namespace parameters {

repository::models::category parameters::put_category::to_repo(const std::string &user_id) const {
  return repository::models::category{
      .id = id,
      .user_id = user_id,
      .name = name,
      .color = color,
      .version = version
  };
}

}
}
