#include <aws/lambda-runtime/runtime.h>

#include <aws/core/Aws.h>
#include <aws/core/client/ClientConfiguration.h>
#include <aws/core/http/HttpTypes.h>
#include <aws/core/utils/json/JsonSerializer.h>
#include <aws/core/utils/logging/ConsoleLogSystem.h>
#include <aws/core/utils/memory/stl/AWSString.h>
#include <aws/core/utils/memory/stl/AWSVector.h>

#include <aws/s3/S3Client.h>
#include <aws/ssm/SSMClient.h>
#include <aws/ssm/SSMServiceClientModel.h>
#include <aws/ssm/model/GetParameterRequest.h>

#include "conncpp/Connection.hpp"
#include "conncpp/DriverManager.hpp"
#include "conncpp/Exception.hpp"
#include "conncpp/ResultSet.hpp"
#include "conncpp/SQLString.hpp"
#include "conncpp/Statement.hpp"

#include <aws-lambda-cpp/common/logger.hpp>

#include "config.h"

std::string connection_string;

using namespace aws::lambda_runtime;
using namespace Aws::Utils::Logging;
using namespace Aws::Utils::Json;
using namespace aws_lambda_cpp::common;

invocation_response lambda_handler(const logger& logger, invocation_request const& request) {
  logger.info("Version %s", VERSION);

  try {
    logger.info("Establishing connection with the database...");
    
    sql::SQLString url(connection_string);
    std::unique_ptr<sql::Connection> conn(sql::DriverManager::getConnection(url));
    if (conn == nullptr) {
      logger.error("Unable to establish connection with database!");
      return invocation_response::failure("{\"statusCode\": 500}", "application/json");
    }

    logger.info("Successfully connected to datbase!");
    logger.info("Reading from database!");

    std::unique_ptr<sql::Statement> stmnt(conn->createStatement());
    sql::ResultSet* res = stmnt->executeQuery("select count(1) from users u");
    res->next();
    logger.info("Found users: %d", res->getInt(1));
    
    conn->close();
  } catch (sql::SQLException& e) {
    logger.error("Error occured while doing operation on database: %s", e.what());
  }

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

  InitAPI(options);
  {
    logger logger("receipt-recognition-service");

#ifdef DEBUG
      connection_string = getenv("DB_CONNECTION_STRING");
 #else
      std::string functionName = getenv("AWS_LAMBDA_FUNCTION_NAME");
      logger.info("Executing function %s", functionName.c_str());

      int envStartPos = functionName.find_last_of('-');
      std::string stage = functionName.substr(envStartPos + 1, functionName.size() - envStartPos - 1);
      logger.info("Running on stage %s", stage.c_str());

      std::string ssmPrefix = str_format("/receipt-scan/%s", stage.c_str());
      Aws::Client::ClientConfiguration config;
      Aws::SSM::SSMClient ssmClient(config);
      Aws::SSM::Model::GetParameterRequest connStrReq;
      connStrReq
        .WithName(str_format("%s/db-connection-string", ssmPrefix.c_str()))
        .WithWithDecryption(true);
      Aws::SSM::Model::GetParameterOutcome outcome = ssmClient.GetParameter(connStrReq);
      if (!outcome.IsSuccess()) {
        throw std::runtime_error(str_format(
              "Error occured while obtaining parameter from ssm: %s",
              outcome.GetError().GetMessage().c_str()));
      }
      connection_string = outcome.GetResult().GetParameter().GetValue();
#endif

      run_handler([&](const invocation_request& req) {
        return lambda_handler(logger, req);
      });
  }
  ShutdownAPI(options);

  return 0;
}

