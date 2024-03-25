#include <aws-lambda-cpp/common/nullable.hpp>
#include <memory>
#include <stdlib.h>

#include <aws/lambda-runtime/runtime.h>

#include <aws/core/Aws.h>
#include <aws/core/client/ClientConfiguration.h>
#include <aws/core/http/HttpTypes.h>
#include <aws/core/utils/json/JsonSerializer.h>
#include <aws/core/utils/logging/ConsoleLogSystem.h>

#include <aws/s3/S3Client.h>

#include <aws-lambda-cpp/common/logger.hpp>
#include <aws-lambda-cpp/common/json.hpp>
#include <aws-lambda-cpp/models/lambda_payloads/gateway_proxy.hpp>

#include "config.h"

using namespace Aws;
using namespace aws::lambda_runtime;
using namespace Aws::S3;
using namespace Aws::Utils::Logging;
using namespace Aws::Utils::Json;
using namespace aws_lambda_cpp::json;
using namespace aws_lambda_cpp::models::lambda_payloads;

static invocation_response lambda_handler(
  const aws_lambda_cpp::common::logger& logger,
  const invocation_request& request) {

  logger.info("Version %s", VERSION);

  gateway_proxy_request gpr = deserialize<gateway_proxy_request>(request.payload);
  std::string gpr_json = serialize(gpr);
  logger.info("Received AWS Gateway Proxy event: %s", gpr_json.c_str());

  std::string body_s = gpr.get_body();
  logger.info("The request payload is: %s", body_s.c_str());

  JsonValue requestBody(body_s);
  assert(requestBody.WasParseSuccessful());
  assert(requestBody.View().ValueExists("name"));
  std::string filename = requestBody.View().GetString("name");

  Aws::Client::ClientConfiguration config;
#ifdef DEBUG
  config.region = AWS_REGION;
#endif
  S3Client s3Client(config);
  
  std::string bucketName(getenv(IMAGES_BUCKET));
  std::string presignedUrl = s3Client.GeneratePresignedUrlWithSSES3(
    bucketName,
    filename,
    Aws::Http::HttpMethod::HTTP_PUT);

  JsonValue body;
  body.WithString("url", presignedUrl);

  JsonValue resp;
  resp
    .WithString("body", body.View().WriteCompact())
    .WithInteger("statusCode", 200);
  
  return invocation_response::success(resp.View().WriteCompact(), "application/json");
}

std::function<std::shared_ptr<LogSystemInterface>()> GetConsoleLoggerFactory() {
  return [] {
    return Aws::MakeShared<ConsoleLogSystem>(
      "console_logger",
      LogLevel::Info);
  };
}

int main() {
  SDKOptions options;
  options.loggingOptions.logLevel = Utils::Logging::LogLevel::Info;
  options.loggingOptions.logger_create_fn = GetConsoleLoggerFactory();

  InitAPI(options);
  {
    aws_lambda_cpp::common::logger logger("file-uploader");
    std::function<invocation_response(const invocation_request&)> handler = [&](const invocation_request& req) {
      return lambda_handler(logger, req);
    };
    run_handler(handler);
  }
  ShutdownAPI(options);

  return 0;
}

