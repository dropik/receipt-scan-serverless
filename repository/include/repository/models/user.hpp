//
// Created by Daniil Ryzhkov on 04/06/2024.
//

#pragma once

#include "common.hpp"

namespace repository::models {

struct user {
  guid id;
  bool has_subscription = false;
};

}
