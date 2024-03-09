#include <stdexcept>

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
#include <aws/ssm/SSMClient.h>
#include <aws/ssm/SSMServiceClientModel.h>
#include <aws/ssm/model/GetParameterRequest.h>

#include "conncpp/Connection.hpp"
#include "conncpp/DriverManager.hpp"
#include "conncpp/Exception.hpp"
#include "conncpp/ResultSet.hpp"
#include "conncpp/SQLString.hpp"
#include "conncpp/Statement.hpp"

#include "config.h"

char const TAG[] = "receipt-recognition-service";
std::string connection_string;

using namespace aws::lambda_runtime;
using namespace Aws::Utils::Logging;
using namespace Aws::Utils::Json;

template<typename ... Args>
static std::string str_format(const std::string& format, Args ... args) {
  int s = std::snprintf(nullptr, 0, format.c_str(), args ...) + 1;
  if (s <= 0) {
    throw std::runtime_error("Error occured during string formatting!");
  }
  size_t size = static_cast<size_t>(s);
  std::unique_ptr<char[]> buf(new char[size]);
  std::snprintf(buf.get(), size, format.c_str(), args ...);
  return std::string(buf.get(), buf.get() + size - 1);
}

static void log_info(std::string message) {
  AWS_LOGSTREAM_INFO(TAG, message);
  AWS_LOGSTREAM_FLUSH();
}

static void log_error(std::string message) {
  AWS_LOGSTREAM_ERROR(TAG, message);
  AWS_LOGSTREAM_FLUSH();
}

static invocation_response lambda_handler(invocation_request const& request) {
  log_info(str_format("Version %s", VERSION));

  try {
    log_info("Establishing connection with the database...");
    
    sql::SQLString url(connection_string);
    std::unique_ptr<sql::Connection> conn(sql::DriverManager::getConnection(url));
    if (conn == nullptr) {
      log_error("Unable to establish connection with database!");
      return invocation_response::failure("{\"statusCode\": 500}", "application/json");
    }

    log_info("Successfully connected to datbase!");
    log_info("Reading from database!");

    std::unique_ptr<sql::Statement> stmnt(conn->createStatement());
    sql::ResultSet* res = stmnt->executeQuery("select count(1) from users u");
    res->next();
    log_info(str_format("Found users: %d", res->getInt(1)));
    
    conn->close();
  } catch (sql::SQLException& e) {
    log_error(str_format("Error occured while doing operation on database: %s", e.what()));
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
#ifdef DEBUG
      connection_string = getenv("DB_CONNECTION_STRING");
 #else
      std::string functionName = getenv("AWS_LAMBDA_FUNCTION_NAME");
      log_info(str_format("Executing function %s", functionName.c_str()));

      int envStartPos = functionName.find_last_of('-');
      std::string stage = functionName.substr(envStartPos + 1, functionName.size() - envStartPos - 1);
      log_info(str_format("Running on stage %s", stage.c_str()));

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

    run_handler(lambda_handler);
  }
  ShutdownAPI(options);

  return 0;
}

