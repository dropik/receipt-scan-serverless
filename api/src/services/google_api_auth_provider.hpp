//
// Created by Daniil Ryzhkov on 19/10/2024.
//

#pragma once

#include <memory>
#include <optional>
#include <aws/core/client/ClientConfiguration.h>

#include "../settings/google_api_settings.hpp"

namespace api::services {

class google_api_auth_provider {
 public:
  explicit google_api_auth_provider(
      std::shared_ptr<api::settings::google_api_settings> settings,
      std::shared_ptr<Aws::Client::ClientConfiguration> client_configuration);
  std::string get_access_token();

 private:
  std::shared_ptr<api::settings::google_api_settings> m_settings;
  std::shared_ptr<Aws::Client::ClientConfiguration> m_client_configuration;
  std::optional<std::string> m_access_token;
};

}
