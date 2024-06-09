//
// Created by Daniil Ryzhkov on 02/06/2024.
//

#pragma once

#include <utility>
#include <lambda/json.hpp>

namespace rest {

struct api_exception : public std::exception {
  int error;
  std::string message;

  api_exception(int error, std::string message) : error(error), message(std::move(message)) {}

  JSON_BEGIN_SERIALIZER(api_exception)
      JSON_PROPERTY("error", error)
      JSON_PROPERTY("message", message)
  JSON_END_SERIALIZER()
};

}