//
// Created by Daniil Ryzhkov on 02/06/2024.
//

#pragma once

#include <type_traits>

#include <aws-lambda-cpp/models/lambda_payloads/gateway_proxy.hpp>
#include <aws-lambda-cpp/models/lambda_responses/gateway_proxy.hpp>

namespace rest {

typedef aws_lambda_cpp::models::lambda_payloads::base_gateway_proxy_request api_request_t;
typedef aws_lambda_cpp::models::lambda_responses::base_gateway_proxy_response api_response_t;

template<typename T, class Enabled = void>
struct has_id {
  constexpr static auto value = false;
};

template<typename T>
struct has_id<T, typename std::enable_if_t<!std::is_void<decltype(T().id)>::value>> {
  constexpr static auto value = true;
};

}