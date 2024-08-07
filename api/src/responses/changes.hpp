//
// Created by Daniil Ryzhkov on 07/08/2024.
//

#pragma once

#include "category.hpp"
#include "budget.hpp"
#include "receipt.hpp"
#include "change.hpp"

namespace api {
namespace responses {

struct changes {
  std::vector<change<category>> categories;
  std::vector<change<budget>> budgets;
  std::vector<change<receipt>> receipts;

  JSON_BEGIN_SERIALIZER(changes)
      JSON_PROPERTY("categories", categories)
      JSON_PROPERTY("budgets", budgets)
      JSON_PROPERTY("receipts", receipts)
  JSON_END_SERIALIZER()
};

}
}
