//
// Created by Daniil Ryzhkov on 22/06/2024.
//

#pragma once

#include <lambda/logger.hpp>
#include <aws/core/utils/logging/ConsoleLogSystem.h>

namespace lambda {

inline std::function<std::shared_ptr<LogSystemInterface>()> GetConsoleLoggerFactory() {
  return [] {
    return Aws::MakeShared<ConsoleLogSystem>(
        "console_logger",
        LogLevel::Info);
  };
}

extern lambda::logger log;

}
