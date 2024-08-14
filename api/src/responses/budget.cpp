//
// Created by Daniil Ryzhkov on 03/08/2024.
//

#include "budget.hpp"

namespace api {
namespace responses {

budget budget::from_repo(const repository::models::budget &b) {
  return budget{
      b.id,
      b.month,
      b.amount,
      b.version
  };
}

}
}
