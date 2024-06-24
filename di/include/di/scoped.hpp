//
// Created by Daniil Ryzhkov on 22/06/2024.
//

#pragma once

#include <memory>
#include "service_factory.hpp"

namespace di {

template<typename TInterface, typename TService = TInterface>
struct scoped {
  using interface = TInterface;
  using service = TService;

  template<typename T>
  using ptr = std::shared_ptr<T>;

  template<typename TContainer>
  auto get_or_create(TContainer &container) {
    using type = typename TContainer::template resolve<TService>::type;
    if (!m_instance) {
      m_instance = service_factory<TService>::create(
          container,
          [](auto ...params) {
            return std::move(std::make_shared<type>(std::move(params)...));
          });
    }
    return std::static_pointer_cast<type>(m_instance);
  }

 private:
  ptr<void> m_instance = nullptr;
};

}
