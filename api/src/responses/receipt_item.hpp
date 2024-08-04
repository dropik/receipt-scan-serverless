//
// Created by Daniil Ryzhkov on 04/08/2024.
//

#pragma once

#include <lambda/json.hpp>
#include "../model_types.hpp"
#include "repository/models/receipt_item.hpp"

namespace api {
namespace responses {

struct receipt_item {
  guid_t id;
  std::string description;
  long double amount = 0;
  std::string category;

  JSON_BEGIN_SERIALIZER(receipt_item)
      JSON_PROPERTY("id", id)
      JSON_PROPERTY("description", description)
      JSON_PROPERTY("amount", amount)
      JSON_PROPERTY("category", category)
  JSON_END_SERIALIZER()

  static receipt_item from_repo(const repository::models::receipt_item &item);
};

}
}
