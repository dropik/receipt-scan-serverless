//
// Created by Daniil Ryzhkov on 31/07/2024.
//

#pragma once

#include "common.hpp"

namespace repository {
namespace models {

struct budget {
  guid id;
  guid user_id;
  std::string month;
  long double amount = 0;
  int version = 0;
};

}
}
