//
// Created by Daniil Ryzhkov on 03/08/2024.
//

#pragma once

#include <lambda/json.hpp>
#include "repository/models/category.hpp"

#include "../model_types.hpp"

namespace api::parameters {

struct put_category {
  guid_t id;
  std::string name;
  int color;
  lambda::nullable<int> icon;
  int version = 0;

  JSON_BEGIN_SERIALIZER(put_category)
      JSON_PROPERTY("id", id)
      JSON_PROPERTY("name", name)
      JSON_PROPERTY("color", color)
      JSON_PROPERTY("icon", icon)
      JSON_PROPERTY("version", version)
  JSON_END_SERIALIZER()

  [[nodiscard]] repository::models::category to_repo(const std::string &user_id) const;
};

}
