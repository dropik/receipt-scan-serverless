#include <aws-lambda-cpp/common/json.hpp>
#include <aws-lambda-cpp/common/macros.h>
#include <aws/core/utils/memory/stl/AWSAllocator.h>
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
#include <aws/textract/TextractClient.h>

#include "conncpp/Connection.hpp"
#include "conncpp/DriverManager.hpp"
#include "conncpp/Exception.hpp"
#include "conncpp/ResultSet.hpp"
#include "conncpp/SQLString.hpp"
#include "conncpp/Statement.hpp"

#include <aws-lambda-cpp/common/logger.hpp>
#include <aws-lambda-cpp/common/string_utils.hpp>
#include <aws-lambda-cpp/models/lambda_payloads/s3.hpp>
#include <memory>

#include "config.h"

std::string connection_string;

using namespace aws::lambda_runtime;
using namespace Aws::Utils::Logging;
using namespace Aws::Utils::Json;
using namespace aws_lambda_cpp::common;

class receipt_recognition_service {
  public:
    receipt_recognition_service(
        const std::shared_ptr<const Aws::Textract::TextractClient>& textractClient,
        const std::shared_ptr<const logger>& logger) {
      this->_logger = logger;
      this->_textractClient = textractClient;
    }

    invocation_response handle_request(invocation_request const& request) {
      _logger->info("Version %s", VERSION);

      aws_lambda_cpp::models::lambda_payloads::s3_request s3_request =
        aws_lambda_cpp::json::deserialize<aws_lambda_cpp::models::lambda_payloads::s3_request>(request.payload);

      std::string payload_json = aws_lambda_cpp::json::serialize(s3_request);
      _logger->info("Obtained s3 request: %s", payload_json.c_str());

      try {
        _logger->info("Establishing connection with the database...");
    
        sql::SQLString url(connection_string);
        std::unique_ptr<sql::Connection> conn(sql::DriverManager::getConnection(url));
        if (conn == nullptr) {
          _logger->error("Unable to establish connection with database!");
          return invocation_response::failure("{\"statusCode\": 500}", "application/json");
        }

        _logger->info("Successfully connected to datbase!");
        _logger->info("Reading from database!");

        std::unique_ptr<sql::Statement> stmnt(conn->createStatement());
        sql::ResultSet* res = stmnt->executeQuery("select count(1) from users u");
        res->next();
        _logger->info("Found users: %d", res->getInt(1));
    
        conn->close();
      } catch (sql::SQLException& e) {
        _logger->error("Error occured while doing operation on database: %s", e.what());
      }

      return invocation_response::success("{\"statusCode\": 200}", "application/json");
    }

  private:
    std::shared_ptr<const logger> _logger;
    std::shared_ptr<const Aws::Textract::TextractClient> _textractClient;
};

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
    std::shared_ptr<logger> l = std::make_shared<logger>("receipt-recognition-service");
    Aws::Client::ClientConfiguration config;

#ifdef DEBUG
      connection_string = getenv("DB_CONNECTION_STRING");
 #else
      std::string functionName = getenv("AWS_LAMBDA_FUNCTION_NAME");
      l->info("Executing function %s", functionName.c_str());

      int envStartPos = functionName.find_last_of('-');
      std::string stage = functionName.substr(envStartPos + 1, functionName.size() - envStartPos - 1);
      l->info("Running on stage %s", stage.c_str());

      std::string ssmPrefix = str_format("/receipt-scan/%s", stage.c_str());
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

      std::shared_ptr<Aws::Textract::TextractClient> textractClient = Aws::MakeShared<Aws::Textract::TextractClient>("textract_client", config);

      receipt_recognition_service handler = receipt_recognition_service(textractClient, l);
      run_handler([&](const invocation_request& req) {
        return handler.handle_request(req);
      });
  }
  ShutdownAPI(options);

  return 0;
}

