//
// Created by Daniil Ryzhkov on 22/06/2024.
//

#pragma once

#include <di/service_factory.hpp>

#include <lambda/lambda.hpp>

#include "connection_settings.hpp"
#include "client.hpp"

namespace di {

template<>
struct service_factory<repository::connection_settings> {
  template<typename TContainer, typename TPointerFactory>
  static auto create(TContainer &container, TPointerFactory &&factory) {
    auto stage = lambda::get_stage();
    auto connection_string = repository::get_connection_string(stage, *container.template get<Aws::Client::ClientConfiguration>());
    return std::move(factory(connection_string));
  }
};

}
