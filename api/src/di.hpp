//
// Created by Daniil Ryzhkov on 02/06/2024.
//

#pragma once

#include <memory>

#include <aws-lambda-cpp/common/logger.hpp>
#include <aws/s3/S3Client.h>
#include "repository/client.hpp"
#include "services/file_service.hpp"

namespace api {

template<typename TService>
struct di;

template<>
struct di<aws_lambda_cpp::common::logger> {
  static std::shared_ptr<aws_lambda_cpp::common::logger> get();
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
struct di<services::file_service> {
  static std::unique_ptr<services::file_service> get();
};

}
