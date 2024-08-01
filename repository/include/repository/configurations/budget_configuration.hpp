//
// Created by Daniil Ryzhkov on 31/07/2024.
//

#pragma once

#include "repository_configuration.hpp"
#include "repository/models/budget.hpp"

namespace repository {
namespace configurations {

template<>
class repository_configuration<models::budget>
    : public common::base_repository_configuration<models::budget> {
 public:
  repository_configuration() {
    HAS_TABLE("budgets");

    HAS_ID(id) WITH_COLUMN("id");

    HAS_STRING(user_id) WITH_COLUMN("user_id");
    HAS_STRING(month) WITH_COLUMN("month");
    HAS_DOUBLE(amount) WITH_COLUMN("amount");

    HAS_VERSION();
    HAS_TRACKING(id, user_id) WITH_ENTITY_NAME("budget");
  }
};

}
}
