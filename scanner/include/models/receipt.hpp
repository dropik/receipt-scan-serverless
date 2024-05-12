#pragma once

#include <string>

namespace scanner {
namespace models {

class receipt {
 public:
  std::string id;
  std::string user_id;
  std::string date;
  long double total_amount;
  std::string store_name;
};

}  // namespace models
}  // namespace scanner
