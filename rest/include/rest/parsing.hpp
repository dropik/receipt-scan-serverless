//
// Created by Daniil Ryzhkov on 02/06/2024.
//

#pragma once

#include <stdexcept>

#include "utils.hpp"

namespace rest {

int parse_int(const std::string &s);
std::string parse_string(const std::string &s);

template<typename TParam>
struct parser {
  constexpr static auto parse = 0;
};

template<>
struct parser<int> {
  static int parse(const std::string &s) {
    return parse_int(s);
  }
};

template<>
struct parser<std::string> {
  static std::string parse(const std::string &s) {
    return parse_string(s);
  }
};

}