//
// Created by Daniil Ryzhkov on 22/06/2024.
//

#pragma once

#include <vector>

#include "repository/models/receipt.hpp"
#include <aws/textract/TextractClient.h>

namespace scanner {
namespace services {

struct t_receipt_extractor {};

template<typename TTextractClient = Aws::Textract::TextractClient>
class receipt_extractor : t_receipt_extractor {
 public:
  explicit receipt_extractor(TTextractClient textract_client) : m_textract_client(std::move(textract_client)) {}

  std::vector<repository::models::receipt> extract(const std::string &bucket, const std::string &key) const {

    std::regex file_regex("users/" GUID_REGEX "/receipts/.*",
                          std::regex_constants::extended);
    if (!std::regex_match(key, file_regex)) {
      lambda::log.info("The key %s does not conform receipt file path structure.",
                       key.c_str());
      return {};
    }

    std::string key_parted = key.substr(6, key.length() - 6);
    auto users_delimiter = key_parted.find('/');
    guid user_id = key_parted.substr(0, users_delimiter);
    key_parted.erase(0, users_delimiter + 10);
    std::string file_name = key_parted;

    lambda::log.info("Processing request %s of user %s", file_name.c_str(),
                     user_id.c_str());

    Aws::Textract::Model::S3Object s3_object;
    s3_object.WithBucket(bucket).WithName(key);
    Aws::Textract::Model::Document s3_document;
    s3_document.WithS3Object(s3_object);
    Aws::Textract::Model::AnalyzeExpenseRequest expense_request;
    expense_request.WithDocument(s3_document);

    auto outcome = m_textract_client->AnalyzeExpense(expense_request);
    if (!outcome.IsSuccess()) {
      lambda::log.error("Error occurred while analyzing expense: %s",
                        outcome.GetError().GetMessage().c_str());
      return {};
    }

    auto &expense_result = outcome.GetResult();

    auto &expense_documents = expense_result.GetExpenseDocuments();
    std::vector<receipt> results;
    for (auto &doc : expense_documents) {
      receipt receipt;
      receipt.id = utils::gen_uuid();
      receipt.user_id = user_id;
      receipt.state = receipt::processing;

      receipt.file = receipt_file{utils::gen_uuid(), receipt.id, file_name, doc.GetExpenseIndex()};

      parse_document(doc, receipt);

      results.push_back(receipt);
    }

    lambda::log.info("Successfully extracted %d of %d documents from the request.",
                     results.size(),
                     expense_documents.size());

    return results;
  }

 private:
  TTextractClient m_textract_client;

  using expense_fields = std::vector<Aws::Textract::Model::ExpenseField>;
  using line_item_groups = std::vector<Aws::Textract::Model::LineItemGroup>;
  using guid = std::string;
  using receipt = repository::models::receipt;
  using receipt_item = repository::models::receipt_item;
  using receipt_file = repository::models::receipt_file;

  static constexpr auto receipt_name = "NAME";
  static constexpr auto receipt_vendor_name = "VENDOR_NAME";
  static constexpr auto receipt_date = "INVOICE_RECEIPT_DATE";
  static constexpr auto receipt_amount = "AMOUNT_PAID";
  static constexpr auto receipt_total = "TOTAL";
  static constexpr auto item_desc = "ITEM";
  static constexpr auto item_quantity = "QUANTITY";
  static constexpr auto item_price = "PRICE";
  static constexpr auto item_unit_price = "UNIT_PRICE";
  static constexpr auto default_currency = "EUR";

  std::array<std::string, 24> date_formats = {
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

  void parse_document(const Aws::Textract::Model::ExpenseDocument &document, receipt &receipt) const {
    auto &summary_fields = document.GetSummaryFields();
    parse_summary_fields(summary_fields, receipt);

    auto &item_groups = document.GetLineItemGroups();
    std::vector<receipt_item> receipt_items;
    parse_items(item_groups, receipt);

    receipt.state = receipt::done;
  }

  static std::string parse_name(const std::string &text) {
    std::string result(text);
    lambda::string::replace_all(result, "\n", " ");
    lambda::string::replace_all(result, "\r", " ");
    lambda::string::replace_all(result, "\\", " ");
    lambda::string::replace_all(result, "/", " ");
    lambda::string::replace_all(result, "<", " ");
    lambda::string::replace_all(result, ">", " ");
    std::regex space_regex("\\s{2,}", std::regex_constants::extended);
    result = std::regex_replace(result, space_regex, " ");
    return result;
  }

  bool try_parse_date(std::string &result, const std::string &input) const {
    bool parsed = false;
    std::time_t now = std::time(nullptr);
    double best_diff = std::numeric_limits<double>::max();

    for (const auto &format : date_formats) {
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

  bool try_parse_total(long double &result, const std::string &input) const {
    // Since deducing a locale might not be reliable, because
    // the currency might not be present in the text or decimal
    // separator might be different, we will try to remove all
    // non-numeric characters and parse the number as a long double
    // and then divide it by 100 to get the correct value.

    std::string text(input);
    lambda::string::replace_all(text, ",", "");
    lambda::string::replace_all(text, ".", "");

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
      lambda::log.info("No numeric characters found in the total field.");
      result = 0;
      return false;
    }

    std::istringstream ss(numeric);
    long double total;
    ss >> std::get_money(total);
    if (ss.fail()) {
      lambda::log.info("Unable to parse total as currency.");
      result = 0;
      return false;
    }
    result = total / 100.0;
    return true;
  }

  std::string try_get_currency(const Aws::Textract::Model::ExpenseField &field) const {
    const auto &currency = field.GetCurrency().GetCode();
    if (!currency.empty()) {
      return currency;
    } else {
      lambda::log.info("No currency found for receipt. Assuming EUR.");
      return default_currency;
    }
  }

  void parse_summary_fields(const expense_fields &summary_fields, receipt &receipt) const {
    double best_store_name_confidence = 0;
    double best_date_confidence = 0;
    double best_total_confidence = 0;

    for (const auto &summary_field : summary_fields) {
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
          lambda::log.info("Unable to parse found receipt date string %s.",
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
          lambda::log.info("Unable to parse found total string %s.", value.c_str());
          receipt.total_amount = 0;
        }
      }
    }

    if (best_store_name_confidence == 0) {
      lambda::log.info("No store name found on the receipt.");
    }
    if (best_date_confidence == 0) {
      lambda::log.info("No date found on the receipt.");
    }
    if (best_total_confidence == 0) {
      lambda::log.info("No total found on the receipt.");
    }
  }

  void parse_items(const line_item_groups &line_item_groups, receipt &receipt) const {
    int sort_order = 0;
    for (auto &group : line_item_groups) {
      auto &items = group.GetLineItems();
      for (auto &item : items) {
        receipt_item receipt_item;
        receipt_item.id = utils::gen_uuid();
        receipt_item.receipt_id = receipt.id;
        receipt_item.sort_order = sort_order;

        parse_item(item, receipt_item);

        receipt.items.push_back(receipt_item);
        sort_order++;
      }
    }
  }

  void parse_item(const Aws::Textract::Model::LineItemFields &item, receipt_item &receipt_item) const {
    auto &fields = item.GetLineItemExpenseFields();

    int quantity = 1;
    long double unit_price = 0;

    double best_description_confidence = 0;
    double best_amount_confidence = 0;
    double best_quantity_confidence = 0;
    double best_unit_price_confidence = 0;

    for (auto &field : fields) {
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
          lambda::log.info("Unable to parse found amount string %s.",
                           value.c_str());
          receipt_item.amount = 0;
        }
      } else if (field_type == item_quantity &&
          best_quantity_confidence < confidence) {
        try {
          int found_quantity = std::stoi(value);
          if (found_quantity > 0) {
            best_quantity_confidence = confidence;
            quantity = found_quantity;
          } else {
            lambda::log.info("Unable to parse found quantity string %s.",
                             value.c_str());
            quantity = 1;
          }
        } catch (std::invalid_argument &e) {
          lambda::log.info("Unable to parse found quantity string %s.",
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
          lambda::log.info("Unable to parse found unit price string %s.",
                           value.c_str());
          unit_price = 0;
        }
      }
    }

    if (best_description_confidence == 0) {
      lambda::log.info("No description found for item %d.",
                       receipt_item.sort_order);
    }
    if (best_amount_confidence == 0 &&
        (best_quantity_confidence == 0 || best_unit_price_confidence == 0)) {
      lambda::log.info("No amount found for item %s.",
                       receipt_item.description.c_str());
    }

    if (receipt_item.amount == 0) {
      receipt_item.amount = quantity * unit_price;
    }
  }
};

}
}
