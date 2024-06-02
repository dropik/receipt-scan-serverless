//
// Created by Daniil Ryzhkov on 02/06/2024.
//

#include "di.hpp"
#include "config.h"

namespace api {

std::shared_ptr<Aws::Client::ClientConfiguration> singleton_config = nullptr;
std::shared_ptr<aws_lambda_cpp::common::logger> singleton_logger = nullptr;
std::shared_ptr<Aws::S3::S3Client> singleton_s3_client = nullptr;

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

}