//
// Created by Daniil Ryzhkov on 03/08/2024.
//

#pragma once

#include <lambda/json.hpp>
#include "model_types.hpp"
#include "repository/models/user.hpp"

namespace api {
namespace models {

struct user {
  guid_t id;

  JSON_BEGIN_SERIALIZER(user)
      JSON_PROPERTY("id", id)
  JSON_END_SERIALIZER()

  static user from_repository(const repository::models::user &u);
};

}
}
