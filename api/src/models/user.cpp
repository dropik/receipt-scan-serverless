//
// Created by Daniil Ryzhkov on 03/08/2024.
//

#include "user.hpp"

namespace api {
namespace models {

user user::from_repository(const repository::models::user &u) {
  return {
      .id = u.id,
  };
}

}
}
