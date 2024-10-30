//
// Created by Daniil Ryzhkov on 23/06/2024.
//

#pragma once

#include <aws/bedrock-runtime/BedrockRuntimeClient.h>
#include <aws/bedrock-runtime/model/InvokeModelRequest.h>
#include "repository/models/receipt.hpp"
#include "repository/category_repository.hpp"
#include "../models/bedrock_payload.hpp"
#include "../models/bedrock_response.hpp"

namespace scanner::services {

struct t_categorizer {};

template<
    typename TBedrockRuntimeClient = const Aws::BedrockRuntime::BedrockRuntimeClient,
    typename TCategoryRepository = const repository::t_category_repository>
class categorizer {
 public:
  explicit categorizer(TBedrockRuntimeClient bedrock_runtime_client, TCategoryRepository category_repository)
  : m_bedrock_runtime_client(std::move(bedrock_runtime_client)),
  m_category_repository(std::move(category_repository)) {}

  void categorize(repository::models::receipt &receipt) const {
    const auto categories = m_category_repository->get_all(receipt.user_id);
    std::vector<std::string> category_names;

    for (auto &category : categories) {
      category_names.push_back(category.name);
    }
    const auto categories_str = lambda::string::join(", ", category_names);

    // Preparing prompt
    Aws::BedrockRuntime::Model::InvokeModelRequest invoke_request;
    invoke_request.WithModelId("anthropic.claude-instant-v1");
    invoke_request.SetContentType("application/json");
    models::bedrock_payload payload;

    if (!receipt.items.empty()) {
      std::string prompt_start_format =
          "\n\nHuman: For each receipt item guess and print a category (only) or a \"-\" if not possible to determine, "
          "using following categories: %s.\nReceipt: %s %.2Lf %s.\nItems:";

      payload.prompt = lambda::string::format(
          prompt_start_format, categories_str.c_str(), receipt.store_name.c_str(),
          receipt.total_amount, receipt.currency.c_str());

      std::string prompt_item_format = "\n%d. %s %.2Lf %s";

      for (auto &item : receipt.items) {
        payload.prompt += lambda::string::format(
            prompt_item_format, item.sort_order, item.description.c_str(),
            item.amount, receipt.currency.c_str());
      }
    } else {
      std::string prompt_format =
          "\n\nHuman: Guess and print category (only) of receipt or a \"-\" if not possible to determine, using following "
          "categories: %s.\nReceipt: %s %.2Lf %s.";

      payload.prompt = lambda::string::format(
          prompt_format, categories_str.c_str(), receipt.store_name.c_str(),
          receipt.total_amount, receipt.currency.c_str());
    }

    payload.prompt += "\n\nAssistant:";

    // Invoking bedrock model
    std::string payload_str = lambda::json::serialize(payload);
    auto ss = std::make_shared<std::stringstream>();
    *ss << payload_str;
    invoke_request.SetBody(ss);
    const auto outcome = invoke_model(invoke_request);
    if (!outcome.IsSuccess()) {
      lambda::log.error("Error occurred while invoking bedrock model: %s",
                        outcome.GetError().GetMessage().c_str());
      return;
    }
    const auto &result = outcome.GetResult();
    auto &body = result.GetBody();

    // Parsing categories
    std::stringstream response_ss;
    std::string line;
    while (std::getline(body, line)) {
      response_ss << line;
    }
    std::string response_str = response_ss.str();
    auto response = lambda::json::deserialize<models::bedrock_response>(response_str);

    size_t start = 0;
    if (!receipt.items.empty()) {
      size_t end;
      for (auto &item : receipt.items) {
        end = response.completion.find('\n', start);
        auto category = response.completion.substr(start, end - start);
        utils::ltrim(category);
        utils::rtrim(category);
        item.category = category;
        start = end + 1;
        if (end == std::string::npos) {
          break;
        }
      }
    } else {
      auto category = response.completion.substr(start);
      utils::ltrim(category);
      utils::rtrim(category);
      receipt.category = category;
    }
  }

 private:
  TBedrockRuntimeClient m_bedrock_runtime_client;
  TCategoryRepository m_category_repository;

  using receipt = repository::models::receipt;

  [[nodiscard]] Aws::BedrockRuntime::Model::InvokeModelOutcome invoke_model(const Aws::BedrockRuntime::Model::InvokeModelRequest &request) const {
    try {
      return m_bedrock_runtime_client->InvokeModel(request);
    } catch (const std::exception &e) {
      lambda::log.error("Error occurred while invoking bedrock model: %s", e.what());
      return {};
    }
  }
};

}
