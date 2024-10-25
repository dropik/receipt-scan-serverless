//
// Created by Daniil Ryzhkov on 25/10/2024.
//

#pragma once

#include <string>
#include <lambda/json.hpp>

namespace api::parameters {

struct set_user_purchase_token {
  std::string purchase_token;

  JSON_BEGIN_SERIALIZER(set_user_purchase_token)
      JSON_PROPERTY("purchaseToken", purchase_token)
  JSON_END_SERIALIZER()
};

}
