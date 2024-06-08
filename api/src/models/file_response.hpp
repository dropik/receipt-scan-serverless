#pragma once

#include <string>
#include <aws-lambda-cpp/common/json.hpp>

namespace api {
  namespace models {
    class file_response {
    public:
      std::string url;

      JSON_BEGIN_SERIALIZER(file_response)
        JSON_PROPERTY("url", url)
      JSON_END_SERIALIZER()
    };
  }
}