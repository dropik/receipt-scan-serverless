//
// Created by Daniil Ryzhkov on 22/06/2024.
//

#pragma once

#include <memory>
#include "service_factory.hpp"
#include "transient.hpp"
#include "singleton.hpp"
#include "scoped.hpp"

namespace di {

template<typename ...TServices>
class container {
 private:
  template<typename T, bool IsConst>
  static auto resolve_const(std::shared_ptr<T> ptr) {
    return std::move(std::static_pointer_cast<typename std::conditional<
        IsConst,
        const T,
        T>::type>(ptr));
  }

  template<typename T, bool IsConst>
  static auto resolve_const(std::unique_ptr<T> ptr) {
    return std::move(std::unique_ptr<typename std::conditional<IsConst, const T, T>::type>(std::move(ptr)));
  }

  template<typename T, typename C, std::size_t I>
  struct tuple_index_r;

  template<typename H, typename ...R, typename C, std::size_t I>
  struct tuple_index_r<std::tuple<H, R...>, C, I>
  : public std::conditional<std::is_same<C, H>::value,
                            std::integral_constant<std::size_t, I>,
  tuple_index_r<std::tuple<R...>, C, I+1>>::type
  {};

  template<typename C, std::size_t I>
  struct tuple_index_r<std::tuple<>, C, I>
  {};

  template<typename T, typename C>
  struct tuple_index_of
      : public std::integral_constant<std::size_t, tuple_index_r<T, C, 0>::value> {};

  using _tuple = std::tuple<TServices...>;
  _tuple m_services;

  template<typename TInterface>
  static constexpr auto &
  get_service(_tuple &_t) noexcept
  { return std::get<typename std::tuple_element<tuple_index_of<std::tuple<typename TServices::interface...>, TInterface>::value, _tuple>::type>(_t); }


 public:
  template<typename T>
  auto get() {
    auto &service = get_service<typename std::decay<T>::type>(m_services);
    using resolved_type = typename resolve<typename std::decay<decltype(service)>::type::service>::type;
    return std::move(resolve_const<resolved_type, std::is_const<T>::value>(service.template get_or_create(*this)));
  }

  template<typename TService>
  struct resolve {
    using type = TService;
  };

  template<template<typename ...> class TService, typename ...TArgs>
  struct resolve<TService<TArgs...>> {
    using type = TService<
        typename std::decay<decltype(get_service<typename std::decay<TArgs>::type>(std::declval<_tuple &>()))>::type::template ptr<
            typename std::conditional<
                std::is_const<TArgs>::value,
                const typename resolve<typename std::decay<decltype(get_service<typename std::decay<TArgs>::type>(std::declval<_tuple &>()))>::type::service>::type,
                typename resolve<typename std::decay<decltype(get_service<typename std::decay<TArgs>::type>(std::declval<_tuple &>()))>::type::service>::type>::type>...>;
  };
};

}
