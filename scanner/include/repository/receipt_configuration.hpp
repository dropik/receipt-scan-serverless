#pragma once

#include <string>

#include "repository_configuration.hpp"
#include "models/receipt.hpp"

namespace scanner {
namespace repository {

template <>
class repository_configuration<models::receipt>
    : public common::base_repository_configuration<models::receipt> {
 public:
  repository_configuration() {
    has_table("receipts");

    has_id([](const models::receipt& r) -> const std::string& {
      return r.id;
    }).with_column_name("id");

    has_property<std::string>([](const models::receipt& r) -> const std::string& {
      return r.user_id;
    }).with_column_name("user_id");

    has_property<std::string>([](const models::receipt& r) -> const std::string& {
      return r.date;
    }).with_column_name("date");

    has_property<long double>([](const models::receipt& r) -> long double {
      return r.total_amount;
    }).with_column_name("total_amount");

    has_property<std::string>([](const models::receipt& r) -> const std::string& {
      return r.store_name;
    }).with_column_name("store_name");
  }
};

}  // namespace repository
}  // namespace scanner