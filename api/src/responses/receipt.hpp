//
// Created by Daniil Ryzhkov on 08/06/2024.
//

#pragma once

#include <string>
#include <vector>

#include <lambda/json.hpp>

#include "../model_types.hpp"
#include "receipt_item.hpp"
#include "repository/models/receipt.hpp"

namespace api {
namespace responses {

struct receipt {
  guid_t id;
  std::string date;
  long double total_amount = 0;
  std::string currency;
  std::string store_name;
  std::vector<std::string> categories;
  std::string state;
  lambda::nullable<std::string> image_name;
  int version;
  std::vector<receipt_item> items;

  JSON_BEGIN_SERIALIZER(receipt)
      JSON_PROPERTY("id", id)
      JSON_PROPERTY("date", date)
      JSON_PROPERTY("totalAmount", total_amount)
      JSON_PROPERTY("currency", currency)
      JSON_PROPERTY("storeName", store_name)
      JSON_PROPERTY("categories", categories)
      JSON_PROPERTY("state", state)
      JSON_PROPERTY("imageName", image_name)
      JSON_PROPERTY("version", version)
      JSON_PROPERTY("items", items)
  JSON_END_SERIALIZER()

  static receipt from_repo(const repository::models::receipt &receipt);
};

}
}
