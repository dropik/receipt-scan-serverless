//
// Created by Daniil Ryzhkov on 18/05/2024.
//

#pragma once

#include <vector>
#include <lambda/json.hpp>

namespace scanner {
namespace models {

struct bedrock_payload {
  std::string prompt;
  int max_tokens_to_sample = 500;
  double temperature = 0.5;
  double top_p = 0.5;
  double top_k = 250;
  std::vector<std::string> stop_sequences;
  std::string anthropic_version = "bedrock-2023-05-31";

  JSON_BEGIN_SERIALIZER(bedrock_payload)
      JSON_PROPERTY("prompt", prompt)
      JSON_PROPERTY("max_tokens_to_sample", max_tokens_to_sample)
      JSON_PROPERTY("temperature", temperature)
      JSON_PROPERTY("top_p", top_p)
      JSON_PROPERTY("top_k", top_k)
      JSON_PROPERTY("stop_sequences", stop_sequences)
      JSON_PROPERTY("anthropic_version", anthropic_version)
  JSON_END_SERIALIZER()
};

}
}