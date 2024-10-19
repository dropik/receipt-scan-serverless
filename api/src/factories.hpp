//
// Created by Daniil Ryzhkov on 22/06/2024.
//

#pragma once

#include <aws/s3/S3Client.h>
#include <aws/cognito-idp/CognitoIdentityProviderClient.h>

#include <di/container.hpp>

#include <lambda/logger.hpp>
#include <lambda/lambda.hpp>
#include <lambda/factories.hpp>

#include <repository/client.hpp>
#include <repository/factories.hpp>

#include "identity.hpp"
#include "settings/s3_settings.hpp"
#include "settings/cognito_settings.hpp"
#include "settings/google_api_settings.hpp"

#include "services/file_service.hpp"
#include "services/user_service.hpp"
#include "services/receipt_service.hpp"
#include "services/category_service.hpp"
#include "services/google_api_auth_provider.hpp"

namespace di {

template<>
struct service_factory<lambda::logger> {
  template<typename TContainer, typename TPointerFactory>
  static auto create(TContainer &container, TPointerFactory &&factory) {
    return std::move(factory("Api"));
  }
};

template<>
struct service_factory<Aws::S3::S3Client> {
  template<typename TContainer, typename TPointerFactory>
  static auto create(TContainer &container, TPointerFactory &&factory) {
    return std::move(factory(*container.template get<Aws::Client::ClientConfiguration>()));
  }
};

template<>
struct service_factory<api::settings::s3_settings> {
  template<typename TContainer, typename TPointerFactory>
  static auto create(TContainer &container, TPointerFactory &&factory) {
    return std::move(factory(getenv("IMAGES_BUCKET")));
  }
};

template<>
struct service_factory<Aws::CognitoIdentityProvider::CognitoIdentityProviderClient> {
  template<typename TContainer, typename TPointerFactory>
  static auto create(TContainer &container, TPointerFactory &&factory) {
    return std::move(factory(*container.template get<Aws::Client::ClientConfiguration>()));
  }
};

template<>
struct service_factory<api::settings::cognito_settings> {
  template<typename TContainer, typename TPointerFactory>
  static auto create(TContainer &container, TPointerFactory &&factory) {
    return std::move(factory(getenv("COGNITO_USER_POOL_ID")));
  }
};

template<>
struct service_factory<api::settings::google_api_settings> {
  template<typename TContainer, typename TPointerFactory>
  static auto create(TContainer &container, TPointerFactory &&factory) {
    auto parameters = container.template get<parameter_manager>();
    auto private_key_id = parameters->get("GoogleApiSettings/PrivateKeyId", "GOOGLE_API_PRIVATE_KEY_ID");
    auto private_key = parameters->get("GoogleApiSettings/PrivateKey", "GOOGLE_API_PRIVATE_KEY");
    auto client_email = parameters->get("GoogleApiSettings/ClientEmail", "GOOGLE_API_CLIENT_EMAIL");
    return std::move(factory(private_key_id, private_key, client_email));
  }
};

template<>
struct service_factory<api::services::google_api_auth_provider> {
  template<typename TContainer, typename TPointerFactory>
  static auto create(TContainer &container, TPointerFactory &&factory) {
    auto settings = container.template get<api::settings::google_api_settings>();
    return std::move(factory(settings));
  }
};

}
