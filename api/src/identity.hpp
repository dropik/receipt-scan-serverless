//
// Created by Daniil Ryzhkov on 04/06/2024.
//

#pragma once

#include <string>

namespace api {

struct identity {
  std::string user_id;
  bool has_subscription = false;
  lambda::nullable<std::string> subscription_expiry_time;
};

}
