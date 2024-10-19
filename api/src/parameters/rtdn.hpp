//
// Created by Daniil Ryzhkov on 19/10/2024.
//

#pragma once

#include <string>
#include <lambda/json.hpp>
#include <aws/core/utils/base64/Base64.h>

namespace api::parameters {

template<typename TNotification>
struct rtdn_message {
  std::string data;

  TNotification decode() const {
    Aws::Utils::Base64::Base64 base64;
    auto buf = base64.Decode(data);
    unsigned char* u_array = buf.GetUnderlyingData();

    std::string result(u_array, u_array + buf.GetLength());
    return lambda::json::deserialize<TNotification>(result);
  }

  JSON_BEGIN_SERIALIZER(rtdn_message)
      JSON_PROPERTY("data", data)
  JSON_END_SERIALIZER()
};

template<typename TNotification>
struct rtdn {
  rtdn_message<TNotification> message;

  TNotification get_message() const {
    return std::move(message.decode());
  }

  JSON_BEGIN_SERIALIZER(rtdn)
      JSON_PROPERTY("message", message)
  JSON_END_SERIALIZER()
};

}
