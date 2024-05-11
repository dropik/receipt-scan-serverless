#pragma once

#include <string>

namespace scanner {
  namespace models {
    class receipt_item {
    public:
      std::string id;
      std::string receipt_id;
      std::string description;
      long double amount;
      std::string category;
      int sort_order;
    };
  }
}
