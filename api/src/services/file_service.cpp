//
// Created by Daniil Ryzhkov on 04/06/2024.
//

#include <aws/s3/model/DeleteObjectRequest.h>

#include "file_service.hpp"
#include "rest/api_exception.hpp"
#include "../api_errors.hpp"

#include <utility>

using namespace api::models;

namespace api {
namespace services {

static std::string get_key(const std::string &user_id, const std::string &name) {
  return lambda::string::format("users/%s/receipts/%s", user_id.c_str(), name.c_str());
}

file_service::file_service(std::shared_ptr<Aws::S3::S3Client> s3_client,
                           std::string bucket,
                           std::shared_ptr<repository::client> repository,
                           const models::identity &identity) :
    m_s3_client(std::move(s3_client)),
    m_bucket(std::move(bucket)),
    m_repository(std::move(repository)),
    m_identity(identity) {}

file file_service::get_upload_file_url(const upload_file_params &params) {
  if (params.name.empty()) {
    throw rest::api_exception(invalid_argument, "Name is required");
  }

  auto key = get_key(m_identity.user_id, params.name);

  std::string presignedUrl = m_s3_client->GeneratePresignedUrlWithSSES3(m_bucket,
                                                                        key,
                                                                        Aws::Http::HttpMethod::HTTP_PUT);

  return file{presignedUrl};
}

models::file file_service::get_download_file_url(const std::string &name) {
  if (name.empty()) {
    throw rest::api_exception(invalid_argument, "Name is required");
  }

  auto key = get_key(m_identity.user_id, name);

  std::string presignedUrl = m_s3_client->GeneratePresignedUrl(m_bucket,
                                                               key,
                                                               Aws::Http::HttpMethod::HTTP_GET);

  return file{presignedUrl};
}

void file_service::delete_file(const std::string &name) {
  if (name.empty()) {
    throw rest::api_exception(invalid_argument, "Name is required");
  }

  auto key = get_key(m_identity.user_id, name);

  Aws::S3::Model::DeleteObjectRequest request;
  request.WithBucket(m_bucket.c_str()).WithKey(key.c_str());

  auto outcome = m_s3_client->DeleteObject(request);
  if (!outcome.IsSuccess()) {
    throw std::runtime_error(outcome.GetError().GetMessage());
  }
}

}
}
