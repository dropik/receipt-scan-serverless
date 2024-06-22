//
// Created by Daniil Ryzhkov on 22/06/2024.
//

#pragma once

#include <aws/core/Aws.h>
#include <di/service_factory.hpp>

namespace di {

template<>
struct service_factory<Aws::Client::ClientConfiguration> {
  template<typename TContainer, typename TPointerFactory>
  static auto create(TContainer &container, TPointerFactory &&factory) {
    auto config = factory();
#ifdef DEBUG
    config->region = AWS_REGION;
#endif
    return std::move(config);
  }
};

}
