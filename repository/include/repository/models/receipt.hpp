#pragma once

#include <string>

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
};

}  // namespace models
}  // namespace repository
