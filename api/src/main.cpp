#include <aws/core/Aws.h>
#include <aws/core/utils/logging/ConsoleLogSystem.h>

#ifdef DEBUG
#include <lambda/runtime.hpp>
#endif

#include <lambda/log.hpp>
#include <di/container.hpp>

#include "api.hpp"
#include "factories.hpp"
#include "http_request.hpp"

using namespace Aws;
using namespace aws::lambda_runtime;
using namespace Aws::Utils::Logging;
using namespace api;
using namespace api::settings;
using namespace rest;
using namespace di;
using namespace services;

int main(int argc, char** argv) {
  SDKOptions options;
  options.loggingOptions.logLevel = Utils::Logging::LogLevel::Info;
  options.loggingOptions.logger_create_fn = lambda::GetConsoleLoggerFactory();

#ifdef DEBUG
  lambda::runtime::set_debug(argc, argv);
  lambda::runtime::load_payload(argc, argv);
#endif // DEBUG

  InitAPI(options);
  {
    lambda::log = lambda::logger("Api");

    auto function = [](auto req) {
      container<
          singleton<Aws::Client::ClientConfiguration>,

          singleton<parameter_manager>,
          singleton<repository::connection_settings>,
          singleton<google_api_settings>,
          singleton<s3_settings>,
          singleton<cognito_settings>,

          singleton<Aws::S3::S3Client>,
          singleton<Aws::CognitoIdentityProvider::CognitoIdentityProviderClient>,

          singleton<repository::t_client, repository::client<>>,
          transient<repository::t_category_repository, repository::category_repository<>>,
          transient<repository::t_receipt_repository, repository::receipt_repository<>>,

          scoped<identity>,
          scoped<http_request>,

          transient<t_user_service, user_service<>>,
          transient<t_budget_service, budget_service<>>,
          transient<t_category_service, category_service<>>,
          transient<t_file_service, file_service<>>,
          transient<t_receipt_service, receipt_service<>>
      > services;

      auto api = create_api(services);
      return (*api)(req);
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
