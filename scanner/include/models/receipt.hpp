#pragma once

#include <string>

namespace scanner {
namespace models {

struct receipt {
  std::string id;
  std::string user_id;
  std::string request_id;
  int doc_number;
  std::string date;
  long double total_amount;
  std::string store_name;
};

}  // namespace models
}  // namespace scanner
