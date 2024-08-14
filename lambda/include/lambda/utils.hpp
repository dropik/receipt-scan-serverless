//
// Created by Daniil Ryzhkov on 10/08/2024.
//

#pragma once

#include <string>

namespace lambda {
namespace utils {

std::string today();
std::string lpad(const std::string &s, size_t n, char c = ' ');

}
}
