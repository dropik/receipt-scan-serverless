//
// Created by Daniil Ryzhkov on 08/06/2024.
//

#pragma once

#include <string>
#include <vector>

#include <aws-lambda-cpp/common/json.hpp>

#include "model_types.hpp"

namespace api {
namespace models {

struct receipt_item_response {
  guid_t id;
  std::string description;
  long double amount = 0;
  std::string category;

  JSON_BEGIN_SERIALIZER(receipt_item_response)
      JSON_PROPERTY("id", id)
      JSON_PROPERTY("description", description)
      JSON_PROPERTY("amount", amount)
      JSON_PROPERTY("category", category)
  JSON_END_SERIALIZER()
};

struct receipt_response {
  guid_t id;
  std::string date;
  long double total_amount = 0;
  std::string currency;
  std::string store_name;
  std::string category;
  std::string file_url;
  std::vector<receipt_item_response> items;

  JSON_BEGIN_SERIALIZER(receipt_response)
      JSON_PROPERTY("id", id)
      JSON_PROPERTY("date", date)
      JSON_PROPERTY("total_amount", total_amount)
      JSON_PROPERTY("currency", currency)
      JSON_PROPERTY("store_name", store_name)
      JSON_PROPERTY("category", category)
      JSON_PROPERTY("file_url", file_url)
      JSON_PROPERTY("items", items)
  JSON_END_SERIALIZER()
};

}
}
