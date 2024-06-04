//
// Created by Daniil Ryzhkov on 04/06/2024.
//

#include "file_service.hpp"
#include "rest/api_exception.hpp"
#include "../api_errors.h"

#include <utility>

using namespace api::models;

namespace api {
namespace services {

file_service::file_service(std::shared_ptr<Aws::S3::S3Client> s3_client,
                           std::string bucket,
                           std::shared_ptr<repository::client> repository,
                           const models::identity &identity) :
    m_s3_client(std::move(s3_client)),
    m_bucket(std::move(bucket)),
    m_repository(std::move(repository)),
    m_identity(identity) {}

upload_file_response file_service::get_upload_file_url(const upload_file_params &params) {
  if (params.name.empty()) {
    throw rest::api_exception(invalid_argument, "Name is required");
  }

  auto key=
      aws_lambda_cpp::common::str_format("users/%s/receipts/%s", m_identity.user_id.c_str(), params.name.c_str());

  std::string presignedUrl = m_s3_client->GeneratePresignedUrlWithSSES3(m_bucket,
                                                                        key,
                                                                        Aws::Http::HttpMethod::HTTP_PUT);

  return upload_file_response{presignedUrl};
}

}
}