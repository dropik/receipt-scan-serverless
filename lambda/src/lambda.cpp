//
// Created by Daniil Ryzhkov on 04/06/2024.
//

#include <lambda/lambda.hpp>

namespace lambda {

std::string get_stage() {
  auto function_name_env = getenv("AWS_LAMBDA_FUNCTION_NAME");
  if (!function_name_env) {
      return "local";
  }

  std::string function_name(function_name_env);

  auto envStartPos = function_name.find_last_of('-');
  std::string stage = function_name.substr(
      envStartPos + 1, function_name.size() - envStartPos - 1);

  return stage;
}

}
