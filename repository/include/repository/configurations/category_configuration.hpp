#pragma once

#include <repository/models/category.hpp>
#include "repository_configuration.hpp"

namespace repository::configurations {

template <>
class repository_configuration<models::category>
    : public common::base_repository_configuration<models::category> {
 public:
  repository_configuration() : base_repository_configuration() {
    HAS_TABLE("categories");

    HAS_ID(id) WITH_COLUMN("id");

    HAS_STRING(user_id) WITH_COLUMN("user_id");
    HAS_STRING(name) WITH_COLUMN("name");
    HAS_INT(color) WITH_COLUMN("color");
    HAS_INT(icon) WITH_COLUMN("icon");
    HAS_BOOL(is_deleted) WITH_COLUMN("is_deleted");

    HAS_VERSION();
  }
};

} // namespace repository::configurations
