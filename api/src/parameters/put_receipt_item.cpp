//
// Created by Daniil Ryzhkov on 04/08/2024.
//

#include "put_receipt_item.hpp"

namespace api {
namespace parameters {

repository::models::receipt_item put_receipt_item::to_repo(const guid_t &receipt_id, int index) const {
  return repository::models::receipt_item{
      .id = id,
      .receipt_id = receipt_id,
      .description = description,
      .amount = amount,
      .category = category,
      .sort_order = index,
  };
}

}
}
