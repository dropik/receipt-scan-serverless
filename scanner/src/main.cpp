#include <memory>

#include <aws/lambda-runtime/runtime.h>

#include <aws/core/Aws.h>
#include <aws/core/client/ClientConfiguration.h>
#include <aws/core/utils/logging/ConsoleLogSystem.h>
#include <aws/core/utils/memory/stl/AWSAllocator.h>
#include <aws/textract/TextractClient.h>
#include <aws/bedrock-runtime/BedrockRuntimeClient.h>

#include "conncpp/Connection.hpp"
#include "conncpp/DriverManager.hpp"
#include "conncpp/Exception.hpp"
#include "conncpp/SQLString.hpp"

#include <aws-lambda-cpp/common/logger.hpp>
#include <aws-lambda-cpp/common/string_utils.hpp>

#include <repository/repository.hpp>

#ifdef DEBUG
#include <aws-lambda-cpp/common/runtime.hpp>
#include <config.h>
#else
#include <aws/ssm/SSMClient.h>
#include <aws/ssm/model/GetParameterRequest.h>
#endif

#include "handler.hpp"

std::string connection_string;

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
    connection_string = getenv("DB_CONNECTION_STRING");
#else

    std::string function_name = getenv("AWS_LAMBDA_FUNCTION_NAME");
    l->info("Executing function %s", function_name.c_str());

    auto envStartPos = function_name.find_last_of('-');
    std::string stage = function_name.substr(
        envStartPos + 1, function_name.size() - envStartPos - 1);
    l->info("Running on stage %s", stage.c_str());

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
    connection_string = outcome.GetResult().GetParameter().GetValue();
#endif

    try {
      l->info("Establishing connection with the database...");

      sql::SQLString url(connection_string);
      std::unique_ptr<sql::Connection> conn(
          sql::DriverManager::getConnection(url));
      if (conn == nullptr) {
        l->error("Unable to establish connection with database!");
        return -1;
      }
      db_connection = std::move(conn);

      std::shared_ptr<repository::repository> repo =
          std::make_shared<repository::repository>(connection_string, l);

      std::shared_ptr<TextractClient> textract_client =
          Aws::MakeShared<TextractClient>("textract_client", config);

      std::shared_ptr<BedrockRuntimeClient> bedrock_client =
          Aws::MakeShared<BedrockRuntimeClient>("bedrock_client", config);

      auto handler = std::make_unique<scanner::handler>(repo, textract_client,
                                                        bedrock_client, l);
      auto handler_f = [&](const invocation_request& req) {
        return handler->handle_request(req);
      };

#ifdef DEBUG
      aws_lambda_cpp::runtime::run_debug(handler_f);
#else
      run_handler(handler_f);
#endif  // DEBUG

    } catch (sql::SQLException& e) {
      l->error(
          "Error occurred while establishing connection to the database: %s",
          e.what());
    }
  }

  if (db_connection) {
    db_connection->close();
  }
  ShutdownAPI(options);

  return 0;
}

