//
// Created by Daniil Ryzhkov on 04/06/2024.
//

#pragma once

#include <string>

namespace api {
namespace models {

struct identity {
  std::string user_id;
};

extern identity current_identity;

}
}