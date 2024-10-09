//
// Created by Daniil Ryzhkov on 03/08/2024.
//

#pragma once

#include "../../src/factories.hpp"
#include "mock_s3_client.hpp"

namespace di {

template<>
struct service_factory<api::integration_tests::mocks::mock_s3_client> {
  template<typename TContainer, typename TPointerFactory>
  static auto create(TContainer &container, TPointerFactory &&factory) {
    return std::move(factory(*container.template get<Aws::Client::ClientConfiguration>()));
  }
};

template<>
struct service_factory<api::integration_tests::mocks::mock_cognito_idp_client> {
  template<typename TContainer, typename TPointerFactory>
  static auto create(TContainer &container, TPointerFactory &&factory) {
    return std::move(factory(*container.template get<Aws::Client::ClientConfiguration>()));
  }
};

}
