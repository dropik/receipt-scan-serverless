//
// Created by Daniil Ryzhkov on 02/06/2024.
//

#include <stdexcept>

#include <rest/utils.hpp>

namespace rest {

std::string remove_slashes(const std::string &s) {
  std::string text = s;
  std::string::size_type pos = 0;
  while ((pos = text.find('/', pos)) != std::string::npos) {
    text.erase(pos, 1);
  }
  return text;
}

std::string get_next_segment(const std::string &path) {
  auto next_pos = path.find('/', 1);
  if (next_pos == std::string::npos) {
    return path;
  } else {
    return path.substr(0, next_pos);
  }
}

void validate_path(const std::string &path) {
  if (path.empty()) {
    throw std::invalid_argument("Path cannot be empty");
  }
  if (path[0] != '/') {
    throw std::invalid_argument("Path must start with /");
  }
  if (path.size() > 1 && path.find('/', 1) != std::string::npos) {
    throw std::invalid_argument("Path cannot contain more than one segment");
  }
}

}