#include "handler.hpp"

#include <cstdlib>
#include <ctime>
#include <exception>
#include <iomanip>
#include <iostream>
#include <limits>
#include <regex>
#include <sstream>
#include <string>

#include <boost/locale.hpp>

#include <aws-lambda-cpp/common/string_utils.hpp>

#include <aws/textract/model/AnalyzeExpenseRequest.h>
#include <aws/textract/model/Document.h>

#include <aws/bedrock-runtime/model/InvokeModelRequest.h>

#include <conncpp/Exception.hpp>

#include "config.h"
#include "repository/repository.hpp"
#include "utils.hpp"

using namespace Aws::Textract;
using namespace Aws::BedrockRuntime;
using namespace aws_lambda_cpp::common;
using namespace aws_lambda_cpp::models::lambda_payloads;
using namespace aws::lambda_runtime;
using namespace aws_lambda_cpp;
using namespace scanner;

struct bedrock_payload {
  std::string prompt;
  int max_tokens_to_sample = 500;
  double temperature = 0.5;
  double top_p = 0.5;
  double top_k = 250;
  std::vector<std::string> stop_sequences;
  std::string anthropic_version = "bedrock-2023-05-31";

  JSON_BEGIN_SERIALIZER(bedrock_payload)
  JSON_PROPERTY("prompt", prompt)
  JSON_PROPERTY("max_tokens_to_sample", max_tokens_to_sample)
  JSON_PROPERTY("temperature", temperature)
  JSON_PROPERTY("top_p", top_p)
  JSON_PROPERTY("top_k", top_k)
  JSON_PROPERTY("stop_sequences", stop_sequences)
  JSON_PROPERTY("anthropic_version", anthropic_version)
  JSON_END_SERIALIZER()
};

struct bedrock_response {
  std::string completion;

  JSON_BEGIN_SERIALIZER(bedrock_response)
  JSON_PROPERTY("completion", completion)
  JSON_END_SERIALIZER()
};

handler::handler(std::shared_ptr<repository::repository> repository,
                 std::shared_ptr<const TextractClient> textract_client,
                 std::shared_ptr<const BedrockRuntimeClient> bedrock_client,
                 std::shared_ptr<const logger> logger)
    : m_repository(std::move(repository)),
      m_textract_client(std::move(textract_client)),
      m_bedrock_client(std::move(bedrock_client)),
      m_logger(std::move(logger)) {}

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

constexpr auto receipt_name = "NAME";
constexpr auto receipt_vendor_name = "VENDOR_NAME";
constexpr auto receipt_date = "INVOICE_RECEIPT_DATE";
constexpr auto receipt_amount = "AMOUNT_PAID";
constexpr auto receipt_total = "TOTAL";
constexpr auto item_desc = "ITEM";
constexpr auto item_quantity = "QUANTITY";
constexpr auto item_price = "PRICE";
constexpr auto item_unit_price = "UNIT_PRICE";

const std::string default_currency = "EUR";

static std::string parse_name(const std::string& text) {
  std::string result(text);
  replace_all(result, "\n", " ");
  replace_all(result, "\r", " ");
  replace_all(result, "\\", " ");
  replace_all(result, "/", " ");
  replace_all(result, "<", " ");
  replace_all(result, ">", " ");
  std::regex space_regex("\\s{2,}", std::regex_constants::extended);
  result = std::regex_replace(result, space_regex, " ");
  return result;
}

static void ltrim(std::string& s) {
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
            return !std::isspace(ch);
          }));
}

invocation_response handler::handle_request(invocation_request const& request) {
  m_logger->info("Version %s", VERSION);

  try {
    aws_lambda_cpp::models::lambda_payloads::s3_request s3_request =
        json::deserialize<aws_lambda_cpp::models::lambda_payloads::s3_request>(
            request.payload);

    for (auto &record : s3_request.records) {
      process_s3_object(record);
    }
    m_logger->info("All files processed.");

    return invocation_response::success("All files processed!",
                                        "application/json");

  } catch (const std::exception &e) {
    m_logger->error(
        "Error occurred while processing invocation request: %s", e.what());
    return invocation_response::failure("Internal error occurred.",
                                        "application/json");
  }
}

void handler::process_s3_object(s3_record& record) {
  const auto &key = record.s3.object.key;

  if (!record.is_put()) {
    m_logger->info("Skipping not put request.");
    return;
  }

  std::regex file_regex("users/" GUID_REGEX "/receipts/" GUID_REGEX,
                        std::regex_constants::extended);
  if (!std::regex_match(key, file_regex)) {
    m_logger->error("The key %s does not conform receipt file path structure.",
                    key.c_str());
    return;
  }

  std::string key_parted = key.substr(6, key.length() - 6);
  auto users_delimiter = key_parted.find('/');
  models::guid user_id = key_parted.substr(0, users_delimiter);
  key_parted.erase(0, users_delimiter + 10);
  models::guid request_id = key_parted;

  m_logger->info("Processing request %s of user %s", request_id.c_str(),
                 user_id.c_str());

  Aws::Textract::Model::S3Object s3_object;
  s3_object.WithBucket(record.s3.bucket.name).WithName(key);
  Aws::Textract::Model::Document s3_document;
  s3_document.WithS3Object(s3_object);
  Aws::Textract::Model::AnalyzeExpenseRequest expense_request;
  expense_request.WithDocument(s3_document);

  auto outcome = m_textract_client->AnalyzeExpense(expense_request);
  if (!outcome.IsSuccess()) {
    m_logger->error("Error occurred while analyzing expense: %s",
                    outcome.GetError().GetMessage().c_str());
    return;
  }

  auto &expense_result = outcome.GetResult();

  auto &expense_documents = expense_result.GetExpenseDocuments();
  int extracted_documents = 0;
  for (auto &doc : expense_documents) {
    if (try_parse_document(doc, user_id, request_id)) {
      extracted_documents++;
    }
  }
  m_logger->info("Successfully extracted %d of %d documents from the request.",
                 extracted_documents,
                 expense_documents.size());
}

bool handler::try_parse_document(
    const Aws::Textract::Model::ExpenseDocument& document,
    const models::guid& user_id, const models::guid& request_id) {
  models::receipt receipt;
  receipt.id = utils::gen_uuid();
  receipt.user_id = user_id;
  receipt.request_id = request_id;
  receipt.doc_number = document.GetExpenseIndex();

  auto& summary_fields = document.GetSummaryFields();
  if (!try_parse_summary_fields(summary_fields, receipt)) {
    return false;
  }

  auto& item_groups = document.GetLineItemGroups();
  std::vector<models::receipt_item> receipt_items;
  try_parse_items(item_groups, receipt.id, receipt_items);

  try_assign_categories(receipt, receipt_items);

  return true;
}

bool handler::try_parse_summary_fields(const expense_fields_t& summary_fields,
                                       models::receipt& receipt) {
  double best_store_name_confidence = 0;
  double best_date_confidence = 0;
  double best_total_confidence = 0;

  for (const auto & summary_field : summary_fields) {
    std::string field_type = summary_field.GetType().GetText();
    double confidence = summary_field.GetType().GetConfidence();
    std::string value = summary_field.GetValueDetection().GetText();

    if ((field_type == receipt_name || field_type == receipt_vendor_name) &&
        best_store_name_confidence < confidence) {
      receipt.store_name = parse_name(value);
      best_store_name_confidence = confidence;
    } else if (field_type == receipt_date &&
               best_date_confidence < confidence) {
      std::string found_date;
      if (try_parse_date(found_date, value)) {
        best_date_confidence = confidence;
        receipt.date = found_date;
      } else {
        m_logger->info("Unable to parse found receipt date string %s.",
                       value.c_str());
        receipt.date = "";
      }
    } else if ((field_type == receipt_amount || field_type == receipt_total) &&
               best_total_confidence < confidence) {
      receipt.currency = try_get_currency(summary_field);
      long double found_total = 0;
      if (try_parse_total(found_total, value)) {
        best_total_confidence = confidence;
        receipt.total_amount = found_total;
      } else {
        m_logger->info("Unable to parse found total string %s.", value.c_str());
        receipt.total_amount = 0;
      }
    }
  }

  if (best_store_name_confidence == 0) {
    m_logger->info("No store name found on the receipt.");
  }
  if (best_date_confidence == 0) {
    m_logger->info("No date found on the receipt.");
  }
  if (best_total_confidence == 0) {
    m_logger->info("No total found on the receipt.");
  }

  try {
    auto existing_receipt =
        m_repository
            ->select<models::receipt>(
                "select * from receipts r "
                "where r.request_id = ? and r.doc_number = ?")
            .with_param(receipt.request_id)
            .with_param(receipt.doc_number)
            .first_or_default();

    if (!existing_receipt) {
      m_repository->create(receipt);
    } else {
      receipt.id = existing_receipt->id;
      m_repository->update(receipt);
    }

  } catch (sql::SQLException& e) {
    m_logger->error("Error occurred while storing receipt in database: %s",
                    e.what());
    return false;
  }

  return true;
}

void handler::try_parse_items(
    const line_item_groups_t& line_item_groups, const models::guid& receipt_id,
    std::vector<models::receipt_item>& receipt_items) {
  int sort_order = 0;
  for (auto& group : line_item_groups) {
    auto& items = group.GetLineItems();
    for (auto& item : items) {
      models::receipt_item receipt_item;
      receipt_item.id = utils::gen_uuid();
      receipt_item.receipt_id = receipt_id;
      receipt_item.sort_order = sort_order;

      if (try_parse_item(item, receipt_item)) {
        receipt_items.push_back(receipt_item);
      }
      sort_order++;
    }
  }
}

bool handler::try_parse_item(const Aws::Textract::Model::LineItemFields& item,
                             models::receipt_item& receipt_item) {
  auto& fields = item.GetLineItemExpenseFields();

  int quantity = 1;
  long double unit_price = 0;

  double best_description_confidence = 0;
  double best_amount_confidence = 0;
  double best_quantity_confidence = 0;
  double best_unit_price_confidence = 0;

  for (auto& field : fields) {
    std::string field_type = field.GetType().GetText();
    double confidence = field.GetType().GetConfidence();
    std::string value = field.GetValueDetection().GetText();

    if (field_type == item_desc && best_description_confidence < confidence) {
      receipt_item.description = parse_name(value);
      best_description_confidence = confidence;
    } else if (field_type == item_price &&
               best_amount_confidence < confidence) {
      long double found_amount = 0;
      if (try_parse_total(found_amount, value)) {
        best_amount_confidence = confidence;
        receipt_item.amount = found_amount;
      } else {
        m_logger->info("Unable to parse found amount string %s.",
                       value.c_str());
        receipt_item.amount = 0;
      }
    } else if (field_type == item_quantity &&
               best_quantity_confidence < confidence) {
      int found_quantity = std::stoi(value);
      if (found_quantity > 0) {
        best_quantity_confidence = confidence;
        quantity = found_quantity;
      } else {
        m_logger->info("Unable to parse found quantity string %s.",
                       value.c_str());
        quantity = 1;
      }
    } else if (field_type == item_unit_price &&
               best_unit_price_confidence < confidence) {
      long double found_unit_price = 0;
      if (try_parse_total(found_unit_price, value)) {
        best_unit_price_confidence = confidence;
        unit_price = found_unit_price;
      } else {
        m_logger->info("Unable to parse found unit price string %s.",
                       value.c_str());
        unit_price = 0;
      }
    }
  }

  if (best_description_confidence == 0) {
    m_logger->info("No description found for item %d.",
                   receipt_item.sort_order);
  }
  if (best_amount_confidence == 0 &&
      (best_quantity_confidence == 0 || best_unit_price_confidence == 0)) {
    m_logger->info("No amount found for item %s.",
                   receipt_item.description.c_str());
  }

  if (receipt_item.amount == 0) {
    receipt_item.amount = quantity * unit_price;
  }

  try {
    auto existing_item =
        m_repository
            ->select<models::receipt_item>(
                "select * from receipt_items ri "
                "where ri.receipt_id = ? and ri.sort_order = ?")
            .with_param(receipt_item.receipt_id)
            .with_param(receipt_item.sort_order)
            .first_or_default();

    if (!existing_item) {
      m_repository->create(receipt_item);
    } else {
      receipt_item.id = existing_item->id;
      m_repository->update(receipt_item);
    }
    return true;
  } catch (sql::SQLException& e) {
    m_logger->error("Error occurred while storing receipt item in database: %s",
                    e.what());
    return false;
  }
}

void handler::try_assign_categories(models::receipt& receipt,
                                    std::vector<models::receipt_item>& items) {
  // Loading categories
  const auto categories =
      m_repository
          ->select<models::category>(
              "select * from categories c where c.user_id = ? order by c.name")
          .with_param(receipt.user_id)
          .all();

  std::ostringstream categories_ss;
  for (auto& category : *categories) {
    categories_ss << category->name << ", ";
  }
  categories_ss << "Altro";  // hard coding special 'other' category
  const auto categories_str = categories_ss.str();

  // Preparing prompt
  Aws::BedrockRuntime::Model::InvokeModelRequest invoke_request;
  invoke_request.WithModelId("anthropic.claude-instant-v1");
  invoke_request.SetContentType("application/json");
  bedrock_payload payload;

  if (!items.empty()) {
    std::string prompt_start_format =
        "\n\nHuman: For each receipt item guess and print a category (only) "
        "using following categories: %s.\nReceipt: %s %.2Lf %s.\nItems:";

    payload.prompt = aws_lambda_cpp::common::str_format(
        prompt_start_format, categories_str.c_str(), receipt.store_name.c_str(),
        receipt.total_amount, receipt.currency.c_str());

    std::string prompt_item_format = "\n%d. %s %.2Lf %s";

    for (auto& item : items) {
      payload.prompt += aws_lambda_cpp::common::str_format(
          prompt_item_format, item.sort_order, item.description.c_str(),
          item.amount, receipt.currency.c_str());
    }
  } else {
    std::string prompt_format =
        "\n\nHuman: Guess and print category (only) of receipt using following "
        "categories: %s.\nReceipt: %s %.2Lf %s.";

    payload.prompt = aws_lambda_cpp::common::str_format(
        prompt_format, categories_str.c_str(), receipt.store_name.c_str(),
        receipt.total_amount, receipt.currency.c_str());
  }

  payload.prompt += "\n\nAssistant:";

  // Invoking bedrock model
  std::string payload_str = json::serialize(payload);
  auto ss = std::make_shared<std::stringstream>();
  *ss << payload_str;
  invoke_request.SetBody(ss);
  const auto& outcome = m_bedrock_client->InvokeModel(invoke_request);
  if (!outcome.IsSuccess()) {
    m_logger->error("Error occurred while invoking bedrock model: %s",
                    outcome.GetError().GetMessage().c_str());
    return;
  }
  const auto& result = outcome.GetResult();
  auto& body = result.GetBody();
  
  // Parsing categories
  std::stringstream response_ss;
  std::string line;
  while (std::getline(body, line)) {
    response_ss << line;
  }
  std::string response_str = response_ss.str();
  bedrock_response response = json::deserialize<bedrock_response>(response_str);
  
  size_t start = 0;
  if (!items.empty()) {
    size_t end;
    for (auto & item : items) {
      end = response.completion.find('\n', start);
      auto category = response.completion.substr(start, end - start);
      ltrim(category);
      item.category = category;
      start = end + 1;
      if (end == std::string::npos) {
        break;
      }
    }
  } else {
    auto category = response.completion.substr(start);
    ltrim(category);
    receipt.category = category;
  }

  // Storing categories
  try {
    m_repository->update(receipt);
    for (auto& item : items) {
      m_repository->update(item);
    }
  } catch (std::exception& e) {
    m_logger->error("Error occurred while storing receipt in database: %s",
                    e.what());
  }
}

bool handler::try_parse_date(std::string& result, const std::string& input) {
  bool parsed = false;
  std::time_t now = std::time(nullptr);
  double best_diff = std::numeric_limits<double>::max();

  for (const auto& format : date_formats) {
    std::tm datetime = {};
    std::istringstream ss(input);
    ss >> std::get_time(&datetime, format.c_str());
    if (ss.fail()) {
      continue;
    }
    // sanitizing any short dates parsed incorrectly
    if (datetime.tm_year < 69) {
      datetime.tm_year += 100;
    }

    std::ostringstream check_ss;
    std::tm check_tm(datetime);
    check_ss << std::put_time(&check_tm, format.c_str());
    std::string check = check_ss.str();
    if (check != input) {
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

bool handler::try_parse_total(long double& result, const std::string& input) const {
  // Since deducing a locale might not be reliable, because
  // the currency might not be present in the text or decimal
  // separator might be different, we will try to remove all
  // non-numeric characters and parse the number as a long double
  // and then divide it by 100 to get the correct value.

  std::string text(input);
  replace_all(text, ",", "");
  replace_all(text, ".", "");

  std::string::size_type start = 0;
  for (; start < text.length(); start++) {
    if (std::isdigit(text[start])) {
      break;
    }
  }
  std::string::size_type end = start;
  for (; end < text.length(); end++) {
    if (!std::isdigit(text[end])) {
      break;
    }
  }
  std::string numeric = text.substr(start, end - start);

  if (numeric.empty()) {
    m_logger->info("No numeric characters found in the total field.");
    result = 0;
    return false;
  }

  std::istringstream ss(numeric);
  long double total;
  ss >> boost::locale::as::currency >> total;
  if (ss.fail()) {
    m_logger->info("Unable to parse total as currency.");
    result = 0;
    return false;
  }
  result = total / 100.0;
  return true;
}

const std::string & handler::try_get_currency(const Aws::Textract::Model::ExpenseField &field) const {
  const auto& currency = field.GetCurrency().GetCode();
  if (!currency.empty()) {
    return currency;
  } else {
    m_logger->info("No currency found for receipt. Assuming EUR.");
    return default_currency;
  }
}
