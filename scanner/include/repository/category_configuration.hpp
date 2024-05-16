#pragma once

#include "repository_configuration.hpp"
#include "models/category.hpp"

namespace scanner {
namespace repository {

template <>
class repository_configuration<models::category>
    : public common::base_repository_configuration<models::category> {
 public:
  repository_configuration() : base_repository_configuration() {
    HAS_TABLE("categories");

    HAS_ID(id) WITH_COLUMN("id");

    HAS_STRING(user_id) WITH_COLUMN("user_id");
    HAS_STRING(name) WITH_COLUMN("name");
  }
};

}  // namespace repository
}  // namespace scanner