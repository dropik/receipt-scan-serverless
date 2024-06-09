//
// Created by Daniil Ryzhkov on 02/06/2024.
//

#pragma once

#include <memory>

#include <lambda/logger.hpp>
#include <aws/s3/S3Client.h>
#include "repository/client.hpp"
#include "models/identity.hpp"

#include "services/file_service.hpp"
#include "services/user_service.hpp"
#include "services/receipt_service.hpp"
#include "services/category_service.hpp"

namespace api {

template<typename TService>
struct di;

template<>
struct di<lambda::logger> {
  static std::shared_ptr<lambda::logger> get();
};

template<>
struct di<Aws::Client::ClientConfiguration> {
  static std::shared_ptr<const Aws::Client::ClientConfiguration> get();
};

template<>
struct di<Aws::S3::S3Client> {
  static std::shared_ptr<Aws::S3::S3Client> get();
};

template<>
struct di<repository::client> {
  static std::shared_ptr<repository::client> get();
};

template<>
struct di<models::identity> {
  static const models::identity& get();
};

template<>
struct di<services::file_service> {
  static std::unique_ptr<services::file_service> get();
};

template<>
struct di<services::user_service> {
  static std::unique_ptr<services::user_service> get();
};

template<>
struct di<services::receipt_service> {
  static std::unique_ptr<services::receipt_service> get();
};

template<>
struct di<services::category_service> {
  static std::unique_ptr<services::category_service> get();
};

}
