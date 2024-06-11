//
// Created by Daniil Ryzhkov on 09/06/2024.
//

#pragma once

#include <lambda/json.hpp>
#include "model_types.hpp"

namespace api {
namespace models {

struct receipt_list_item {
  guid_t id;
  std::string date;
  long double total_amount = 0;
  std::string currency;
  std::string store_name;
  std::string category;
  std::string state;

  JSON_BEGIN_SERIALIZER(receipt_list_item)
      JSON_PROPERTY("id", id)
      JSON_PROPERTY("date", date)
      JSON_PROPERTY("totalAmount", total_amount)
      JSON_PROPERTY("currency", currency)
      JSON_PROPERTY("storeName", store_name)
      JSON_PROPERTY("category", category)
      JSON_PROPERTY("state", state)
  JSON_END_SERIALIZER()
};

}
}
