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
using namespace Aws::BedrockRuntime;
using namespace scanner;
using namespace scanner::services;
using namespace repository;

struct fake_textract_client {

};

struct fake_bedrock_runtime_client {

};

struct fake_repository {

};

class scanner_test : public ::testing::Test {
 protected:
  void SetUp() override {
    request = aws::lambda_runtime::invocation_request();
    lambda::models::payloads::s3_request s3_request;
    lambda::models::payloads::s3_record s3_record;
    lambda::models::payloads::s3_bucket s3_bucket;
    lambda::models::payloads::s3_object s3_object;
    s3_bucket.name = "bucket";
    s3_object.key = "key";
    s3_record.s3.bucket = s3_bucket;
    s3_record.s3.object = s3_object;
    s3_request.records.push_back(s3_record);
    request.payload = lambda::json::serialize(s3_request);
  }

  void TearDown() override {
  }

  container<
      singleton<TextractClient, fake_textract_client>,
      singleton<BedrockRuntimeClient, fake_bedrock_runtime_client>,
      singleton<repository::t_client, fake_repository>,

      transient<t_receipt_extractor, receipt_extractor<>>,
      transient<t_receipt_repository, receipt_repository<>>,

      transient<t_handler, handler_v2<>>
  > services;

  aws::lambda_runtime::invocation_request request;
};

TEST_F(scanner_test, should_create_receipt) {
  auto handler = services.get<t_handler>();
  auto res = handler->operator()(request);

  EXPECT_TRUE(res.is_success());
}
