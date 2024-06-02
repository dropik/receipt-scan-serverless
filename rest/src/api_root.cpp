//
// Created by Daniil Ryzhkov on 02/06/2024.
//

#include <rest/api_root.hpp>

namespace rest {

api_root::api_root() {
  m_api_entrypoint = [&](const api_request_t &request) {
    return route(request, request.path);
  };
}

void api_root::use_exception_filter() {
  use([this](const api_request_t &request, const auto &next) {
    try {
      return next(request);
    } catch (api_exception &e) {
      if (m_logger) {
        m_logger->info("API Exception: %d %s", e.error, e.message.c_str());
      }
      return bad_request(e);
    } catch (std::exception &e) {
      if (m_logger) {
        m_logger->error("Internal error: %s", e.what());
      }
      return internal_server_error();
    }
  });
}

void api_root::use_logging(const std::shared_ptr<aws_lambda_cpp::common::logger> &logger) {
  m_logger = logger;
  use([this](const api_request_t &request, const auto &next) {
    m_logger->info("Request: %s %s", request.http_method.c_str(), request.path.c_str());
    auto response = next(request);
    m_logger->info("%s %s Response: %d", request.http_method.c_str(), request.path.c_str(), response.status_code);
    return response;
  });
}

api_response_t api_root::operator()(const api_request_t &request) {
  return m_api_entrypoint(request);
}

aws::lambda_runtime::invocation_response api_root::operator()(const aws::lambda_runtime::invocation_request &request) {
  api_request_t gpr = aws_lambda_cpp::json::deserialize<api_request_t>(request.payload);

  api_response_t response = this->operator()(gpr);

  auto response_json = aws_lambda_cpp::json::serialize(response, true);
  return aws::lambda_runtime::invocation_response::success(response_json, "application/json");
}

}