//
// Created by Daniil Ryzhkov on 22/06/2024.
//

#include <gtest/gtest.h>
#include <aws/core/Aws.h>
#include "lambda/log.hpp"

using namespace Aws;

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);

  SDKOptions options;
  options.loggingOptions.logLevel = Utils::Logging::LogLevel::Info;
  options.loggingOptions.logger_create_fn = lambda::GetConsoleLoggerFactory();

  int test_result;
  InitAPI(options);
  {
    test_result = RUN_ALL_TESTS();
  }
  ShutdownAPI(options);

  return test_result;
}
