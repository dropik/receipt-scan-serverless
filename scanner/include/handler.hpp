#pragma once

#include <memory>

#include "conncpp/Connection.hpp"

#include <aws/lambda-runtime/runtime.h>
#include <aws/textract/TextractClient.h>
#include <aws-lambda-cpp/common/logger.hpp>

#include "repository/repository.hpp"

namespace scanner {

class handler {
 public:
  handler(std::shared_ptr<repository::repository> repository,
          std::shared_ptr<const Aws::Textract::TextractClient> textract_client,
          std::shared_ptr<const aws_lambda_cpp::common::logger> logger);

  aws::lambda_runtime::invocation_response handle_request(
      const aws::lambda_runtime::invocation_request& request);

 private:
  std::shared_ptr<repository::repository> m_repository;
  std::shared_ptr<const Aws::Textract::TextractClient> m_textract_client;
  std::shared_ptr<const aws_lambda_cpp::common::logger> m_logger;
  bool try_parse_date(std::string& result, const std::string& input);
  bool try_parse_total(long double& result, const std::string& input);
};

}  // namespace scanner
