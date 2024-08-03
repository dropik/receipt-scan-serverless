//
// Created by Daniil Ryzhkov on 03/08/2024.
//

#pragma once

#include "integration_tests_common/repository_integration_test.hpp"

class base_repository_integration_test : public repository_integration_test {
 public:
  void SetUp() override;
};
