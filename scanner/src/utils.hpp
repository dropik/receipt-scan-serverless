#pragma once

#include <string>

namespace scanner {
namespace utils {

std::string gen_uuid();
void ltrim(std::string &s);
void rtrim(std::string &s);
std::string today();
std::string lpad(const std::string &s, size_t n, char c = ' ');

}
}