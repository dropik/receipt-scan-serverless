//
// Created by Daniil Ryzhkov on 03/08/2024.
//

#pragma once

#include <lambda/json.hpp>
#include "../model_types.hpp"

namespace api {
namespace parameters {

struct put_device {
  guid_t id;

  JSON_BEGIN_SERIALIZER(put_device)
      JSON_PROPERTY("id", id)
  JSON_END_SERIALIZER()
};

}
}
