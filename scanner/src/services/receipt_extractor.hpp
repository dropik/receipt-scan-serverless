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
  }

 private:
  TTextractClient m_textract_client;
};

}
}
