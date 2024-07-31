//
// Created by Daniil Ryzhkov on 31/07/2024.
//

#pragma once

#include "common.hpp"
namespace repository {
namespace models {

struct entity_event {
  guid id;
  guid device_id;
  std::string entity_type;
  guid entity_id;
  std::string event_type;

  static constexpr auto create = "create";
  static constexpr auto update = "update";
  static constexpr auto del = "delete";
};

}  // namespace models
}  // namespace repository
