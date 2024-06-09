#pragma once

#include <memory>

#include <aws/lambda-runtime/runtime.h>
#include <aws/textract/TextractClient.h>
#include <aws/bedrock-runtime/BedrockRuntimeClient.h>
#include <lambda/logger.hpp>
#include <lambda/models/payloads/s3.hpp>

#include <repository/client.hpp>

namespace scanner {

class handler {
 public:
  handler(std::shared_ptr<repository::client> repository,
          std::shared_ptr<const Aws::Textract::TextractClient> textract_client,
          std::shared_ptr<const Aws::BedrockRuntime::BedrockRuntimeClient>
              bedrock_client,
          std::shared_ptr<const lambda::logger> logger);

  aws::lambda_runtime::invocation_response operator()(
      const aws::lambda_runtime::invocation_request& request);

 private:
  std::shared_ptr<repository::client> m_repository;
  std::shared_ptr<const Aws::Textract::TextractClient> m_textract_client;
  std::shared_ptr<const Aws::BedrockRuntime::BedrockRuntimeClient>
      m_bedrock_client;
  std::shared_ptr<const lambda::logger> m_logger;

  typedef std::vector<Aws::Textract::Model::ExpenseField> expense_fields_t;
  typedef std::vector<Aws::Textract::Model::LineItemGroup> line_item_groups_t;

  void process_s3_object(
      lambda::models::payloads::s3_record& record);

  bool try_parse_document(const Aws::Textract::Model::ExpenseDocument& document,
                          const repository::models::guid& user_id,
                          const repository::models::guid& request_id);

  bool try_parse_summary_fields(const expense_fields_t& summary_fields,
                                repository::models::receipt& receipt);
  void try_parse_items(const line_item_groups_t& line_item_groups,
                       const repository::models::guid& receipt_id,
                       std::vector<repository::models::receipt_item>& receipt_items);
  bool try_parse_item(const Aws::Textract::Model::LineItemFields& item,
                      repository::models::receipt_item& receipt_item);
  static bool try_parse_date(std::string& result, const std::string& input);
  bool try_parse_total(long double& result, const std::string& input) const;
  const std::string &try_get_currency(const Aws::Textract::Model::ExpenseField &field) const;
  void try_assign_categories(repository::models::receipt& receipt,
                             std::vector<repository::models::receipt_item>& items);
};

}  // namespace scanner
