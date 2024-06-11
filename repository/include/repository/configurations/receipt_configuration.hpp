#pragma once

#include <string>

#include <repository/models/receipt.hpp>
#include "repository_configuration.hpp"

namespace repository {
namespace configurations {

template <>
class repository_configuration<models::receipt>
    : public common::base_repository_configuration<models::receipt> {
 public:
  repository_configuration() {
    HAS_TABLE("receipts");

    HAS_ID(id) WITH_COLUMN("id");

    HAS_STRING(user_id) WITH_COLUMN("user_id");
    HAS_STRING(date) WITH_COLUMN("date");
    HAS_DECIMAL(total_amount) WITH_COLUMN("total_amount");
    HAS_STRING(currency) WITH_COLUMN("currency");
    HAS_STRING(store_name) WITH_COLUMN("store_name");
    HAS_STRING(category) WITH_COLUMN("category");
    HAS_STRING(state) WITH_COLUMN("state");
  }
};

}  // namespace configurations
}  // namespace repository
