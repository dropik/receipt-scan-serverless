//
// Created by Daniil Ryzhkov on 02/06/2024.
//

#pragma once

#include <type_traits>

#include <lambda/models/payloads/gateway_proxy.hpp>
#include <lambda/models/responses/gateway_proxy.hpp>

namespace rest {

typedef lambda::models::payloads::base_gateway_proxy_request api_request_t;
typedef lambda::models::responses::base_gateway_proxy_response api_response_t;

template<typename T, class Enabled = void>
struct has_id {
  constexpr static auto value = false;
};

template<typename T>
struct has_id<T, typename std::enable_if_t<!std::is_void<decltype(T().id)>::value>> {
  constexpr static auto value = true;
};

}
