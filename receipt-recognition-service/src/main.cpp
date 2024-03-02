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

#include <mysqlx/xdevapi.h>

#include "config.h"

char const TAG[] = "receipt-recognition-service";
std::string connection_string;

using namespace aws::lambda_runtime;
using namespace Aws::Utils::Logging;
using namespace Aws::Utils::Json;
using namespace mysqlx;

static invocation_response lambda_handler(invocation_request const& request) {
  AWS_LOGSTREAM_INFO(TAG, "Version " << VERSION);

  AWS_LOGSTREAM_INFO(TAG, "Establishing connection with database...");
  Session sess(connection_string);
  AWS_LOGSTREAM_INFO(TAG, "Successfully connected to datbase!");

  sess.close();

  AWS_LOGSTREAM_FLUSH();

  return invocation_response::success("{\"statusCode\": 200}", "application/json");
}

std::function<std::shared_ptr<LogSystemInterface>()> GetConsoleLoggerFactory() {
  return [] {
    return Aws::MakeShared<ConsoleLogSystem>(
      "console_logger",
      LogLevel::Info);
  };
}

int main() {
  using namespace Aws;

  SDKOptions options;
  options.loggingOptions.logLevel = Utils::Logging::LogLevel::Info;
  options.loggingOptions.logger_create_fn = GetConsoleLoggerFactory();

#ifdef DEBUG
  connection_string = getenv("DB_CONNECTION_STRING");
#endif

  InitAPI(options);
  {
    run_handler(lambda_handler);
  }
  ShutdownAPI(options);

  return 0;
}

