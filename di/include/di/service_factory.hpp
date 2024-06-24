//
// Created by Daniil Ryzhkov on 22/06/2024.
//

#pragma once

#include <utility>

namespace di {

template<typename TService>
struct service_factory {
  template<typename TContainer, typename TPointerFactory>
  static auto create(TContainer &container, TPointerFactory &&factory) {
    return std::move(factory());
  }
};

template<template<typename ...> class TService, typename ...TArgs>
struct service_factory<TService<TArgs...>> {
  template<typename TContainer, typename TPointerFactory>
  static auto create(TContainer &container, TPointerFactory &&factory) {
    return std::move(factory(std::move(container.template get<TArgs>())...));
  }
};

}
