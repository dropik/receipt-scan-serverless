//
// Created by Daniil Ryzhkov on 04/08/2024.
//

#include "receipt_item.hpp"

namespace api {
namespace responses {

receipt_item responses::receipt_item::from_repo(const repository::models::receipt_item &item) {
  return {
      .id = item.id,
      .description = item.description,
      .amount = item.amount,
      .category = item.category,
  };
}

}
}
