//
// Created by Daniil Ryzhkov on 03/08/2024.
//

#include <lambda/string_utils.hpp>
#include "user.hpp"

namespace api::responses {

user user::from_repository(const repository::models::user &u) {
  return {
      .id = u.id,
      .has_subscription = u.verify_subscription(),
      .subscription_expiration_time = u.subscription_expiry_time,
  };
}

}
