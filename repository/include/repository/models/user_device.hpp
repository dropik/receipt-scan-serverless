//
// Created by Daniil Ryzhkov on 31/07/2024.
//

#pragma once

#include "common.hpp"

namespace repository {
namespace models {

struct user_device {
  guid id;
  guid user_id;
};

}  // namespace models
}  // namespace repository
