//
// Created by Daniil Ryzhkov on 04/06/2024.
//

#pragma once

#include "common.hpp"

namespace repository::models {

struct user {
  /*
   * User profile id, arrives directly as 'sub' from Cognito.
   */
  guid id;

  /*
   * Whether user has an active subscription, which is not paused, or which is cancelled but
   * still active until the end of the billing period, or which is in a grace period.
   */
  bool has_subscription = false;

  /*
   * Expiry time of subscription. We will cache this basically from Google API
   * so that we can use this autonomously without having to call Google API every time.
   */
  lambda::nullable<std::string> subscription_expiry_time;

  /*
   * The token that we get from Google Play when a user subscribes.
   */
  lambda::nullable<std::string> purchase_token;

  /*
   * We will need this when a user resubscribes from Google Play directly, and otherwise
   * we have to way to know for which Speza user the subscription is for.
   */
  lambda::nullable<std::string> payment_account_email;
};

}
