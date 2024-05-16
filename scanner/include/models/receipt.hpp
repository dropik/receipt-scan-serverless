#pragma once

#include <string>

namespace scanner {
namespace models {

struct receipt {
  guid id;
  guid user_id;
  guid request_id;
  int doc_number;
  std::string date;
  long double total_amount;
  std::string currency;
  std::string store_name;
  std::string category;
};

}  // namespace models
}  // namespace scanner
