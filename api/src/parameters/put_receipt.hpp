//
// Created by Daniil Ryzhkov on 10/06/2024.
//

#pragma once

#include <lambda/json.hpp>
#include "../model_types.hpp"
#include "repository/models/receipt.hpp"
#include "put_receipt_item.hpp"

namespace api {
namespace parameters {

struct put_receipt {
  guid_t id;
  std::string date;
  long double total_amount = 0;
  std::string currency;
  std::string store_name;
  lambda::nullable<std::string> category;
  std::string state;
  lambda::nullable<std::string> image_name;
  int version;
  std::vector<put_receipt_item> items;

  JSON_BEGIN_SERIALIZER(put_receipt)
      JSON_PROPERTY("id", id)
      JSON_PROPERTY("date", date)
      JSON_PROPERTY("totalAmount", total_amount)
      JSON_PROPERTY("currency", currency)
      JSON_PROPERTY("storeName", store_name)
      JSON_PROPERTY("category", category)
      JSON_PROPERTY("state", state)
      JSON_PROPERTY("imageName", image_name)
      JSON_PROPERTY("version", version)
      JSON_PROPERTY("items", items)
  JSON_END_SERIALIZER()

  repository::models::receipt to_repo(const std::string &user_id) const;
};

}
}
