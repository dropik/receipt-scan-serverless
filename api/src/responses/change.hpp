//
// Created by Daniil Ryzhkov on 07/08/2024.
//

#pragma once

#include <string>
#include <lambda/nullable.hpp>
#include <lambda/json.hpp>
#include "../model_types.hpp"

namespace api::responses {

namespace change_action {

inline static constexpr auto create = "create";
inline static constexpr auto update = "update";
inline static constexpr auto del = "delete";

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
