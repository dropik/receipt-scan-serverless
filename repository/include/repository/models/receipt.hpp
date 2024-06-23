#pragma once

#include <string>
#include <lambda/nullable.hpp>
#include "receipt_item.hpp"
#include "receipt_file.hpp"

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

  std::vector<receipt_item> items;
  lambda::nullable<receipt_file> file;

  static constexpr auto processing = "processing";
  static constexpr auto done = "done";
};

}  // namespace models
}  // namespace repository
