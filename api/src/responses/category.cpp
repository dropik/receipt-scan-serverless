//
// Created by Daniil Ryzhkov on 03/08/2024.
//

#include "category.hpp"

namespace api::responses {

category responses::category::from_repo(const repository::models::category &c) {
  return category{
      .id = c.id,
      .name = c.name,
      .color = c.color,
      .icon = c.icon == 0 ? lambda::nullable<int>() : lambda::nullable<int>(c.icon),
      .version = c.version
  };
}

}
