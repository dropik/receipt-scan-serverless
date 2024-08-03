//
// Created by Daniil Ryzhkov on 22/06/2024.
//

#pragma once

#include <lambda/logger.hpp>
#include <lambda/lambda.hpp>
#include <aws/s3/S3Client.h>
#include <repository/client.hpp>
#include <di/container.hpp>
#include <lambda/factories.hpp>
#include <repository/factories.hpp>

#include "identity.hpp"
#include "s3_settings.hpp"

#include "services/file_service.hpp"
#include "services/user_service.hpp"
#include "services/receipt_service.hpp"
#include "services/category_service.hpp"

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
struct service_factory<api::s3_settings> {
  template<typename TContainer, typename TPointerFactory>
  static auto create(TContainer &container, TPointerFactory &&factory) {
    return std::move(factory(getenv("IMAGES_BUCKET")));
  }
};

}
