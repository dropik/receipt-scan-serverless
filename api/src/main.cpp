#include <memory>

#include <aws/core/Aws.h>
#include <aws/core/http/HttpTypes.h>
#include <aws/core/utils/logging/ConsoleLogSystem.h>

#ifdef DEBUG
#include <lambda/runtime.hpp>
#endif

#include "api.hpp"
#include "di.hpp"
#include "factories.hpp"

using namespace Aws;
using namespace aws::lambda_runtime;
using namespace Aws::Utils::Logging;
using namespace api;
using namespace rest;

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
  lambda::runtime::set_debug(argc, argv);
  lambda::runtime::load_payload(argc, argv);
#endif // DEBUG

  InitAPI(options);
  {
    auto function = [](auto req) {
      service_container<
          singleton<Aws::Client::ClientConfiguration>,
          singleton<repository::connection_settings>,
          singleton<models::s3_settings>,
          singleton<lambda::logger>,
          singleton<Aws::S3::S3Client>,

          scoped<models::identity>,
          scoped<repository::i_client, repository::client<>>,

          transient<services::i_user_service, services::user_service<>>,
          transient<services::i_file_service, services::file_service<>>,
          transient<services::i_receipt_service, services::receipt_service<>>,
          transient<services::i_category_service, services::category_service<>>
      > services;

      auto api = create_api(services);
      return api(req);
    };

#ifdef DEBUG
    lambda::runtime::run_debug(function);
#else
    run_handler(function);
#endif // DEBUG
  }
  ShutdownAPI(options);

  return 0;
}

