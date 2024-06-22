#include <memory>

#include <aws/core/Aws.h>
#include <aws/core/client/ClientConfiguration.h>
#include <aws/core/utils/logging/ConsoleLogSystem.h>
#include <aws/core/utils/memory/stl/AWSAllocator.h>
#include <aws/textract/TextractClient.h>
#include <aws/bedrock-runtime/BedrockRuntimeClient.h>

#include <lambda/logger.hpp>

#include <repository/client.hpp>
#include <lambda/lambda.hpp>
#include <di/container.hpp>

#ifdef DEBUG
#include <lambda/runtime.hpp>
#include <config.h>
#else
#include <aws/lambda-runtime/runtime.h>
#endif

#include "handler.hpp"
#include "factories.hpp"

using namespace Aws;
using namespace aws::lambda_runtime;
using namespace Aws::Utils::Logging;
using namespace Aws::Utils::Json;
using namespace Aws::Textract;
using namespace Aws::BedrockRuntime;
using namespace scanner;
using namespace di;

static std::function<std::shared_ptr<LogSystemInterface>()> GetConsoleLoggerFactory() {
  return [] {
    return Aws::MakeShared<ConsoleLogSystem>(
      "console_logger",
      LogLevel::Info);
  };
}

int main(int argc, char* argv[]) {

  SDKOptions options;
  options.loggingOptions.logLevel = Utils::Logging::LogLevel::Info;
  options.loggingOptions.logger_create_fn = GetConsoleLoggerFactory();
  std::shared_ptr<sql::Connection> db_connection;

#ifdef DEBUG
  lambda::runtime::set_debug(argc, argv);
  lambda::runtime::load_payload(argc, argv);
#endif  // DEBUG

  InitAPI(options);
  {
    std::shared_ptr<lambda::logger> l = std::make_shared<lambda::logger>("Scanner");

    Aws::Client::ClientConfiguration config;
#ifdef DEBUG
    config.region = AWS_REGION;
#endif

    try {
      auto h = [](auto req) {
        container<
            singleton<Aws::Client::ClientConfiguration>,
            singleton<repository::connection_settings>,
            singleton<lambda::logger>,
            singleton<TextractClient>,
            singleton<BedrockRuntimeClient>,

            scoped<repository::t_client, repository::client<>>,
            transient<t_handler, handler<>>
        > services;

        return services.template get<t_handler>()->operator()(req);
      };

#ifdef DEBUG
      lambda::runtime::run_debug(h);
#else
      run_handler(h);
#endif  // DEBUG

    } catch (std::exception &e) {
      l->error("Error occurred during execution of the function.");
    }
  }

  ShutdownAPI(options);

  return 0;
}

