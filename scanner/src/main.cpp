#include <memory>

#include <aws/core/Aws.h>
#include <aws/core/client/ClientConfiguration.h>
#include <aws/core/utils/logging/ConsoleLogSystem.h>
#include <aws/core/utils/memory/stl/AWSAllocator.h>
#include <aws/textract/TextractClient.h>
#include <aws/bedrock-runtime/BedrockRuntimeClient.h>
#include <aws/ssm/SSMClient.h>
#include <aws/ssm/model/GetParameterRequest.h>

#include <aws-lambda-cpp/common/logger.hpp>
#include <aws-lambda-cpp/common/string_utils.hpp>

#include <repository/client.hpp>

#ifdef DEBUG
#include <aws-lambda-cpp/common/runtime.hpp>
#include <config.h>
#else
#include <aws/lambda-runtime/runtime.h>
#endif

#include "handler.hpp"

using namespace aws::lambda_runtime;
using namespace Aws::Utils::Logging;
using namespace Aws::Utils::Json;
using namespace Aws::Textract;
using namespace Aws::BedrockRuntime;
using namespace aws_lambda_cpp;
using namespace aws_lambda_cpp::common;
using namespace scanner;

static std::function<std::shared_ptr<LogSystemInterface>()> GetConsoleLoggerFactory() {
  return [] {
    return Aws::MakeShared<ConsoleLogSystem>(
      "console_logger",
      LogLevel::Info);
  };
}

static std::string get_stage(const std::shared_ptr<logger>& logger) {
  std::string function_name = getenv("AWS_LAMBDA_FUNCTION_NAME");
  logger->info("Executing function %s", function_name.c_str());

  auto envStartPos = function_name.find_last_of('-');
  std::string stage = function_name.substr(
      envStartPos + 1, function_name.size() - envStartPos - 1);
  logger->info("Running on stage %s", stage.c_str());

  return stage;
}

static std::string get_connection_string(const std::string& stage, const Aws::Client::ClientConfiguration& config) {
  auto conn_env = getenv("DB_CONNECTION_STRING");
  if (conn_env != nullptr) {
    return conn_env;
  }

  std::string ssmPrefix = str_format("/receipt-scan/%s", stage.c_str());
  Aws::SSM::SSMClient ssmClient(config);
  Aws::SSM::Model::GetParameterRequest connStrReq;
  connStrReq
      .WithName(str_format("%s/db-connection-string", ssmPrefix.c_str()))
      .WithWithDecryption(true);
  Aws::SSM::Model::GetParameterOutcome outcome =
      ssmClient.GetParameter(connStrReq);
  if (!outcome.IsSuccess()) {
    throw std::runtime_error(
        str_format("Error occurred while obtaining parameter from ssm: %s",
                   outcome.GetError().GetMessage().c_str()));
  }
  return outcome.GetResult().GetParameter().GetValue();
}

int main(int argc, char* argv[]) {
  using namespace Aws;

  SDKOptions options;
  options.loggingOptions.logLevel = Utils::Logging::LogLevel::Info;
  options.loggingOptions.logger_create_fn = GetConsoleLoggerFactory();
  std::shared_ptr<sql::Connection> db_connection;

#ifdef DEBUG
  aws_lambda_cpp::runtime::set_debug(argc, argv);
  aws_lambda_cpp::runtime::load_payload(argc, argv);
#endif  // DEBUG

  InitAPI(options);
  {
    std::shared_ptr<logger> l = std::make_shared<logger>("Scanner");

    Aws::Client::ClientConfiguration config;
#ifdef DEBUG
    config.region = AWS_REGION;
#endif

    try {
      auto stage = get_stage(l);
      auto connection_string = get_connection_string(stage, config);

      auto repo = std::make_shared<repository::client>(connection_string, l);

      std::shared_ptr<TextractClient> textract_client =
          Aws::MakeShared<TextractClient>("textract_client", config);

      std::shared_ptr<BedrockRuntimeClient> bedrock_client =
          Aws::MakeShared<BedrockRuntimeClient>("bedrock_client", config);

      handler h(repo, textract_client, bedrock_client, l);
#ifdef DEBUG
      aws_lambda_cpp::runtime::run_debug(h);
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

