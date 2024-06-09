//
// Created by Daniil Ryzhkov on 18/05/2024.
//

#pragma once

#include <string>
#include <lambda/json.hpp>

namespace scanner {
namespace models {

struct bedrock_response {
  std::string completion;

  JSON_BEGIN_SERIALIZER(bedrock_response)
      JSON_PROPERTY("completion", completion)
  JSON_END_SERIALIZER()
};

}
}