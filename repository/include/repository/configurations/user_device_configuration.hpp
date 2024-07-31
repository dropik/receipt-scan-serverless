//
// Created by Daniil Ryzhkov on 31/07/2024.
//

#pragma once

#include "repository/models/user_device.hpp"
#include "repository_configuration.hpp"

namespace repository {
namespace configurations {

template<>
class repository_configuration<models::user_device>
    : public common::base_repository_configuration<models::user_device> {
 public:
  repository_configuration() {
    HAS_TABLE("user_devices");

    HAS_ID(id) WITH_COLUMN("id");

    HAS_STRING(user_id) WITH_COLUMN("user_id");
  }
};

}
}
