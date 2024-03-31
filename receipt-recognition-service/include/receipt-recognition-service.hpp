#pragma once

#include "conncpp/Connection.hpp"
#include <aws-lambda-cpp/common/logger.hpp>
#include <aws/lambda-runtime/runtime.h>
#include <aws/textract/TextractClient.h>
#include <memory>

class receipt_recognition_service {
  public:
    receipt_recognition_service(
      const std::shared_ptr<const Aws::Textract::TextractClient> & textract_client,
      const std::shared_ptr<const aws_lambda_cpp::common::logger>& logger,
      const std::shared_ptr<sql::Connection>& db_connection);

    aws::lambda_runtime::invocation_response handle_request(
      const aws::lambda_runtime::invocation_request& request);

  private:
    std::shared_ptr<const Aws::Textract::TextractClient> m_textract_client;
    std::shared_ptr<const aws_lambda_cpp::common::logger> m_logger;
    std::shared_ptr<sql::Connection> m_db_connection;
};

