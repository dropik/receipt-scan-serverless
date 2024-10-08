#pragma once

#include <string>
#include <lambda/nullable.hpp>
#include "receipt_item.hpp"

namespace repository::models {

struct receipt {
  guid id;
  guid user_id;
  std::string date;
  long double total_amount;
  std::string currency;
  std::string store_name;
  std::string category;
  std::string state;
  std::string image_name;
  int version = 0;
  bool is_deleted = false;

  std::vector<receipt_item> items;

  inline static constexpr auto processing = "processing";
  inline static constexpr auto done = "done";
  inline static constexpr auto failed = "failed";
};

} // namespace repository::models
