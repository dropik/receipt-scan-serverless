#pragma once

#include <memory>
#include <regex>
#include <ctime>
#include <iomanip>

#include <aws/lambda-runtime/runtime.h>

#include <lambda/log.hpp>
#include <lambda/models/payloads/s3.hpp>
#include <repository/receipt_repository.hpp>

#include "config.h"
#include "utils.hpp"
#include "services/receipt_extractor.hpp"
#include "services/categorizer.hpp"

namespace scanner {

struct t_handler {};

template<
    typename TReceiptExtractor = const services::t_receipt_extractor,
    typename TRepository = repository::t_receipt_repository,
    typename TCategorizer = const services::t_categorizer>
class handler {
 public:
  handler(TReceiptExtractor extractor,
          TRepository repository,
          TCategorizer categorizer)
      : m_extractor(std::move(extractor)),
        m_repository(std::move(repository)),
        m_categorizer(std::move(categorizer)) {}

  aws::lambda_runtime::invocation_response operator()(
      const aws::lambda_runtime::invocation_request &request) {
    lambda::log.info("Version %s", APP_VERSION);

    try {
      lambda::models::payloads::s3_request s3_request =
          lambda::json::deserialize<lambda::models::payloads::s3_request>(request.payload);

      for (auto &record : s3_request.records) {
        if (!record.is_put()) {
          lambda::log.info("Skipping not put request.");
          continue;
        }

        auto receipts = m_extractor->extract(record.s3.bucket.name, record.s3.object.key);
        for (auto &receipt : receipts) {
          m_categorizer->categorize(receipt);

          auto existing_receipt = m_repository->get(
              receipt.user_id,
              receipt.file.get_value().file_name,
              receipt.file.get_value().doc_number);

          if (existing_receipt.has_value()) {
            receipt.id = existing_receipt.get_value().id;
            receipt.file = existing_receipt.get_value().file;
            for (auto &item : receipt.items) {
              item.receipt_id = receipt.id;
            }
          }
          m_repository->store(receipt);
        }
      }

      lambda::log.info("All files processed.");
      return aws::lambda_runtime::invocation_response::success("All files processed!", "application/json");
    } catch (const std::exception &e) {
      lambda::log.error("Error occurred while processing invocation request: %s", e.what());
      return aws::lambda_runtime::invocation_response::failure("Internal error occurred.", "application/json");
    }
  }

 private:
  TReceiptExtractor m_extractor;
  TRepository m_repository;
  TCategorizer m_categorizer;
};

}  // namespace scanner
