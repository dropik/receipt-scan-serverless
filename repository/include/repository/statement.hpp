//
// Created by Daniil Ryzhkov on 07/06/2024.
//

#pragma once

#include "base_query.hpp"

namespace repository {

class statement : public repository::base_query {
 public:
  explicit statement(std::shared_ptr<sql::PreparedStatement> stmt);

  statement &with_param(int t);
  statement &with_param(double t);
  statement &with_param(long t);
  statement &with_param(const std::string &t);
  statement &with_param(long double t);
  void go();
};

}
