//
// Created by Daniil Ryzhkov on 22/06/2024.
//

#include <gtest/gtest.h>

#include <di/container.hpp>
#include <aws/textract/TextractClient.h>
#include <aws/bedrock-runtime/BedrockRuntimeClient.h>
#include "repository/client.hpp"
#include "../src/handler.hpp"

using namespace di;
using namespace Aws::Textract;
using namespace Aws::Textract::Model;
using namespace Aws::BedrockRuntime;
using namespace scanner;
using namespace scanner::services;
using namespace repository;
using namespace repository::models;

#define DEFAULT_KEY "users/20a79fcd-1783-475b-9095-35afb0d34b7f/receipts/2024-06-22.jpg"
#define DEFAULT_EVENT "ObjectCreated:Put"

static aws::lambda_runtime::invocation_request create_request(
    const std::string &key = DEFAULT_KEY,
    const std::string &event = DEFAULT_EVENT) {
  aws::lambda_runtime::invocation_request request;
  lambda::models::payloads::s3_request s3_request;
  lambda::models::payloads::s3_record s3_record;
  lambda::models::payloads::s3_bucket s3_bucket;
  lambda::models::payloads::s3_object s3_object;
  s3_bucket.name = "bucket";
  s3_object.key = key;
  s3_record.s3.bucket = s3_bucket;
  s3_record.s3.object = s3_object;
  s3_record.event_name = event;
  s3_request.records.push_back(s3_record);
  request.payload = lambda::json::serialize(s3_request);
  return request;
}

struct fake_textract_client {
  bool should_fail = false;
  bool set_quantity = true;
  std::string invoice_date = "2024-06-22";
  std::string amount = "100.00";
  double confidence = 99;
  std::string currency;
  std::string quantity = "1";

  AnalyzeExpenseOutcome AnalyzeExpense(const AnalyzeExpenseRequest &request) {
    if (should_fail) {
      return {
          TextractError(
              Aws::Client::AWSError<Aws::Client::CoreErrors>(
                  Aws::Client::CoreErrors::INVALID_ACTION,
                  "Invalid parameter value",
                  "Invalid parameter value",
                  false))};
    }

    AnalyzeExpenseResult result;
    ExpenseDocument document;
    document.SetExpenseIndex(1);

    document.AddSummaryFields(
        ExpenseField()
            .WithType(ExpenseType().WithText("NAME").WithConfidence(confidence))
            .WithValueDetection(ExpenseDetection().WithText("Amazon")));
    document.AddSummaryFields(
        ExpenseField()
            .WithType(ExpenseType().WithText("VENDOR_NAME").WithConfidence(confidence))
            .WithValueDetection(ExpenseDetection().WithText("Amazon")));
    document.AddSummaryFields(
        ExpenseField()
            .WithType(ExpenseType().WithText("INVOICE_RECEIPT_DATE").WithConfidence(confidence))
            .WithValueDetection(ExpenseDetection().WithText(invoice_date)));
    document.AddSummaryFields(
        ExpenseField()
            .WithType(ExpenseType().WithText("AMOUNT_PAID").WithConfidence(confidence))
            .WithValueDetection(ExpenseDetection().WithText(amount))
            .WithCurrency(ExpenseCurrency().WithCode(currency)));
    document.AddSummaryFields(
        ExpenseField()
            .WithType(ExpenseType().WithText("TOTAL").WithConfidence(confidence))
            .WithValueDetection(ExpenseDetection().WithText(amount))
            .WithCurrency(ExpenseCurrency().WithCode(currency)));

    document.AddLineItemGroups(
        LineItemGroup()
            .WithLineItemGroupIndex(1)
            .WithLineItems(
                {
                    LineItemFields()
                        .WithLineItemExpenseFields(
                            {
                                ExpenseField()
                                    .WithType(ExpenseType().WithText("ITEM").WithConfidence(confidence))
                                    .WithValueDetection(ExpenseDetection().WithText("Item 1")),
                                ExpenseField()
                                    .WithType(ExpenseType().WithText(set_quantity ? "QUANTITY" : "ASDF").WithConfidence(confidence))
                                    .WithValueDetection(ExpenseDetection().WithText(quantity)),
                                ExpenseField()
                                    .WithType(ExpenseType().WithText("PRICE").WithConfidence(confidence))
                                    .WithValueDetection(ExpenseDetection().WithText(amount))
                                    .WithCurrency(ExpenseCurrency().WithCode(currency)),
                                ExpenseField()
                                    .WithType(ExpenseType().WithText("UNIT_PRICE").WithConfidence(confidence))
                                    .WithValueDetection(ExpenseDetection().WithText(amount))
                                    .WithCurrency(ExpenseCurrency().WithCode(currency)),
                            })
                })
    );
    result.SetExpenseDocuments({document});
    return {result};
  }
};

struct fake_bedrock_runtime_client {

};

struct fake_repository {
  std::vector<receipt> repo;

  lambda::nullable<receipt> get(const std::string &user_id, const std::string &file_name, int doc_number) {
    if (repo.empty()) {
      return {};
    } else {
      return repo[0];
    }
  }

  void store(const receipt &r) {
    auto it = std::find_if(repo.begin(), repo.end(), [&r](const receipt &o) {
      return r.id == o.id;
    });
    if (it != repo.end()) {
      *it = r;
    } else {
      repo.push_back(r);
    }
  }
};

class scanner_test : public ::testing::Test {
 protected:
  void SetUp() override {
  }

  void TearDown() override {
  }

  container<
      scoped<TextractClient, fake_textract_client>,
      scoped<BedrockRuntimeClient, fake_bedrock_runtime_client>,

      transient<t_receipt_extractor, receipt_extractor<>>,
      scoped<t_receipt_repository, fake_repository>,

      transient<t_handler, handler_v2<>>
  > services;
};

TEST_F(scanner_test, should_create_receipt) {
  auto handler = services.get<t_handler>();
  auto request = create_request();
  auto res = handler->operator()(request);

  EXPECT_TRUE(res.is_success());

  auto repo = services.get<repository::t_receipt_repository>();
  auto receipts = repo->repo;
  EXPECT_EQ(receipts.size(), 1);
  EXPECT_EQ(receipts[0].store_name, "Amazon");
  EXPECT_EQ(receipts[0].date, "2024-06-22");
  EXPECT_EQ(receipts[0].total_amount, 100.00);
  auto receipt = receipts[0];

  EXPECT_TRUE(receipt.file.has_value());
  EXPECT_EQ(receipt.file.get_value().receipt_id, receipts[0].id);
  EXPECT_EQ(receipt.file.get_value().file_name, "2024-06-22.jpg");
  EXPECT_EQ(receipt.file.get_value().doc_number, 1);

  auto items = receipt.items;
  EXPECT_EQ(items.size(), 1);
  EXPECT_EQ(items[0].receipt_id, receipts[0].id);
  EXPECT_EQ(items[0].description, "Item 1");
  EXPECT_EQ(items[0].amount, 100.00);
  EXPECT_EQ(items[0].sort_order, 0);
}

TEST_F(scanner_test, should_ignore_not_receipt_key_patter) {
  auto handler = services.get<t_handler>();
  auto request = create_request("users/20a79fcd-1783-475b-9095-35afb0d34b7f/whatever");
  auto res = handler->operator()(request);

  EXPECT_TRUE(res.is_success());

  auto repo = services.get<repository::t_receipt_repository>();
  auto receipts = repo->repo;
  EXPECT_EQ(receipts.size(), 0);
}

TEST_F(scanner_test, should_continue_if_textract_fails) {
  auto textract = services.get<TextractClient>();
  textract->should_fail = true;

  auto handler = services.get<t_handler>();
  auto request = create_request();
  auto res = handler->operator()(request);

  EXPECT_TRUE(res.is_success());

  auto repo = services.get<repository::t_receipt_repository>();
  auto receipts = repo->repo;
  EXPECT_EQ(receipts.size(), 0);
}

TEST_F(scanner_test, should_recognize_short_year) {
  auto textract = services.get<TextractClient>();
  textract->invoice_date = "24-06-22";

  auto handler = services.get<t_handler>();
  auto request = create_request();
  auto res = handler->operator()(request);

  EXPECT_TRUE(res.is_success());

  auto repo = services.get<repository::t_receipt_repository>();
  auto receipts = repo->repo;
  EXPECT_EQ(receipts.size(), 1);
  EXPECT_EQ(receipts[0].date, "2024-06-22");
}

TEST_F(scanner_test, should_sanitise_amount) {
  auto test = [this](const std::string &amount) {
    auto textract = services.get<TextractClient>();
    textract->amount = amount;

    auto handler = services.get<t_handler>();
    auto request = create_request();
    auto res = handler->operator()(request);

    EXPECT_TRUE(res.is_success());

    auto repo = services.get<repository::t_receipt_repository>();
    auto receipts = repo->repo;
    EXPECT_EQ(receipts.size(), 1);
    EXPECT_EQ(receipts[0].total_amount, 100.00);

    repo->repo.clear();
  };
  test("USD 100.00");
  test("100.00 USD");
}

TEST_F(scanner_test, should_ignore_amount_if_invalid) {
  auto textract = services.get<TextractClient>();
  textract->amount = "AMOUNT";

  auto handler = services.get<t_handler>();
  auto request = create_request();
  auto res = handler->operator()(request);

  EXPECT_TRUE(res.is_success());

  auto repo = services.get<repository::t_receipt_repository>();
  auto receipts = repo->repo;
  EXPECT_EQ(receipts.size(), 1);
  EXPECT_EQ(receipts[0].total_amount, 0.00);
}

TEST_F(scanner_test, should_import_currency) {
  auto textract = services.get<TextractClient>();
  textract->currency = "USD";

  auto handler = services.get<t_handler>();
  auto request = create_request();
  auto res = handler->operator()(request);

  EXPECT_TRUE(res.is_success());

  auto repo = services.get<repository::t_receipt_repository>();
  auto receipts = repo->repo;
  EXPECT_EQ(receipts.size(), 1);
  EXPECT_EQ(receipts[0].currency, "USD");
}

TEST_F(scanner_test, should_handle_incorrect_date) {
  auto textract = services.get<TextractClient>();
  textract->invoice_date = "2024-06-32";

  auto handler = services.get<t_handler>();
  auto request = create_request();
  auto res = handler->operator()(request);

  EXPECT_TRUE(res.is_success());

  auto repo = services.get<repository::t_receipt_repository>();
  auto receipts = repo->repo;
  EXPECT_EQ(receipts.size(), 1);
  EXPECT_EQ(receipts[0].date, "");
}

TEST_F(scanner_test, should_set_default_quantity) {
  auto textract = services.get<TextractClient>();
  textract->set_quantity = false;

  auto handler = services.get<t_handler>();
  auto request = create_request();
  auto res = handler->operator()(request);

  EXPECT_TRUE(res.is_success());

  auto repo = services.get<repository::t_receipt_repository>();
  auto receipts = repo->repo;
  EXPECT_EQ(receipts.size(), 1);
  auto items = receipts[0].items;
  EXPECT_EQ(items.size(), 1);
  EXPECT_EQ(items[0].amount, 100.00);
}

TEST_F(scanner_test, should_handle_incorrect_quantity) {
  auto textract = services.get<TextractClient>();
  textract->quantity = "QUANTITY";

  auto handler = services.get<t_handler>();
  auto request = create_request();
  auto res = handler->operator()(request);

  EXPECT_TRUE(res.is_success());

  auto repo = services.get<repository::t_receipt_repository>();
  auto receipts = repo->repo;
  EXPECT_EQ(receipts.size(), 1);
  auto items = receipts[0].items;
  EXPECT_EQ(items.size(), 1);
  EXPECT_EQ(items[0].amount, 100.00);
}

TEST_F(scanner_test, should_update_existing_receipt_if_present_by_user_file_and_doc_number) {
  auto handler = services.get<t_handler>();
  auto request = create_request();
  auto res = handler->operator()(request);

  EXPECT_TRUE(res.is_success());

  auto repo = services.get<repository::t_receipt_repository>();
  auto receipts = repo->repo;
  EXPECT_EQ(receipts.size(), 1);

  auto receipt = receipts[0];
  receipt.total_amount = 200.00;
  repo->store(receipt);

  auto res2 = handler->operator()(request);
  EXPECT_TRUE(res2.is_success());

  auto receipts2 = repo->repo;
  EXPECT_EQ(receipts2.size(), 1);
  EXPECT_EQ(receipts2[0].total_amount, 100.00);
}

TEST_F(scanner_test, should_ignore_non_put_s3_events) {
  auto handler = services.get<t_handler>();
  auto request = create_request(DEFAULT_KEY, "ObjectCreated:Delete");
  auto res = handler->operator()(request);

  EXPECT_TRUE(res.is_success());

  auto repo = services.get<repository::t_receipt_repository>();
  auto receipts = repo->repo;
  EXPECT_EQ(receipts.size(), 0);
}
