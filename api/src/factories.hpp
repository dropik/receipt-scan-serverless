//
// Created by Daniil Ryzhkov on 22/06/2024.
//

#pragma once

#include "di.hpp"

#include <lambda/logger.hpp>
#include <lambda/lambda.hpp>
#include <aws/s3/S3Client.h>
#include <repository/client.hpp>

#include "models/identity.hpp"
#include "models/s3_settings.hpp"

#include "services/file_service.hpp"
#include "services/user_service.hpp"
#include "services/receipt_service.hpp"
#include "services/category_service.hpp"

namespace api {

template<>
struct service_factory<lambda::logger> {
  template<typename TContainer, typename TPointerFactory>
  static auto create(TContainer &container, TPointerFactory &&factory) {
    return std::move(factory("Api"));
  }
};

template<>
struct service_factory<Aws::Client::ClientConfiguration> {
  template<typename TContainer, typename TPointerFactory>
  static auto create(TContainer &container, TPointerFactory &&factory) {
    auto config = factory();
#ifdef DEBUG
    config->region = AWS_REGION;
#endif
    return std::move(config);
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
struct service_factory<repository::connection_settings> {
  template<typename TContainer, typename TPointerFactory>
  static auto create(TContainer &container, TPointerFactory &&factory) {
    auto stage = lambda::get_stage();
    auto connection_string = repository::get_connection_string(stage, *container.template get<Aws::Client::ClientConfiguration>());
    return std::move(factory(connection_string));
  }
};

template<>
struct service_factory<models::s3_settings> {
  template<typename TContainer, typename TPointerFactory>
  static auto create(TContainer &container, TPointerFactory &&factory) {
    return std::move(factory(getenv("IMAGES_BUCKET")));
  }
};

}
