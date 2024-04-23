#include "receipt-recognition-service.hpp"

#include <cstdlib>
#include <ctime>
#include <exception>
#include <iomanip>
#include <limits>
#include <regex>
#include <sstream>
#include <string>

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
: m_textract_client(textract_client), m_logger(logger), m_db_connection(db_connection) {  }

std::vector<std::string> date_formats= {
  // YMD
  "%Y-%m-%d",
  "%y-%m-%d",
  "%Y/%m/%d",
  "%y/%m/%d",
  "%Y.%m.%d",
  "%y.%m.%d",
  "%Y %m %d",
  "%y %m %d",

  // DMY
  "%d-%m-%Y",
  "%d-%m-%y",
  "%d/%m/%Y",
  "%d/%m/%y",
  "%d.%m.%Y",
  "%d.%m.%y",
  "%d %m %Y",
  "%d %m %y",

  //MDY
  "%m-%d-%Y",
  "%m-%d-%y",
  "%m/%d/%Y",
  "%m/%d/%y",
  "%m.%d.%Y",
  "%m.%d.%y",
  "%m %d %Y",
  "%m %d %y"
};

bool receipt_recognition_service::try_parse_date(std::string& result, const std::string& text) {
  bool parsed = false;
  std::time_t now = std::time(nullptr);
  double best_diff = std::numeric_limits<double>::max();

  for (int i = 0; i < date_formats.size(); i++) {
    std::string format = date_formats[i];
    std::tm datetime = {};
    std::istringstream ss(text);
    ss >> std::get_time(&datetime, format.c_str());
    if (ss.fail()) {
      continue;
    }
    // sanityzing any short dates parsed incorrectly
    if (datetime.tm_year < 69) {
      datetime.tm_year += 100;
    }
    
    std::ostringstream check_ss;
    std::tm check_tm(datetime);
    check_ss << std::put_time(&check_tm, format.c_str());
    std::string check = check_ss.str();
    if (check != text) {
      continue;
    }

    // We assume that the correct format would produce a date
    // most close to the current date.
    std::time_t t = std::mktime(&datetime);
    double diff = std::abs(std::difftime(now, t));
    if (diff < best_diff) {
      best_diff = diff;
      std::ostringstream result_ss;
      result_ss << std::put_time(&datetime, "%Y-%m-%d");
      result = result_ss.str();
      parsed = true;
    }
  }

  return parsed;
}

std::map<std::string, std::string> currency_to_locale_map = {
  { "USD", "en_US.UTF-8" },
  { "EUR", "fr_FR" }
};

bool receipt_recognition_service::try_parse_total(long double& result, const std::string& text, const std::string& currency) {
  m_logger->info("Starting to parse total string %s. Provided currency: %s.", text.c_str(), currency.c_str());

  if (text.length() == 0) {
    m_logger->info("Invalid total string.");
    result = 0;
    return false;
  }

  std::string locale_str;
  if (currency.length() == 0
      || currency_to_locale_map.find(currency) == currency_to_locale_map.end()) {
    //try {
      locale_str = currency_to_locale_map["EUR"];
      //std::string::size_type sz = 0;
      //long double total = std::stold(text, &sz);
      //result = total;
      //m_logger->info("Successfully parsed total value %f.", result);
      //return true;
    //} catch (std::exception& e) {
    //  m_logger->info("Unable to parse total value: %s.", e.what());
    //  result = 0;
    //  return false;
    //}
  } else {
    locale_str = currency_to_locale_map[currency];
    
  }
  std::istringstream ss(text.c_str());
  //std::locale intl(locale_str.c_str());
  //ss.imbue(intl);
  //m_logger->info("Imbued locale %s", intl.name().c_str());
  long double total;
  ss >> std::get_money(total, true);
  if (ss.fail()) {
    m_logger->info("Failed parsing total value: %s", ss.str().c_str());
    result = 0;
    return false;
  }
  result = total / 100;
  m_logger->info("Successfully parsed total value %f.", result);
  return true;   
}

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
        long double total = 0;
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

          if (field_type == "INVOICE_RECEIPT_DATE" && best_date_confidence < confidence) {
            if (try_parse_date(date, summary_field.GetValueDetection().GetText())) {
              best_date_confidence = confidence;
            } else {
              m_logger->info("Unable to parse found receipt date");
              date = "";
            }
          }

          if ((field_type == "AMOUNT_PAID" || field_type == "TOTAL")
              && best_total_confidence < confidence) {
            std::string currency = summary_field.GetCurrency().GetCode();
            if (try_parse_total(total, summary_field.GetValueDetection().GetText(), currency)) {
              best_total_confidence = confidence;
            } else {
              m_logger->info("Unable to parse found total");
              total = 0;
            }
          }
        }

        // todo: parse items
        
        if (best_date_confidence == 0) {
          m_logger->info("No date found on the receipt.");
        }
        
        try {
          stmnt->setString(1, receipt_id);
          stmnt->setString(2, user_id);
          stmnt->setDateTime(3, date);
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

