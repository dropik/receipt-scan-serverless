#pragma once

#include "repository_configuration.hpp"
#include "models/receipt_item.hpp"

namespace scanner {
namespace repository {

template <>
class repository_configuration<models::receipt_item>
    : public common::base_repository_configuration<models::receipt_item> {
 public:
  repository_configuration() {
    HAS_TABLE("receipt_items");
    HAS_ID(id) WITH_COLUMN("id");
    HAS_STRING(receipt_id) WITH_COLUMN("receipt_id");
    HAS_STRING(description) WITH_COLUMN("description");
    HAS_DECIMAL(amount) WITH_COLUMN("amount");
    HAS_STRING(category) WITH_COLUMN("category");
    HAS_INT(sort_order) WITH_COLUMN("sort_order");
  }
};

}  // namespace repository
}  // namespace scanner