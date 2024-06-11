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

#ifdef DEBUG
#include <lambda/runtime.hpp>
#include <config.h>
#else
#include <aws/lambda-runtime/runtime.h>
#endif

#include "handler.hpp"

using namespace Aws;
using namespace aws::lambda_runtime;
using namespace Aws::Utils::Logging;
using namespace Aws::Utils::Json;
using namespace Aws::Textract;
using namespace Aws::BedrockRuntime;
using namespace scanner;

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
      auto stage = lambda::get_stage();
      auto connection_string = repository::get_connection_string(stage, config);

      auto repo = std::make_shared<repository::client>(connection_string, l);

      std::shared_ptr<TextractClient> textract_client =
          Aws::MakeShared<TextractClient>("textract_client", config);

      std::shared_ptr<BedrockRuntimeClient> bedrock_client =
          Aws::MakeShared<BedrockRuntimeClient>("bedrock_client", config);

      handler h(repo, textract_client, bedrock_client, l);
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

