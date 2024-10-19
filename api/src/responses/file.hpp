#pragma once

#include <string>
#include <lambda/json.hpp>

namespace api {
namespace responses {

class file {
 public:
  std::string url;

  JSON_BEGIN_SERIALIZER(file)
      JSON_PROPERTY("auth_url", url)
  JSON_END_SERIALIZER()
};

}
}
