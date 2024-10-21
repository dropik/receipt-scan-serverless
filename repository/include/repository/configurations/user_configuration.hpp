//
// Created by Daniil Ryzhkov on 04/06/2024.
//

#pragma once

#include "repository/models/user.hpp"
#include "repository_configuration.hpp"

namespace repository::configurations {

template<>
class repository_configuration<models::user> : public common::base_repository_configuration<models::user> {
 public:
  repository_configuration() {
    HAS_TABLE("users");

    HAS_ID(id) WITH_COLUMN("id");
    HAS_BOOL(has_subscription) WITH_COLUMN("has_subscription");
    HAS_OPTIONAL_STRING(subscription_expiry_time) WITH_COLUMN("subscription_expiry_time");
    HAS_OPTIONAL_STRING(purchase_token) WITH_COLUMN("purchase_token");
    HAS_OPTIONAL_STRING(payment_account_email) WITH_COLUMN("payment_account_email");
  }
};

}
