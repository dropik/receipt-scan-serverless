//
// Created by Daniil Ryzhkov on 22/06/2024.
//

#pragma once

#include <di/service_factory.hpp>
#include <lambda/factories.hpp>
#include <repository/factories.hpp>

#include <aws/textract/TextractClient.h>
#include <aws/bedrock-runtime/BedrockRuntimeClient.h>

namespace di {

template<>
struct service_factory<lambda::logger> {
  template<typename TContainer, typename TPointerFactory>
  static auto create(TContainer &container, TPointerFactory &&factory) {
    return std::move(factory("Scanner"));
  }
};

template<>
struct service_factory<Aws::Textract::TextractClient> {
  template<typename TContainer, typename TPointerFactory>
  static auto create(TContainer &container, TPointerFactory &&factory) {
    return std::move(factory(*container.template get<Aws::Client::ClientConfiguration>()));
  }
};

template<>
struct service_factory<Aws::BedrockRuntime::BedrockRuntimeClient> {
  template<typename TContainer, typename TPointerFactory>
  static auto create(TContainer &container, TPointerFactory &&factory) {
    return std::move(factory(*container.template get<Aws::Client::ClientConfiguration>()));
  }
};

}
