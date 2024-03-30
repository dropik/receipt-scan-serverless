#include <aws/textract/model/ExpenseDocument.h>
#include <aws/textract/model/ExpenseField.h>
#include <memory>
#include <regex>

#include <aws/lambda-runtime/runtime.h>

#include <aws/core/Aws.h>
#include <aws/core/client/ClientConfiguration.h>
#include <aws/core/http/HttpTypes.h>
#include <aws/core/utils/json/JsonSerializer.h>
#include <aws/core/utils/logging/ConsoleLogSystem.h>
#include <aws/core/utils/memory/stl/AWSAllocator.h>
#include <aws/core/utils/memory/stl/AWSString.h>
#include <aws/core/utils/memory/stl/AWSVector.h>
#include <aws/s3/S3Client.h>
#include <aws/ssm/SSMClient.h>
#include <aws/ssm/SSMServiceClientModel.h>
#include <aws/ssm/model/GetParameterRequest.h>
#include <aws/textract/TextractClient.h>
#include <aws/textract/model/AnalyzeExpenseRequest.h>
#include <aws/textract/model/S3Object.h>
#include <aws/textract/model/DocumentMetadata.h>

#include "conncpp/Connection.hpp"
#include "conncpp/DriverManager.hpp"
#include "conncpp/Exception.hpp"
#include "conncpp/ResultSet.hpp"
#include "conncpp/SQLString.hpp"
#include "conncpp/Statement.hpp"
#include "conncpp/PreparedStatement.hpp"

#include <aws-lambda-cpp/common/macros.h>
#include <aws-lambda-cpp/common/logger.hpp>
#include <aws-lambda-cpp/common/string_utils.hpp>
#include <aws-lambda-cpp/models/lambda_payloads/s3.hpp>

#include "config.h"

std::string connection_string;

using namespace aws::lambda_runtime;
using namespace Aws::Utils::Logging;
using namespace Aws::Utils::Json;
using namespace Aws::Textract;
using namespace aws_lambda_cpp;
using namespace aws_lambda_cpp::common;

class receipt_recognition_service {
  public:
    receipt_recognition_service(
        const std::shared_ptr<const TextractClient>& textract_client,
        const std::shared_ptr<const logger>& logger,
        const std::shared_ptr<sql::Connection>& db_connection)
      : m_textract_client(textract_client), m_logger(logger), m_db_connection(db_connection)
    {  }

    invocation_response handle_request(invocation_request const& request) {
      m_logger->info("Version %s", VERSION);
     
      try {
        std::shared_ptr<sql::PreparedStatement> stmnt(m_db_connection->prepareStatement(
          "insert into receipts (id, user_id, date, total_amount, store_name) values (?, ?, ?, ?, ?)"));

        models::lambda_payloads::s3_request s3_request =
          json::deserialize<models::lambda_payloads::s3_request>(request.payload);

        for (int i = 0; i < s3_request.records.size(); i++) {
          auto record = s3_request.records[i];
          std::string key = record.s3.object.key;

          if (!record.is_put()) {
            m_logger->info("Skipping not put request.");
            continue;
          }
        
          std::regex file_regex("users/" GUID_REGEX "/receipts/" GUID_REGEX, std::regex_constants::extended);
          if (!std::regex_match(key, file_regex)) {
            m_logger->error("The key %s does not conform receipt file path structure.", key.c_str());
            continue;
          }

          std::string key_parted = key.substr(6, key.length() - 6);
          int users_delimeter = key_parted.find("/");
          std::string user_id = key_parted.substr(0, users_delimeter);
          key_parted.erase(0, users_delimeter + 10);
          std::string receipt_id = key_parted;

          m_logger->info("Processing receipt %s of user %s", receipt_id.c_str(), user_id.c_str());

          Model::S3Object s3_object;
          s3_object
            .WithBucket(record.s3.bucket.name)
            .WithName(key);
          Model::Document s3_document;
          s3_document.WithS3Object(s3_object);
          Model::AnalyzeExpenseRequest expense_request;
          expense_request.WithDocument(s3_document);

          auto outcome = m_textract_client->AnalyzeExpense(expense_request);
          if (!outcome.IsSuccess()) {
            m_logger->error("Error occured while analyzing expense: %s", outcome.GetError().GetMessage().c_str());
            continue;
          }

          auto expense_result = outcome.GetResult();
        
          std::vector<Model::ExpenseDocument> expense_documents = expense_result.GetExpenseDocuments();
          for (int j = 0; j < expense_documents.size(); j++) {
            Model::ExpenseDocument doc = expense_documents[j];
            std::vector<Model::ExpenseField> summary_fields = doc.GetSummaryFields();
          
            std::string store_name;
            double best_store_name_confidence = 0;
            std::string date;
            double best_date_confidence = 0;
            double total = 0;
            double best_total_confidence = 0;

            for (int k = 0; k < summary_fields.size(); k++) {
              auto summary_field = summary_fields[k];
              std::string field_type = summary_field.GetType().GetText();
              double confidence = summary_field.GetType().GetConfidence();
              if ((field_type == "NAME" || field_type == "VENDOR_NAME")
                  && best_store_name_confidence < confidence) {
                store_name = summary_field.GetValueDetection().GetText();
                replace_all(store_name, "\n", " ");
                replace_all(store_name, "\r", " ");
                replace_all(store_name, "\\", " ");
                replace_all(store_name, "/", " ");
                replace_all(store_name, "<", " ");
                replace_all(store_name, ">", " ");
                std::regex space_regex("\\s{2,}", std::regex_constants::extended);
                store_name = std::regex_replace(store_name, space_regex, " ");
                best_store_name_confidence = confidence;
              }

              // todo: parse other fields
            }

            // todo: parse items
          
            try {
              stmnt->setString(1, receipt_id);
              stmnt->setString(2, user_id);
              stmnt->setDateTime(3, "2024-03-30");
              stmnt->setDouble(4, 0);
              stmnt->setString(5, store_name);
              stmnt->executeUpdate();
            } catch (sql::SQLException& e) {
              m_logger->error("Error occured while storing receipt in database: %s", e.what());
            }
          }
        }

        return invocation_response::success("All files processed!", "application/json");

      } catch (sql::SQLException& e) {
        m_logger->error("Error occured while preparing insert receipt statement: %s", e.what());
        return invocation_response::failure("Internal error occured.", "application/json");
      }
    }

  private:
    std::shared_ptr<const logger> m_logger;
    std::shared_ptr<const TextractClient> m_textract_client;
    std::shared_ptr<sql::Connection> m_db_connection;
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
  std::shared_ptr<sql::Connection> db_connection;

  InitAPI(options);
  {
    std::shared_ptr<logger> l = std::make_shared<logger>("receipt-recognition-service");
    Aws::Client::ClientConfiguration config;
    config.region = AWS_REGION;

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

      try {
        l->info("Establishing connection with the database...");
      
        sql::SQLString url(connection_string);
        std::unique_ptr<sql::Connection> conn(sql::DriverManager::getConnection(url));
        if (conn == nullptr) {
          l->error("Unable to establish connection with database!");
          return -1;
        }
        db_connection = std::move(conn);
      
      //  std::unique_ptr<sql::Statement> stmnt(conn->createStatement());
      //  sql::ResultSet* res = stmnt->executeQuery("select count(1) from users u");
      //  res->next();
      //  _logger->info("Found users: %d", res->getInt(1));
      //

        std::shared_ptr<TextractClient> textractClient = Aws::MakeShared<TextractClient>("textract_client", config);

        receipt_recognition_service handler = receipt_recognition_service(textractClient, l, db_connection);
        run_handler([&](const invocation_request& req) {
          return handler.handle_request(req);
        });
      } catch (sql::SQLException& e) {
        l->error("Error occured while establishing connection to the database: %s", e.what());
      }
  }

  if (db_connection) {
    db_connection->close();
  }
  ShutdownAPI(options);

  return 0;
}

