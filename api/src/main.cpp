#include <memory>

#include <aws/core/Aws.h>
#include <aws/core/http/HttpTypes.h>
#include <aws/core/utils/logging/ConsoleLogSystem.h>

#include <aws/s3/S3Client.h>

#ifdef DEBUG
#include <lambda/runtime.hpp>
#endif

#include "rest_api.hpp"

using namespace Aws;
using namespace aws::lambda_runtime;
using namespace Aws::Utils::Logging;
using namespace api;
using namespace rest;

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
  lambda::runtime::set_debug(argc, argv);
  lambda::runtime::load_payload(argc, argv);
#endif // DEBUG

  InitAPI(options);
  {
    auto api = create_api();

#ifdef DEBUG
    lambda::runtime::run_debug(api);
#else
    run_handler(api);
#endif // DEBUG
  }
  ShutdownAPI(options);

  return 0;
}

