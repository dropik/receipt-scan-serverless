//
// Created by Daniil Ryzhkov on 22/06/2024.
//

#pragma once

#include <memory>
#include "service_factory.hpp"

namespace di {

template<typename TInterface, typename TService = TInterface>
struct transient {
  using interface = TInterface;
  using service = TService;

  template<typename T>
  using ptr = std::unique_ptr<T>;

  template<typename TContainer>
  auto get_or_create(TContainer &container) {
    using type = typename TContainer::template resolve<TService>::type;
    return service_factory<TService>::create(
        container,
        [](auto ...params) {
          return std::move(std::make_unique<type>(std::move(params)...));
        });
  }
};

}
