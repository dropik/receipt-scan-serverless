#pragma once

#include <string>

namespace repository {
namespace models {

namespace receipt_state {
constexpr auto processing = "processing";
constexpr auto done = "done";
}

struct receipt {
  guid id;
  guid user_id;
  std::string date;
  long double total_amount;
  std::string currency;
  std::string store_name;
  std::string category;
  std::string state;
};

}  // namespace models
}  // namespace repository
