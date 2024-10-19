//
// Created by Daniil Ryzhkov on 22/06/2024.
//

#include <gtest/gtest.h>

#include <aws/textract/TextractClient.h>
#include <aws/bedrock-runtime/BedrockRuntimeClient.h>

#include <di/container.hpp>
#include "repository/client.hpp"
#include "integration_tests_common/repository_integration_test.hpp"

#include "../src/factories.hpp"
#include "../src/handler.hpp"

using namespace di;
using namespace Aws::Textract;
using namespace Aws::Textract::Model;
using namespace Aws::BedrockRuntime;
using namespace Aws::BedrockRuntime::Model;
using namespace scanner;
using namespace scanner::services;
using namespace repository;
using namespace repository::models;

#define USER_ID "20a79fcd-1783-475b-9095-35afb0d34b7f"
#define DATE "2024-06-22"
#define IMAGE_NAME DATE ".jpg"
#define DEFAULT_KEY "users/" USER_ID "/receipts/" IMAGE_NAME
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
  bool add_items = true;
  std::string invoice_date = DATE;
  std::string amount = "100.00";
  double confidence = 99;
  std::string currency;
  std::string quantity = "1";

  [[nodiscard]] AnalyzeExpenseOutcome AnalyzeExpense(const AnalyzeExpenseRequest &request) const {
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

    if (add_items) {
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
                                      .WithType(ExpenseType().WithText(set_quantity ? "QUANTITY"
                                                                                    : "ASDF").WithConfidence(confidence))
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
    }
    result.SetExpenseDocuments({document});
    return {result};
  }
};

struct fake_bedrock_runtime_client {
  bool should_fail = false;
  std::string completion_body = R"({"completion": "Altro\n"})";

  [[nodiscard]] InvokeModelOutcome InvokeModel(const InvokeModelRequest &request) const {
    if (should_fail) {
      return {
          BedrockRuntimeError(
              Aws::Client::AWSError<Aws::Client::CoreErrors>(
                  Aws::Client::CoreErrors::INVALID_ACTION,
                  "Invalid parameter value",
                  "Invalid parameter value",
                  false))};
    }

    InvokeModelOutcome outcome = {InvokeModelResult()};
    outcome.GetResult().ReplaceBody(new Aws::StringStream(completion_body));
    return outcome;
  }
};

class scanner_test : public repository_integration_test {
 public:
  void SetUp() override {
    repository_integration_test::SetUp();
    auto repo = services.get<t_client>();
    repo->execute("insert into users (id) values ('" USER_ID "')").go();
  }

 protected:
  std::shared_ptr<sql::Connection> get_connection() override {
    return services.get<repository::t_client>()->get_connection();
  }

  receipt create_receipt() {
    auto repo = services.get<t_client>();
    auto r = receipt{
        .id = "12345",
        .user_id = USER_ID,
        .date = DATE,
        .total_amount = 200.00,
        .currency = "EUR",
        .store_name = "Amazon",
        .category = "",
        .state = receipt::done,
        .image_name = IMAGE_NAME,
        .version = 1,
    };
    repo->create<receipt>(r);
    return r;
  }

  container<
      scoped<TextractClient, fake_textract_client>,
      scoped<BedrockRuntimeClient, fake_bedrock_runtime_client>,

      singleton<Aws::Client::ClientConfiguration>,
      singleton<parameter_manager>,
      singleton<repository::connection_settings>,
      singleton<t_client, client<>>,
      transient<t_receipt_repository, receipt_repository<>>,
      transient<t_category_repository, category_repository<>>,

      transient<t_receipt_extractor, receipt_extractor<>>,
      transient<t_categorizer, categorizer<>>,

      transient<t_handler, handler<>>
  > services;
};

TEST_F(scanner_test, should_create_receipt) {
  auto handler = services.get<t_handler>();
  auto request = create_request();
  auto res = handler->operator()(request);

  ASSERT_TRUE(res.is_success());

  auto repo = services.get<t_client>();
  auto receipts = repo->select<receipt>("select * from receipts").all();
  ASSERT_EQ(receipts->size(), 1);

  auto receipt = receipts->at(0);
  ASSERT_EQ(receipt->store_name, "Amazon");
  ASSERT_EQ(receipt->date, DATE);
  ASSERT_EQ(receipt->total_amount, 100.00);
  ASSERT_EQ(receipt->version, 0);
  ASSERT_EQ(receipt->image_name, IMAGE_NAME);

  auto items = repo->select<receipt_item>("select * from receipt_items").all();
  ASSERT_EQ(items->size(), 1);
  auto item = items->at(0);
  ASSERT_EQ(item->receipt_id, receipt->id);
  ASSERT_EQ(item->description, "Item 1");
  ASSERT_EQ(item->amount, 100.00);
  ASSERT_EQ(item->sort_order, 0);
}

TEST_F(scanner_test, should_ignore_not_receipt_key_pattern) {
  auto handler = services.get<t_handler>();
  auto request = create_request("users/" USER_ID "/whatever");
  auto res = handler->operator()(request);

  ASSERT_TRUE(res.is_success());

  auto repo = services.get<t_client>();
  auto receipts = repo->select<receipt>("select * from receipts").all();
  ASSERT_EQ(receipts->size(), 0);
}

TEST_F(scanner_test, should_continue_if_textract_fails) {
  auto textract = services.get<TextractClient>();
  textract->should_fail = true;

  auto handler = services.get<t_handler>();
  auto request = create_request();
  auto res = handler->operator()(request);

  ASSERT_TRUE(res.is_success());

  auto repo = services.get<t_client>();
  auto receipts = repo->select<receipt>("select * from receipts").all();
  ASSERT_EQ(receipts->size(), 1);
  auto receipt = receipts->at(0);
  ASSERT_EQ(receipt->version, 0);
  ASSERT_EQ(receipt->state, receipt::failed);
  ASSERT_EQ(receipt->date, lambda::utils::today());
  ASSERT_EQ(receipt->store_name, "-");
}

TEST_F(scanner_test, should_recognize_short_year) {
  auto textract = services.get<TextractClient>();
  textract->invoice_date = "24-06-22";

  auto handler = services.get<t_handler>();
  auto request = create_request();
  auto res = handler->operator()(request);

  ASSERT_TRUE(res.is_success());

  auto repo = services.get<t_client>();
  auto receipts = repo->select<receipt>("select * from receipts").all();
  ASSERT_EQ(receipts->size(), 1);
  ASSERT_EQ(receipts->at(0)->date, DATE);
}

TEST_F(scanner_test, should_sanitise_amount) {
  auto test = [this](const std::string &amount) {
    auto textract = services.get<TextractClient>();
    textract->amount = amount;

    auto handler = services.get<t_handler>();
    auto request = create_request();
    auto res = handler->operator()(request);

    ASSERT_TRUE(res.is_success());

    auto repo = services.get<t_client>();
    auto receipts = repo->select<receipt>("select * from receipts").all();
    ASSERT_EQ(receipts->size(), 1);
    ASSERT_EQ(receipts->at(0)->total_amount, 100.00);

    repo->execute("delete from receipts").go();
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

  ASSERT_TRUE(res.is_success());

  auto repo = services.get<t_client>();
  auto receipts = repo->select<receipt>("select * from receipts").all();
  ASSERT_EQ(receipts->size(), 1);
  ASSERT_EQ(receipts->at(0)->total_amount, 0.00);
}

TEST_F(scanner_test, should_import_currency) {
  auto textract = services.get<TextractClient>();
  textract->currency = "USD";

  auto handler = services.get<t_handler>();
  auto request = create_request();
  auto res = handler->operator()(request);

  ASSERT_TRUE(res.is_success());

  auto repo = services.get<t_client>();
  auto receipts = repo->select<receipt>("select * from receipts").all();
  ASSERT_EQ(receipts->size(), 1);
  ASSERT_EQ(receipts->at(0)->currency, "USD");
}

TEST_F(scanner_test, should_handle_incorrect_date) {
  auto textract = services.get<TextractClient>();
  textract->invoice_date = "2024-06-32";

  auto handler = services.get<t_handler>();
  auto request = create_request();
  auto res = handler->operator()(request);

  ASSERT_TRUE(res.is_success());

  auto repo = services.get<t_client>();
  auto receipts = repo->select<receipt>("select * from receipts").all();
  ASSERT_EQ(receipts->size(), 1);
  ASSERT_EQ(receipts->at(0)->date, lambda::utils::today());
}

TEST_F(scanner_test, should_set_default_quantity) {
  auto textract = services.get<TextractClient>();
  textract->set_quantity = false;

  auto handler = services.get<t_handler>();
  auto request = create_request();
  auto res = handler->operator()(request);

  ASSERT_TRUE(res.is_success());

  auto repo = services.get<t_client>();
  auto receipts = repo->select<receipt>("select * from receipts").all();
  ASSERT_EQ(receipts->size(), 1);
  auto items = repo->select<receipt_item>("select * from receipt_items").all();
  ASSERT_EQ(items->size(), 1);
  ASSERT_EQ(items->at(0)->amount, 100.00);
}

TEST_F(scanner_test, should_handle_incorrect_quantity) {
  auto textract = services.get<TextractClient>();
  textract->quantity = "QUANTITY";

  auto handler = services.get<t_handler>();
  auto request = create_request();
  auto res = handler->operator()(request);

  ASSERT_TRUE(res.is_success());

  auto repo = services.get<t_client>();
  auto receipts = repo->select<receipt>("select * from receipts").all();
  ASSERT_EQ(receipts->size(), 1);
  auto items = repo->select<receipt_item>("select * from receipt_items").all();
  ASSERT_EQ(items->size(), 1);
  ASSERT_EQ(items->at(0)->amount, 100.00);
}

TEST_F(scanner_test, should_update_existing_receipt_if_present_by_user_and_image_name) {
  auto r = create_receipt();

  auto handler = services.get<t_handler>();
  auto request = create_request();

  auto res = handler->operator()(request);
  ASSERT_TRUE(res.is_success());

  auto repo = services.get<t_client>();
  auto receipts = repo->select<receipt>("select * from receipts").all();
  ASSERT_EQ(receipts->size(), 1);
  auto receipt = receipts->at(0);
  ASSERT_EQ(receipt->total_amount, 100.00);
  ASSERT_EQ(receipt->id, r.id);
  ASSERT_EQ(receipt->image_name, IMAGE_NAME);
  ASSERT_EQ(receipt->version, r.version + 1);
  for (const auto &item : receipt->items) {
    ASSERT_EQ(item.receipt_id, receipt->id);
  }
}

TEST_F(scanner_test, should_ignore_non_put_s3_events) {
  auto handler = services.get<t_handler>();
  auto request = create_request(DEFAULT_KEY, "ObjectCreated:Delete");
  auto res = handler->operator()(request);

  ASSERT_TRUE(res.is_success());

  auto repo = services.get<t_client>();
  auto receipts = repo->select<receipt>("select * from receipts").all();
  ASSERT_EQ(receipts->size(), 0);
}

TEST_F(scanner_test, should_categorize) {
  auto handler = services.get<t_handler>();
  auto request = create_request();
  auto res = handler->operator()(request);

  ASSERT_TRUE(res.is_success());

  auto repo = services.get<t_client>();
  auto receipts = repo->select<receipt>("select * from receipts").all();
  ASSERT_EQ(receipts->size(), 1);

  auto items = repo->select<receipt_item>("select * from receipt_items").all();
  ASSERT_EQ(items->size(), 1);
  ASSERT_EQ(items->at(0)->category, "Altro");
}

TEST_F(scanner_test, should_store_category_if_no_items_present) {
  auto textract = services.get<TextractClient>();
  textract->add_items = false;

  auto handler = services.get<t_handler>();
  auto request = create_request();
  auto res = handler->operator()(request);

  ASSERT_TRUE(res.is_success());

  auto repo = services.get<t_client>();
  auto receipts = repo->select<receipt>("select * from receipts").all();
  ASSERT_EQ(receipts->size(), 1);
  auto receipt = receipts->at(0);

  ASSERT_EQ(receipt->items.size(), 0);
  ASSERT_EQ(receipt->category, "Altro");
}

TEST_F(scanner_test, should_ignore_category_if_failed) {
  auto bedrock = services.get<BedrockRuntimeClient>();
  bedrock->should_fail = true;

  auto handler = services.get<t_handler>();
  auto request = create_request();
  auto res = handler->operator()(request);

  ASSERT_TRUE(res.is_success());

  auto repo = services.get<t_client>();
  auto receipts = repo->select<receipt>("select * from receipts").all();
  ASSERT_EQ(receipts->size(), 1);

  auto receipt = receipts->at(0);
  auto items = repo->select<receipt_item>("select * from receipt_items").all();
  ASSERT_EQ(items->size(), 1);
  ASSERT_EQ(receipt->category, "");
}

TEST_F(scanner_test, should_sanitise_extra_space_in_category) {
  auto bedrock = services.get<BedrockRuntimeClient>();
  bedrock->completion_body = R"({"completion": "Altro \n"})";

  auto handler = services.get<t_handler>();
  auto request = create_request();
  auto res = handler->operator()(request);

  ASSERT_TRUE(res.is_success());

  auto repo = services.get<t_client>();
  auto receipts = repo->select<receipt>("select * from receipts").all();
  ASSERT_EQ(receipts->size(), 1);

  auto items = repo->select<receipt_item>("select * from receipt_items").all();
  ASSERT_EQ(items->size(), 1);
  ASSERT_EQ(items->at(0)->category, "Altro");
}

TEST_F(scanner_test, should_handle_update_of_deleted_receipt) {
  auto r = create_receipt();
  auto repo = services.get<t_client>();
  repo->execute("update receipts set is_deleted = 1").go();

  auto handler = services.get<t_handler>();
  auto request = create_request();
  auto res = handler->operator()(request);

  ASSERT_TRUE(res.is_success());

  auto receipts = repo->select<receipt>("select * from receipts").all();
  ASSERT_EQ(receipts->size(), 1);
  ASSERT_EQ(receipts->at(0)->version, r.version);   // version should remain the same
}
