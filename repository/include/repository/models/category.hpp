#pragma once

#include <string>

#include "common.hpp"

namespace repository::models {

struct category {
  guid id;
  guid user_id;
  std::string name;
  int color;
  int version = 0;
  bool is_deleted = false;
};

} // namespace repository::models
