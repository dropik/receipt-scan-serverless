#pragma once

#include <string>
#include <aws-lambda-cpp/common/json.hpp>

namespace api {
  namespace models {
    class upload_file_params {
      public:
        std::string name;

        JSON_BEGIN_SERIALIZER(upload_file_params)
          JSON_PROPERTY("name", name)
        JSON_END_SERIALIZER()
    };
  }
}