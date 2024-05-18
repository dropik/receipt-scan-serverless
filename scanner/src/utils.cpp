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