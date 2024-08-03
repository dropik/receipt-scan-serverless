//
// Created by Daniil Ryzhkov on 23/06/2024.
//

#pragma once

#include "gtest/gtest.h"
#include <fstream>
#include <memory>
#include <mariadb/conncpp/Statement.hpp>
#include <mariadb/conncpp/Connection.hpp>

#include "lambda/log.hpp"

#define DEFAULT_USER_ID "user_id"

class base_repository_integration_test : public ::testing::Test {
 protected:
  void SetUp() override;
  void TearDown() override;

  virtual std::shared_ptr<sql::Connection> get_connection() = 0;
};
