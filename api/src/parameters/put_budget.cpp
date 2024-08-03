//
// Created by Daniil Ryzhkov on 03/08/2024.
//

#include "put_budget.hpp"

namespace api {
namespace parameters {

repository::models::budget put_budget::to_repo(const std::string &user_id) const {
  return repository::models::budget{
      id,
      user_id,
      month,
      amount,
      version
  };
}

}
}
