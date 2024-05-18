#include <memory>
#include <cstdlib>

#include <aws/lambda-runtime/runtime.h>

#include <aws/core/Aws.h>
#include <aws/core/client/ClientConfiguration.h>
#include <aws/core/http/HttpTypes.h>
#include <aws/core/utils/logging/ConsoleLogSystem.h>

#include <aws/s3/S3Client.h>

#include <aws-lambda-cpp/common/logger.hpp>
#include <aws-lambda-cpp/common/runtime.hpp>
#include <aws-lambda-cpp/http/responses.hpp>
#include <aws-lambda-cpp/models/lambda_payloads/gateway_proxy.hpp>

#include "config.h"
#include "models/upload_file_params.hpp"
#include "models/upload_file_response.hpp"

using namespace Aws;
using namespace aws::lambda_runtime;
using namespace Aws::S3;
using namespace Aws::Utils::Logging;
using namespace Aws::Utils::Json;
using namespace aws_lambda_cpp::json;
using namespace aws_lambda_cpp::models::lambda_payloads;
using namespace api::models;

static invocation_response lambda_handler(
  const aws_lambda_cpp::common::logger& logger,
  const invocation_request& request) {

  logger.info("Version %s", VERSION);

  gateway_proxy_request<upload_file_params> gpr = deserialize<gateway_proxy_request<upload_file_params>>(request.payload);

  upload_file_params params = gpr.get_payload();

  Aws::Client::ClientConfiguration config;
#ifdef DEBUG
  config.region = AWS_REGION;
#endif
  S3Client s3Client(config);
  
  std::string bucketName(getenv(IMAGES_BUCKET));
  std::string presignedUrl = s3Client.GeneratePresignedUrlWithSSES3(
    bucketName,
    params.name,
    Aws::Http::HttpMethod::HTTP_PUT);

  upload_file_response response;
  response.url = presignedUrl;

  return aws_lambda_cpp::http::ok(response);
}

static std::function<std::shared_ptr<LogSystemInterface>()> GetConsoleLoggerFactory() {
  return [] {
    return Aws::MakeShared<ConsoleLogSystem>(
      "console_logger",
      LogLevel::Info);
  };
}

int main(int argc, char** argv) {
  SDKOptions options;
  options.loggingOptions.logLevel = Utils::Logging::LogLevel::Info;
  options.loggingOptions.logger_create_fn = GetConsoleLoggerFactory();

#ifdef DEBUG
  aws_lambda_cpp::runtime::set_debug(argc, argv);
  aws_lambda_cpp::runtime::load_payload(argc, argv);
#endif // DEBUG

  InitAPI(options);
  {
    aws_lambda_cpp::common::logger logger("Api");
    std::function<invocation_response(const invocation_request&)> handler = [&](const invocation_request& req) {
      return lambda_handler(logger, req);
    };

#ifdef DEBUG
    aws_lambda_cpp::runtime::run_debug(handler);
#else
    run_handler(handler);
#endif // DEBUG
  }
  ShutdownAPI(options);

  return 0;
}

