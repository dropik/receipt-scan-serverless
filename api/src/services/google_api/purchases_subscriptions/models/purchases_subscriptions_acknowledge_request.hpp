//
// Created by Daniil Ryzhkov on 20/10/2024.
//

#pragma once

#include <string>
#include <lambda/json.hpp>

namespace api::services::google_api::purchases_subscriptions::models {

struct purchases_subscriptions_acknowledge_request {
  std::string developer_payload;

  JSON_BEGIN_SERIALIZER(purchases_subscriptions_acknowledge_request)
      JSON_PROPERTY("developerPayload", developer_payload)
  JSON_END_SERIALIZER()
};

}
