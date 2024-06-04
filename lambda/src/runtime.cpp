//
// Created by Daniil Ryzhkov on 04/06/2024.
//

#include <lambda/runtime.hpp>

namespace lambda {

std::string get_stage() {
  std::string function_name = getenv("AWS_LAMBDA_FUNCTION_NAME");

  auto envStartPos = function_name.find_last_of('-');
  std::string stage = function_name.substr(
      envStartPos + 1, function_name.size() - envStartPos - 1);

  return stage;
}

}