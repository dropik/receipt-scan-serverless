//
// Created by Daniil Ryzhkov on 02/06/2024.
//

#pragma once

#include <string>

namespace rest {

template<typename TId>
inline std::string id_to_string(const TId &id) {
  return std::to_string(id);
}

template<>
inline std::string id_to_string(const std::string &id) {
  return id;
}

std::string remove_slashes(const std::string &s);
std::string get_next_segment(const std::string &path);
void validate_path(const std::string &path);

}