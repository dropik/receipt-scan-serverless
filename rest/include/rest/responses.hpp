//
// Created by Daniil Ryzhkov on 02/06/2024.
//

#pragma once

#include "api_exception.hpp"
#include "types.hpp"

namespace rest {

api_response_t ok();
api_response_t bad_request();
api_response_t bad_request(const api_exception &e);
api_response_t unauthorized();
api_response_t not_found();
api_response_t method_not_allowed();
api_response_t internal_server_error();

template<typename T>
inline api_response_t ok(const T &payload) {
  api_response_t response;
  response.status_code = 200;
  response.set_body(aws_lambda_cpp::json::serialize(payload, true), false);
  return response;
}

template<typename TId>
inline api_response_t created_at(const std::string &req_path, const TId &id) {
  api_response_t response;
  response.status_code = 201;
  response.set_body("", false);
  auto location = req_path;
  if (location.back() != '/') {
    location += "/";
  }
  location += id_to_string(id);
  response.headers["Location"] = location;
  return response;
}

template<typename THandler, typename TBody = void, class Enabled = void>
struct empty_ok {
  api_response_t operator()(const api_request_t &request, const THandler &&handler, const TBody &body) {
    return ok();
  }
};

template<typename THandler, typename TBody>
struct empty_ok<THandler, TBody, typename std::enable_if_t<has_id<TBody>::value>> {
  api_response_t operator()(const api_request_t &request, const THandler &&handler, const TBody &body) {
    auto id = body.id;
    return created_at(request.path, id);
  }
};

template<typename THandler>
struct empty_ok<THandler, void, void> {
  api_response_t operator()(const api_request_t &request, const THandler &&handler) {
    return ok();
  }
};

template<typename THandler, typename TBody = void, class Enabled = void>
struct post_response {
  api_response_t operator()(const api_request_t &request, const THandler &&handler, const TBody &body) {
    return ok(handler(body));
  }
};

template<typename THandler, typename TBody>
struct post_response<THandler, TBody, typename std::enable_if_t<std::is_void<std::result_of_t<THandler(const TBody&)>>::value>> {
  api_response_t operator()(const api_request_t &request, const THandler &&handler, const TBody &body) {
    handler(body);
    return empty_ok<THandler, TBody>()(request, std::forward<THandler>(handler), body);
  }
};

template<typename THandler, class Enabled>
struct post_response<THandler, void, Enabled> {
  api_response_t operator()(const api_request_t &request, const THandler &&handler) {
    return ok(handler());
  }
};

template<typename THandler>
struct post_response<THandler, void, typename std::enable_if_t<std::is_void<std::result_of_t<THandler()>>::value>> {
  api_response_t operator()(const api_request_t &request, const THandler &&handler) {
    handler();
    return empty_ok<THandler, void>()(request, std::forward<THandler>(handler));
  }
};

}