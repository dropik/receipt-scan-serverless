//
// Created by Daniil Ryzhkov on 08/06/2024.
//

#pragma once

#include <string>
#include <vector>

#include <lambda/json.hpp>

#include "../model_types.hpp"

namespace api {
namespace responses {

struct receipt_item_detail {
  guid_t id;
  std::string description;
  long double amount = 0;
  std::string category;

  JSON_BEGIN_SERIALIZER(receipt_item_detail)
      JSON_PROPERTY("id", id)
      JSON_PROPERTY("description", description)
      JSON_PROPERTY("amount", amount)
      JSON_PROPERTY("category", category)
  JSON_END_SERIALIZER()
};

struct receipt_detail {
  guid_t id;
  std::string date;
  long double total_amount = 0;
  std::string currency;
  std::string store_name;
  std::string category;
  std::string state;
  std::vector<receipt_item_detail> items;

  JSON_BEGIN_SERIALIZER(receipt_detail)
      JSON_PROPERTY("id", id)
      JSON_PROPERTY("date", date)
      JSON_PROPERTY("totalAmount", total_amount)
      JSON_PROPERTY("currency", currency)
      JSON_PROPERTY("storeName", store_name)
      JSON_PROPERTY("category", category)
      JSON_PROPERTY("state", state)
      JSON_PROPERTY("items", items)
  JSON_END_SERIALIZER()
};

}
}
