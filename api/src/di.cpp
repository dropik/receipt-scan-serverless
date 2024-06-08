//
// Created by Daniil Ryzhkov on 02/06/2024.
//

#include <lambda/runtime.hpp>

#include "di.hpp"
#include "config.h"

namespace api {

std::shared_ptr<Aws::Client::ClientConfiguration> singleton_config = nullptr;
std::shared_ptr<aws_lambda_cpp::common::logger> singleton_logger = nullptr;
std::shared_ptr<Aws::S3::S3Client> singleton_s3_client = nullptr;
std::shared_ptr<repository::client> singleton_repository = nullptr;

std::shared_ptr<aws_lambda_cpp::common::logger> di<aws_lambda_cpp::common::logger>::get() {
  if (!singleton_logger) {
    singleton_logger = std::make_shared<aws_lambda_cpp::common::logger>("Api");
  }
  return singleton_logger;
}

std::shared_ptr<const Aws::Client::ClientConfiguration> di<Aws::Client::ClientConfiguration>::get() {
  if (!singleton_config) {
    singleton_config = std::make_shared<Aws::Client::ClientConfiguration>();
#ifdef DEBUG
    singleton_config->region = AWS_REGION;
#endif
  }
  return singleton_config;
}

std::shared_ptr<Aws::S3::S3Client> di<Aws::S3::S3Client>::get() {
  if (!singleton_s3_client) {
    singleton_s3_client = std::make_shared<Aws::S3::S3Client>(
        *di<Aws::Client::ClientConfiguration>::get());
  }
  return singleton_s3_client;
}

std::shared_ptr<repository::client> di<repository::client>::get() {
  if (!singleton_repository) {
    auto stage = lambda::get_stage();
    auto connection_string = repository::get_connection_string(stage, *di<Aws::Client::ClientConfiguration>::get());
    singleton_repository = std::make_shared<repository::client>(
        connection_string,
        di<aws_lambda_cpp::common::logger>::get());
  }
  return singleton_repository;
}

const models::identity &di<models::identity>::get() {
  return models::current_identity;
}

std::unique_ptr<services::file_service> di<services::file_service>::get() {
  return std::make_unique<services::file_service>(
      di<Aws::S3::S3Client>::get(),
      getenv(IMAGES_BUCKET),
      di<repository::client>::get(),
      di<models::identity>::get());
}

std::unique_ptr<services::user_service> di<services::user_service>::get() {
  return std::make_unique<services::user_service>(
      di<repository::client>::get(),
      di<models::identity>::get());
}

std::unique_ptr<services::receipt_service> di<services::receipt_service>::get() {
  return std::make_unique<services::receipt_service>(
      di<repository::client>::get(),
      di<models::identity>::get(),
      di<services::file_service>::get());
}

}
