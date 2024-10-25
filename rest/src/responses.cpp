//
// Created by Daniil Ryzhkov on 02/06/2024.
//

#include <rest/responses.hpp>

namespace rest {

api_response_t ok() {
  api_response_t response;
  response.status_code = 200;
  response.set_body("", false);
  return response;
}

api_response_t no_content() {
  api_response_t response;
  response.status_code = 204;
  response.set_body("", false);
  return response;
}

api_response_t bad_request() {
  api_response_t response;
  response.status_code = 400;
  response.set_body("", false);
  return response;
}

api_response_t bad_request(const api_exception &e) {
  api_response_t response;
  response.status_code = 400;
  response.set_body(lambda::json::serialize(e, true), false);
  return response;
}

api_response_t unauthorized() {
  api_response_t response;
  response.status_code = 401;
  response.set_body("", false);
  return response;
}

api_response_t forbidden() {
  api_response_t response;
  response.status_code = 403;
  response.set_body("", false);
  return response;
}

api_response_t not_found() {
  api_response_t response;
  response.status_code = 404;
  response.set_body("", false);
  return response;
}

api_response_t method_not_allowed() {
  api_response_t response;
  response.status_code = 405;
  response.set_body("", false);
  return response;
}

api_response_t internal_server_error() {
  api_response_t response;
  response.status_code = 500;
  response.set_body("", false);
  return response;
}

api_response_t conflict(const std::optional<std::string> &message) {
  api_response_t response;
  response.status_code = 409;
  if (message.has_value()) {
    response.set_body('"' + message.value() + '"', false);
  }
  return response;
}

}
