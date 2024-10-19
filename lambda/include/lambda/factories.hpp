//
// Created by Daniil Ryzhkov on 22/06/2024.
//

#pragma once

#include <aws/core/Aws.h>
#include <di/service_factory.hpp>
#include <di/parameter_manager.hpp>
#include "config.h"
#include "lambda.hpp"

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

template<>
struct service_factory<parameter_manager> {
  template<typename TContainer, typename TPointerFactory>
  static auto create(TContainer &container, TPointerFactory &&factory) {
    auto stage = lambda::get_stage();
    auto client_configuration = container.template get<Aws::Client::ClientConfiguration>();
    return std::move(factory(stage, *client_configuration));
  }
};

}
