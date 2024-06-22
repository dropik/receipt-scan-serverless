//
// Created by Daniil Ryzhkov on 20/05/2024.
//

#pragma once

#include <utility>

#include <aws/lambda-runtime/runtime.h>

#include <lambda/models/payloads/gateway_proxy.hpp>
#include <lambda/models/responses/gateway_proxy.hpp>
#include <lambda/logger.hpp>

#include "api_resource.hpp"

namespace rest {

class api_root : public api_resource {
 public:
  api_root();

  template<typename TMiddleware>
  void use(const TMiddleware &&middleware) {
    m_api_entrypoint = [next = m_api_entrypoint, middleware](const api_request_t &request) {
      return middleware(request, next);
    };
  }
  void use_exception_filter();
  void use_logging();

  api_response_t operator()(const api_request_t &request);
  aws::lambda_runtime::invocation_response operator()(const aws::lambda_runtime::invocation_request &request);

 private:
  std::function<api_response_t(const api_request_t &)> m_api_entrypoint;
};

}
