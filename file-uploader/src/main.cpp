#include <aws/core/utils/memory/stl/AWSAllocator.h>
#include <stdlib.h>

#include <aws/lambda-runtime/runtime.h>

#include <aws/core/Aws.h>
#include <aws/core/client/ClientConfiguration.h>
#include <aws/core/http/HttpTypes.h>
#include <aws/core/utils/json/JsonSerializer.h>
#include <aws/core/utils/logging/ConsoleLogSystem.h>
#include <aws/core/utils/logging/LogLevel.h>
#include <aws/core/utils/logging/LogMacros.h>
#include <aws/core/utils/memory/stl/AWSString.h>
#include <aws/core/utils/memory/stl/AWSVector.h>

#include <aws/s3/S3Client.h>

#include "config.h"

char const TAG[] = "file-uploader";

using namespace Aws;
using namespace aws::lambda_runtime;
using namespace Aws::S3;
using namespace Aws::Utils::Logging;
using namespace Aws::Utils::Json;

static invocation_response lambda_handler(invocation_request const& request) {
  AWS_LOGSTREAM_INFO(TAG, "Version " << VERSION);

  JsonValue gatewayProxyEvent(request.payload);
  assert(gatewayProxyEvent.WasParseSuccessful());
  assert(gatewayProxyEvent.View().ValueExists("body"));

  JsonValue requestBody(gatewayProxyEvent.View().GetString("body"));
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
  
  // to make sure logs are written
  AWS_LOGSTREAM_FLUSH();

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
    run_handler(lambda_handler);
  }
  ShutdownAPI(options);

  return 0;
}

