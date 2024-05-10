#pragma once

#include <string>
#include <aws-lambda-cpp/common/json.hpp>

namespace api {
  namespace models {
    class upload_file_response {
    public:
      std::string url;

      JSON_BEGIN_SERIALIZER(upload_file_response)
        JSON_PROPERTY("url", url)
      JSON_END_SERIALIZER()
    };
  }
}