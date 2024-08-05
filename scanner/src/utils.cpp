#include "utils.hpp"

#include <string>

#include <aws/core/utils/UUID.h>

std::string scanner::utils::gen_uuid() {
  std::string uuid = Aws::Utils::UUID::RandomUUID();
  std::transform(uuid.begin(), uuid.end(), uuid.begin(), ::tolower);
  return uuid;
}

void scanner::utils::ltrim(std::string& s) {
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
    return !isspace(ch);
  }));
}

void scanner::utils::rtrim(std::string& s) {
  s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
    return !isspace(ch);
  }).base(), s.end());
}

std::string scanner::utils::today() {
  time_t now = time(nullptr);
  tm *ltm = localtime(&now);
  return
      std::to_string(1900 + ltm->tm_year) + "-" +
          lpad(std::to_string(1 + ltm->tm_mon), 2, '0') + "-" +
          lpad(std::to_string(ltm->tm_mday), 2, '0');
}

std::string scanner::utils::lpad(const std::string &s, size_t n, char c) {
  if (s.size() >= n) {
    return s;
  }
  return std::string(n - s.size(), c) + s;
}
