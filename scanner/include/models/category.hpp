#pragma once

#include <string>

#include "models/common.hpp"

namespace scanner {
namespace models {

struct category {
  guid id;
  guid user_id;
  std::string name;
};

}  // namespace models
}  // namespace scanner