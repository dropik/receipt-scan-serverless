#include "utils.hpp"

#include <aws/core/utils/UUID.h>

std::string scanner::utils::gen_uuid() {
  std::string uuid = Aws::Utils::UUID::RandomUUID();
  std::transform(uuid.begin(), uuid.end(), uuid.begin(), ::tolower);
  return uuid;
}
