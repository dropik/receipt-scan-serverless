//
// Created by Daniil Ryzhkov on 04/08/2024.
//

#include "receipt.hpp"

namespace api::responses {

receipt receipt::from_repo(const repository::models::receipt &receipt) {
  std::vector<std::string>                         categories;
  std::vector<receipt_item>                        items;
  std::map<std::string, long double>               category_totals;
  std::vector<std::pair<std::string, long double>> category_totals_vec;

  categories.reserve(receipt.items.size());
  items.reserve(receipt.items.size());
  category_totals_vec.reserve(receipt.items.size());

  if (receipt.items.empty()) {
    categories.push_back(receipt.category);
  } else {
    for (const auto &item : receipt.items) {
      items.push_back(receipt_item::from_repo(item));
      if (!category_totals.contains(item.category)) {
        category_totals[item.category] = 0;
      }
      category_totals[item.category] += item.amount;
    }
    for (const auto &pair : category_totals) {
      category_totals_vec.emplace_back(pair);
    }
    std::sort(category_totals_vec.begin(), category_totals_vec.end(), [](const auto &a, const auto &b) {
      return a.second > b.second;
    });
    for (const auto &pair : category_totals_vec) {
      categories.push_back(pair.first);
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
