//
// Created by Daniil Ryzhkov on 02/06/2024.
//

#pragma once

#include <memory>

#include <lambda/logger.hpp>
#include <aws/s3/S3Client.h>
#include "repository/client.hpp"
#include "models/identity.hpp"

#include "services/file_service.hpp"
#include "services/user_service.hpp"
#include "services/receipt_service.hpp"
#include "services/category_service.hpp"

namespace api {

template<typename TService>
struct di;

template<>
struct di<lambda::logger> {
  static std::shared_ptr<lambda::logger> get();
};

template<>
struct di<Aws::Client::ClientConfiguration> {
  static std::shared_ptr<const Aws::Client::ClientConfiguration> get();
};

template<>
struct di<Aws::S3::S3Client> {
  static std::shared_ptr<Aws::S3::S3Client> get();
};

template<>
struct di<repository::client> {
  static std::shared_ptr<repository::client> get();
};

template<>
struct di<models::identity> {
  static const models::identity &get();
};

template<>
struct di<services::file_service> {
  static std::unique_ptr<services::file_service> get();
};

template<>
struct di<services::user_service> {
  static std::unique_ptr<services::user_service> get();
};

template<>
struct di<services::receipt_service> {
  static std::unique_ptr<services::receipt_service> get();
};

template<>
struct di<services::category_service> {
  static std::unique_ptr<services::category_service> get();
};

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

template<typename TInterface, typename TService = TInterface>
struct transient {
  using interface = TInterface;
  using service = TService;

  template<typename T>
  using ptr = std::unique_ptr<T>;

  template<typename TResolvedType, typename TContainer>
  auto get_or_create(TContainer &container) {
    return service_factory<TService>::create(
        container,
        [](auto ...params) {
          return std::move(std::make_unique<TResolvedType>(std::move(params)...));
        });
  }
};

template<typename TInterface, typename TService = TInterface>
struct singleton {
  using interface = TInterface;
  using service = TService;

  template<typename T>
  using ptr = std::shared_ptr<T>;

  template<typename TResolvedType, typename TContainer>
  auto get_or_create(TContainer &container) {
    static ptr<TResolvedType> instance;
    if (!instance) {
      instance = service_factory<TService>::create(
          container,
          [](auto ...params) {
            return std::move(std::make_shared<TResolvedType>(std::move(params)...));
          });
    }
    return instance;
  }
};

template<typename TInterface, typename TService = TInterface>
struct scoped {
  using interface = TInterface;
  using service = TService;

  template<typename T>
  using ptr = std::shared_ptr<T>;

  template<typename TResolvedType, typename TContainer>
  auto get_or_create(TContainer &container) {
    if (!m_instance) {
      m_instance = service_factory<TService>::create(
          container,
          [](auto ...params) {
            return std::move(std::make_shared<TResolvedType>(std::move(params)...));
          });
    }
    return std::static_pointer_cast<TResolvedType>(m_instance);
  }

 private:
  ptr<void> m_instance;
};

class a {};

struct ib {};

template<typename ia = const a>
class b {
 public:
  constexpr explicit b(ia _a) : m_a(std::move(_a)) {}
 private:
  ia m_a;
};

struct ic {};

template<typename ib = ib>
class c {
 public:
  constexpr explicit c(ib _b) {}
};

struct id {};

template<typename ic = ic, typename a = const a, typename ib = ib>
class d {
 public:
  constexpr explicit d(ic _c, a _a, ib _ib) {}
};

template<typename ...TServices>
class service_container {
 public:
  using _tuple = std::tuple<TServices...>;

  template<typename T>
  auto get() {
    auto service = get_service<typename std::decay<T>::type>(m_services);
    using resolved_type = typename resolve<typename decltype(service)::service>::type;
    return std::move(resolve_const<resolved_type, std::is_const<T>::value>(service.template get_or_create<resolved_type>(*this)));
  }

  template<typename T, bool IsConst>
  auto resolve_const(std::shared_ptr<T> ptr) {
    return std::move(std::static_pointer_cast<typename std::conditional<
        IsConst,
        const T,
        T>::type>(ptr));
  }

  template<typename T, bool IsConst>
  auto resolve_const(std::unique_ptr<T> ptr) {
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

  template<typename TInterface>
  static constexpr auto &
  get_service(_tuple &_t) noexcept
  { return std::get<typename std::tuple_element<tuple_index_of<std::tuple<typename TServices::interface...>, TInterface>::value, _tuple>::type>(_t); }

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

 private:
  _tuple m_services;
};

inline void f() {
  service_container<
      singleton<a>,
      transient<ib, b<>>,
      scoped<ic, c<>>,
      transient<id, d<>>
  > services;
  auto an_a = services.get<const a>();
  auto a_b = services.get<ib>();
  auto a_c = services.get<ic>();
  auto a_d = services.get<id>();
}

}
