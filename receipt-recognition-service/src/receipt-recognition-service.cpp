#include "receipt-recognition-service.hpp"

#include <regex>

#include <aws-lambda-cpp/models/lambda_payloads/s3.hpp>
#include <aws-lambda-cpp/common/string_utils.hpp>

#include <aws/textract/model/Document.h>
#include <aws/textract/model/AnalyzeExpenseRequest.h>

#include <conncpp/PreparedStatement.hpp>
#include <conncpp/Exception.hpp>

#include "config.h"

using namespace Aws::Textract;
using namespace aws_lambda_cpp::common;
using namespace aws::lambda_runtime;
using namespace aws_lambda_cpp;

receipt_recognition_service::receipt_recognition_service(
  const std::shared_ptr<const TextractClient>& textract_client,
  const std::shared_ptr<const logger>& logger,
        const std::shared_ptr<sql::Connection>& db_connection)
      : m_textract_client(textract_client), m_logger(logger), m_db_connection(db_connection)
    {  }

invocation_response receipt_recognition_service::handle_request(
    invocation_request const& request) {
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

