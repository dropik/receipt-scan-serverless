//
// Created by Daniil Ryzhkov on 22/06/2024.
//

#pragma once

#include <string>

namespace repository {

struct connection_settings {
  connection_settings() = default;

    explicit connection_settings(std::string connection_string)
        : connection_string(std::move(connection_string)) {}

  std::string connection_string;
};

}
