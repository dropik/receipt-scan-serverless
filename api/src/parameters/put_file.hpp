#pragma once

#include <string>
#include <lambda/json.hpp>

namespace api {
namespace parameters {

class put_file {
 public:
  std::string name;

  JSON_BEGIN_SERIALIZER(put_file)
      JSON_PROPERTY("name", name)
  JSON_END_SERIALIZER()
};

}
}
