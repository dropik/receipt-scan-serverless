#pragma once

#include <string>

#include "common.hpp"

namespace repository {
namespace models {

struct category {
  guid id;
  guid user_id;
  std::string name;
};

}  // namespace models
}  // namespace repository