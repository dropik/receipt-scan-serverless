//
// Created by Daniil Ryzhkov on 09/06/2024.
//

#pragma once

#include <lambda/json.hpp>
#include "repository/models/category.hpp"

#include "../model_types.hpp"

namespace api::responses {

struct category {
  guid_t id;
  std::string name;
  int color;
  lambda::nullable<int> icon;
  int version = 0;

  JSON_BEGIN_SERIALIZER(category)
      JSON_PROPERTY("id", id)
      JSON_PROPERTY("name", name)
      JSON_PROPERTY("color", color)
      JSON_PROPERTY("icon", icon)
      JSON_PROPERTY("version", version)
  JSON_END_SERIALIZER()

  static category from_repo(const repository::models::category &category);
};

}
