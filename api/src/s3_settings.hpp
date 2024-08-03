//
// Created by Daniil Ryzhkov on 22/06/2024.
//

#pragma once

#include <string>

namespace api {

struct s3_settings {
  s3_settings() = default;

  explicit s3_settings(std::string bucket)
      : bucket(std::move(bucket)) {}

  std::string bucket;
};

}
