//
// Created by Daniil Ryzhkov on 03/08/2024.
//

#pragma once

#include <aws/core/client/ClientConfiguration.h>
#include <aws/cognito-idp/CognitoIdentityProviderClient.h>
#include <aws/s3/S3Client.h>

#include "integration_tests_common/repository_integration_test.hpp"
#include "di/container.hpp"
#include "di/parameter_manager.hpp"
#include "repository/connection_settings.hpp"
#include "repository/client.hpp"
#include "rest/api_root.hpp"

#include "../src/settings/s3_settings.hpp"
#include "../src/settings/cognito_settings.hpp"
#include "../src/settings/google_api_settings.hpp"
#include "../src/identity.hpp"
#include "../src/http_request.hpp"
#include "../src/services/user_service.hpp"
#include "../src/services/file_service.hpp"
#include "../src/services/receipt_service.hpp"
#include "../src/services/category_service.hpp"
#include "../src/services/google_api/google_api_auth_provider.hpp"
#include "../src/services/rtdn_service.hpp"
#include "../src/services/google_api/purchases_subscriptions_v2/purchases_subscriptions_v2_client.hpp"
#include "../src/services/budget_service.hpp"

#include "mocks/factories.hpp"

#define USER_ID "d394a832-4011-7023-c519-afe3adaf0233"
#define TEST_BUDGET "d394a832-4011-7023-c519-afe3adaf0233"
#define TEST_CATEGORY "d394a832-4011-7023-c519-afe3adaf0233"
#define TEST_RECEIPT "d394a832-4011-7023-c519-afe3adaf0233"

namespace api::integration_tests {

class base_api_integration_test : public repository_integration_test {
 protected:
  std::shared_ptr<sql::Connection> get_connection() override;
  void SetUp() override;
  std::unique_ptr<rest::api_root> api;
  di::container<
      di::singleton<Aws::Client::ClientConfiguration>,

      di::singleton<di::parameter_manager>,
      di::singleton<repository::connection_settings>,
      di::singleton<settings::s3_settings>,
      di::singleton<settings::cognito_settings>,
      di::singleton<settings::google_api_settings>,

      di::singleton<Aws::S3::S3Client, mocks::mock_s3_client>,
      di::singleton<Aws::CognitoIdentityProvider::CognitoIdentityProviderClient, mocks::mock_cognito_idp_client>,

      di::singleton<repository::t_client, repository::client<>>,
      di::transient<repository::t_category_repository, repository::category_repository<>>,
      di::transient<repository::t_receipt_repository, repository::receipt_repository<>>,

      di::scoped<identity>,
      di::scoped<http_request>,
      di::scoped<services::google_api::google_api_auth_provider>,

      di::transient<services::t_user_service, services::user_service<>>,
      di::transient<services::t_budget_service, services::budget_service<>>,
      di::transient<services::t_category_service, services::category_service<>>,
      di::transient<services::t_file_service, services::file_service<>>,
      di::transient<services::t_receipt_service, services::receipt_service<>>,
      di::scoped<services::google_api::purchases_subscriptions_v2::t_purchases_subscriptions_v2_client,
                 mocks::mock_purchases_subscriptions_v2_client<>>,
      di::scoped<services::google_api::purchases_subscriptions::t_purchases_subscriptions_client,
                 mocks::mock_purchases_subscriptions_client<>>,
      di::transient<services::t_rtdn_service, services::rtdn_service<>>
  > services;
  void init_user(bool has_subscription = false,
                 const std::optional<std::string> &purchase_token = {},
                 const std::optional<std::string> &payment_account_email = {});

  repository::models::budget create_budget(const lambda::nullable<int> &version = lambda::nullable<int>());
  repository::models::category create_category(const lambda::nullable<int> &version = lambda::nullable<int>());
  repository::models::receipt create_receipt(const lambda::nullable<int> &version = lambda::nullable<int>());
  repository::models::receipt_item create_receipt_item(int sort_order);
};

aws::lambda_runtime::invocation_request create_request(const std::string &method,
                                                       const std::string &path,
                                                       const std::string &body,
                                                       const std::string &origin = "https://speza.it");
void assert_response(const aws::lambda_runtime::invocation_response &response,
                     const std::string &expected_status,
                     const std::string &expected_body,
                     bool expect_cors = true);
std::string expected_response(const std::string &status, const std::string &body = "", bool expect_cors = true);
std::string make_body(const std::string &body);
std::string pretty_json(const std::string &json);
std::string compact_json(const std::string &json);

}
