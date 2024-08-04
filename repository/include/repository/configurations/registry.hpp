//
// Created by Daniil Ryzhkov on 23/06/2024.
//

#pragma once

#include <tuple>

#include "repository/configurations/category_configuration.hpp"
#include "repository/configurations/receipt_configuration.hpp"
#include "repository/configurations/receipt_item_configuration.hpp"
#include "repository/configurations/user_configuration.hpp"
#include "repository/configurations/budget_configuration.hpp"
#include "repository/configurations/user_device_configuration.hpp"
#include "repository/configurations/entity_event_configuration.hpp"

namespace repository {
namespace configurations {

template<typename ...TModels>
struct configurations_registry {
  template<typename T>
  repository_configuration<T> &get() {
    return std::get<repository_configuration<T>>(configurations);
  }

 private:
  std::tuple<repository_configuration<TModels>...> configurations;
};

using registry = configurations_registry<
    models::category,
    models::receipt,
    models::receipt_item,
    models::user,
    models::budget,
    models::user_device,
    models::entity_event
>;

}
}
