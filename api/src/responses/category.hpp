//
// Created by Daniil Ryzhkov on 09/06/2024.
//

#pragma once

#include <lambda/json.hpp>
#include "../model_types.hpp"

namespace api {
namespace responses {

struct category {
  guid_t id;
  std::string name;

  JSON_BEGIN_SERIALIZER(category)
      JSON_PROPERTY("id", id)
      JSON_PROPERTY("name", name)
  JSON_END_SERIALIZER()
};

}
}
