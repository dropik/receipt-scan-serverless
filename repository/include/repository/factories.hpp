//
// Created by Daniil Ryzhkov on 22/06/2024.
//

#pragma once

#include <di/service_factory.hpp>
#include <lambda/factories.hpp>

#include <lambda/lambda.hpp>

#include "connection_settings.hpp"
#include "client.hpp"

namespace di {

template<>
struct service_factory<repository::connection_settings> {
  template<typename TContainer, typename TPointerFactory>
  static auto create(TContainer &container, TPointerFactory &&factory) {
    auto parameters = container.template get<parameter_manager>();
    auto connection_string = parameters->get("db-connection-string", "DB_CONNECTION_STRING");
    return std::move(factory(connection_string));
  }
};

}
