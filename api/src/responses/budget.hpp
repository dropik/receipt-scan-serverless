//
// Created by Daniil Ryzhkov on 03/08/2024.
//

#pragma once

#include <lambda/json.hpp>
#include "repository/models/budget.hpp"

#include "../model_types.hpp"

namespace api {
namespace responses {

struct budget {
  guid_t id;
  std::string month;
  long double amount;
  int version;

  JSON_BEGIN_SERIALIZER(budget)
      JSON_PROPERTY("id", id)
      JSON_PROPERTY("month", month)
      JSON_PROPERTY("amount", amount)
      JSON_PROPERTY("version", version)
  JSON_END_SERIALIZER()

  static budget from_repo(const repository::models::budget &b);
};

}
}
