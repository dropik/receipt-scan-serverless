//
// Created by Daniil Ryzhkov on 04/08/2024.
//

#include "put_receipt.hpp"

namespace api {
namespace parameters {

repository::models::receipt parameters::put_receipt::to_repo(const std::string &user_id) const {
  std::vector<repository::models::receipt_item> repo_items;
  repo_items.reserve(items.size());
  for (int i = 0; i < items.size(); i++) {
    const auto &item = items[i];
    repo_items.push_back(item.to_repo(id, i));
  }

  return repository::models::receipt{
      .id = id,
      .user_id = user_id,
      .date = date,
      .total_amount = total_amount,
      .currency = currency,
      .store_name = store_name,
      .category = category.has_value() ? category.get_value() : "",
      .state = state,
      .image_name = image_name.has_value() ? image_name.get_value() : "",
      .version = version,
      .items = repo_items,
  };
}

}
}
