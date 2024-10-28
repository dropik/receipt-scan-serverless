//
// Created by Daniil Ryzhkov on 03/08/2024.
//

#pragma once

#include <lambda/json.hpp>
#include "../model_types.hpp"
#include "repository/models/user.hpp"

namespace api::responses {

struct user {
  guid_t id;
  bool has_subscription;
  lambda::nullable<std::string> subscription_expiration_time;

  JSON_BEGIN_SERIALIZER(user)
      JSON_PROPERTY("id", id)
      JSON_PROPERTY("hasSubscription", has_subscription)
      JSON_PROPERTY("subscriptionExpirationTime", subscription_expiration_time)
  JSON_END_SERIALIZER()

  static user from_repository(const repository::models::user &u);
};

}
