//
// Created by Daniil Ryzhkov on 10/08/2024.
//

#include "lambda/utils.hpp"

namespace lambda {
namespace utils {

std::string today() {
  time_t now = time(nullptr);
  tm *ltm = localtime(&now);
  return
      std::to_string(1900 + ltm->tm_year) + "-" +
          lpad(std::to_string(1 + ltm->tm_mon), 2, '0') + "-" +
          lpad(std::to_string(ltm->tm_mday), 2, '0');
}

std::string lpad(const std::string &s, size_t n, char c) {
  if (s.size() >= n) {
    return s;
  }
  return std::string(n - s.size(), c) + s;
}

}
}
