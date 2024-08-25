//
// Created by Daniil Ryzhkov on 04/08/2024.
//

#include "receipt.hpp"

namespace api::responses {

receipt receipt::from_repo(const repository::models::receipt &receipt) {
  std::vector<std::string> categories;
  std::vector<receipt_item> items;
  categories.reserve(receipt.items.size());
  items.reserve(receipt.items.size());

  if (receipt.items.empty()) {
    categories.push_back(receipt.category);
  }

  for (const auto &item : receipt.items) {
    items.push_back(receipt_item::from_repo(item));
    if (std::find(categories.begin(), categories.end(), item.category) == categories.end()) {
      categories.push_back(item.category);
    }
  }

  return {
      .id = receipt.id,
      .date = receipt.date,
      .total_amount = receipt.total_amount,
      .currency = receipt.currency,
      .store_name = receipt.store_name,
      .categories = categories,
      .state = receipt.state,
      .image_name = receipt.image_name.empty() ? lambda::nullable<std::string>() : receipt.image_name,
      .version = receipt.version,
      .items = items,
  };
}

}
