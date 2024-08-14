//
// Created by Daniil Ryzhkov on 03/08/2024.
//

#pragma once

#include <lambda/json.hpp>
#include "repository/models/budget.hpp"

#include "../model_types.hpp"

namespace api::parameters {

struct put_budget {
  guid_t id;
  std::string month;
  long double amount;
  int version;

  JSON_BEGIN_SERIALIZER(put_budget)
      JSON_PROPERTY("id", id)
      JSON_PROPERTY("month", month)
      JSON_PROPERTY("amount", amount)
      JSON_PROPERTY("version", version)
  JSON_END_SERIALIZER()

  [[nodiscard]] repository::models::budget to_repo(const std::string &user_id) const;
};

}
