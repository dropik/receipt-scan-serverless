#include <memory>
#include <cstdlib>

#include <aws/core/Aws.h>
#include <aws/core/client/ClientConfiguration.h>
#include <aws/core/http/HttpTypes.h>
#include <aws/core/utils/logging/ConsoleLogSystem.h>

#include <aws/s3/S3Client.h>

#include <aws-lambda-cpp/common/logger.hpp>

#include <config.h>

#ifdef DEBUG
#include <aws-lambda-cpp/common/runtime.hpp>
#endif

#include <repository/models/common.hpp>
#include <rest/api_root.hpp>

#include "models/upload_file_params.hpp"
#include "models/upload_file_response.hpp"

using namespace Aws;
using namespace aws::lambda_runtime;
using namespace Aws::S3;
using namespace Aws::Utils::Logging;
using namespace Aws::Utils::Json;
using namespace aws_lambda_cpp::json;
using namespace aws_lambda_cpp::common;
using namespace aws_lambda_cpp::models::lambda_payloads;
using namespace api;
using namespace api::models;
using namespace repository::models;
using namespace rest;

struct message_response {
  std::string message;

  JSON_BEGIN_SERIALIZER(message_response)
      JSON_PROPERTY("message", message)
  JSON_END_SERIALIZER()
};

struct user {
  std::string name;
  std::string email;

  JSON_BEGIN_SERIALIZER(user)
      JSON_PROPERTY("name", name)
      JSON_PROPERTY("email", email)
  JSON_END_SERIALIZER()
};

static std::function<std::shared_ptr<LogSystemInterface>()> GetConsoleLoggerFactory() {
  return [] {
    return Aws::MakeShared<ConsoleLogSystem>(
      "console_logger",
      LogLevel::Info);
  };
}

int main(int argc, char** argv) {
  SDKOptions options;
  options.loggingOptions.logLevel = Utils::Logging::LogLevel::Info;
  options.loggingOptions.logger_create_fn = GetConsoleLoggerFactory();

#ifdef DEBUG
  aws_lambda_cpp::runtime::set_debug(argc, argv);
  aws_lambda_cpp::runtime::load_payload(argc, argv);
#endif // DEBUG

  InitAPI(options);
  {
    auto l = std::make_shared<logger>("Api");

    Aws::Client::ClientConfiguration config;
#ifdef DEBUG
    config.region = AWS_REGION;
#endif

    auto s3Client= std::make_shared<S3Client>(config);

    api_root api;

    api.any("/hello")([l = std::weak_ptr<logger>(l)](api_resource &res) {
      res.get("/")([l]() {
        l.lock()->info("Version %s", APP_VERSION);
        return message_response{"Hello, World!"};
      });

      res.get<std::string>()([l](const std::string &name) {
        l.lock()->info("Version %s", APP_VERSION);
        return message_response{"Hello, " + name + "!"};
      });
    });

    api.any("/users")([](api_resource &res) {
      res.post<user>("/")([](const user &u) {
        return message_response{"User " + u.name + " created"};
      });

      res.any<int>()([](int user_id, api_resource &res) {
        res.get("/")([user_id]() {
          return message_response{"User with id " + std::to_string(user_id)};
        });

        res.any("/files")([user_id](api_resource &res) {
          res.get("/")([user_id]() {
            return message_response{"User " + std::to_string(user_id) + " files"};
          });
        });
      });
    });

    api.get<int>()([l = std::weak_ptr<logger>(l)](int count) {
        l.lock()->info("Version %s", APP_VERSION);
      return message_response{"Let's count to " + std::to_string(count)};
    });

    api.get<guid>()([l = std::weak_ptr<logger>(l)](const guid &id) {
      l.lock()->info("Version %s", APP_VERSION);
      return message_response{"GUID: " + id};
    });

    api.post<upload_file_params>("/files")([l = std::weak_ptr<logger>(l), s3Client = std::weak_ptr<S3Client>(s3Client)](const upload_file_params &params) {
      l.lock()->info("Version %s", APP_VERSION);

      std::string bucketName(getenv(IMAGES_BUCKET));
      std::string presignedUrl = s3Client.lock()->GeneratePresignedUrlWithSSES3(bucketName,
                                                                                params.name,
                                                                                Aws::Http::HttpMethod::HTTP_PUT);

      return upload_file_response{presignedUrl};
    });

#ifdef DEBUG
    aws_lambda_cpp::runtime::run_debug(api);
#else
    run_handler(api);
#endif // DEBUG
  }
  ShutdownAPI(options);

  return 0;
}

