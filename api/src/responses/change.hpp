//
// Created by Daniil Ryzhkov on 07/08/2024.
//

#pragma once

#include <string>
#include <lambda/nullable.hpp>
#include <lambda/json.hpp>
#include "../model_types.hpp"

namespace api {
namespace responses {

namespace change_action {

static constexpr auto create = "create";
static constexpr auto update = "update";
static constexpr auto del = "delete";

}

template<typename T>
struct change {
  std::string action;
  guid_t id;
  lambda::nullable<T> body;

  JSON_BEGIN_SERIALIZER(change<T>)
      JSON_PROPERTY("action", action)
      JSON_PROPERTY("id", id)
      JSON_PROPERTY("body", body)
  JSON_END_SERIALIZER()
};

}
}
