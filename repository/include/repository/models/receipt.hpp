#pragma once

#include <string>
#include <lambda/nullable.hpp>
#include "receipt_item.hpp"

namespace repository {
namespace models {

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

  std::vector<receipt_item> items;

  static constexpr auto processing = "processing";
  static constexpr auto done = "done";
  static constexpr auto failed = "failed";
};

}  // namespace models
}  // namespace repository
