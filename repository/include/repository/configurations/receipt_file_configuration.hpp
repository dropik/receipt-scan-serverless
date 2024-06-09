//
// Created by Daniil Ryzhkov on 09/06/2024.
//

#pragma once

#include "repository_configuration.hpp"
#include "repository/models/receipt_file.hpp"

namespace repository {
namespace configurations {

template<>
class repository_configuration<models::receipt_file>
    : public common::base_repository_configuration<models::receipt_file> {
 public:
  repository_configuration() {
    HAS_TABLE("receipt_files");

    HAS_ID(id)WITH_COLUMN("id");

    HAS_STRING(receipt_id)WITH_COLUMN("receipt_id");
    HAS_STRING(file_name)WITH_COLUMN("file_name");
    HAS_INT(doc_number)WITH_COLUMN("doc_number");
  }
};

}
}
