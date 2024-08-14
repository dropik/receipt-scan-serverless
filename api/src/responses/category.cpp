//
// Created by Daniil Ryzhkov on 03/08/2024.
//

#include "category.hpp"

namespace api {
namespace responses {

category responses::category::from_repo(const repository::models::category &c) {
  return category{
      .id = c.id,
      .name = c.name,
      .color = c.color,
      .version = c.version
  };
}

}
}
