#pragma once

#include <string>

namespace scanner {
namespace models {

struct receipt_item {
  guid id;
  guid receipt_id;
  std::string description;
  long double amount;
  std::string currency;
  std::string category;
  int sort_order;
};

}  // namespace models
}  // namespace scanner
