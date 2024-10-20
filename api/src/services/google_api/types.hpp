//
// Created by Daniil Ryzhkov on 20/10/2024.
//

#pragma once

#include <string>
#include <lambda/json.hpp>

#define STR_ENUM static inline const std::string

namespace api::services::google_api {

struct money {
  std::string currency_code;
  std::string units;
  long long nanos = 0;

  JSON_BEGIN_SERIALIZER(money)
      JSON_PROPERTY("currencyCode", currency_code)
      JSON_PROPERTY("units", units)
      JSON_PROPERTY("nanos", nanos)
  JSON_END_SERIALIZER()
};

}
