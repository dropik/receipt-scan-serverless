//
// Created by Daniil Ryzhkov on 03/08/2024.
//

#pragma once

#include <aws/core/client/ClientConfiguration.h>
#include <aws/s3/S3Client.h>

#include "integration_tests_common/base_repository_integration_test.hpp"
#include "di/container.hpp"
#include "repository/connection_settings.hpp"
#include "repository/client.hpp"
#include "rest/api_root.hpp"

#include "../src/models/s3_settings.hpp"
#include "../src/models/identity.hpp"
#include "../src/services/user_service.hpp"
#include "../src/services/file_service.hpp"
#include "../src/services/receipt_service.hpp"
#include "../src/services/category_service.hpp"

#include "mocks/mock_s3_client.hpp"

namespace api {
namespace integration_tests {

class base_api_integration_test : public base_repository_integration_test {
 protected:
  std::shared_ptr<sql::Connection> get_connection() override;
  void SetUp() override;
  rest::api_root api;
  di::container<
      di::singleton<Aws::Client::ClientConfiguration>,
      di::singleton<repository::connection_settings>,
      di::singleton<models::s3_settings>,
      di::singleton<Aws::S3::S3Client, mocks::mock_s3_client>,
      di::singleton<repository::t_client, repository::client<>>,

      di::scoped<models::identity>,

      di::transient<services::t_user_service, services::user_service<>>,
      di::transient<services::t_file_service, services::file_service<>>,
      di::transient<services::t_receipt_service, services::receipt_service<>>,
      di::transient<services::t_category_service, services::category_service<>>
  > services;
};

}
}
