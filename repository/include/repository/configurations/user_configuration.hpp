//
// Created by Daniil Ryzhkov on 04/06/2024.
//

#pragma once

#include "repository/models/user.hpp"
#include "repository_configuration.hpp"

namespace repository::configurations {

template<>
class repository_configuration<models::user> : public common::base_repository_configuration<models::user> {
 public:
  repository_configuration() {
    HAS_TABLE("users");

    HAS_ID(id) WITH_COLUMN("id");
    HAS_BOOL(has_subscription) WITH_COLUMN("has_subscription");
  }
};

}
