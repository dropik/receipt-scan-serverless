//
// Created by Daniil Ryzhkov on 02/06/2024.
//

#include <rest/parsing.hpp>

namespace rest {

int parse_int(const std::string &s) {
  std::string int_text = rest::remove_slashes(s);
  size_t pos;
  auto res = std::stoi(int_text, &pos);
  if (pos != int_text.size()) {
    throw std::invalid_argument("Cannot parse int");
  }
  return res;
}

std::string parse_string(const std::string &s) {
  return rest::remove_slashes(s);
}

}