//
// Created by Daniil Ryzhkov on 04/06/2024.
//

#pragma once

#include <aws/s3/S3Client.h>
#include <aws/s3/model/DeleteObjectRequest.h>
#include <repository/client.hpp>
#include <rest/api_exception.hpp>

#include "../models/file.hpp"
#include "../models/upload_file_params.hpp"
#include "../models/identity.hpp"
#include "../models/s3_settings.hpp"
#include "../api_errors.hpp"

namespace api {
namespace services {

class i_file_service {};

template<
    typename IS3Client = Aws::S3::S3Client,
    typename IS3Settings = models::s3_settings,
    typename IIdentity = const models::identity,
    typename IRepository = repository::i_client>
class file_service {
 public:
  using file = models::file;

  explicit file_service(IS3Client s3_client, IS3Settings s3_settings, IIdentity identity, IRepository repository)
      : m_s3_client(std::move(s3_client)),
        m_bucket(std::move(s3_settings->bucket)),
        m_identity(std::move(identity)),
        m_repository(std::move(repository)) {}

  file get_upload_file_url(const models::upload_file_params &params) {
    if (params.name.empty()) {
      throw rest::api_exception(invalid_argument, "Name is required");
    }

    auto key = get_key(m_identity->user_id, params.name);

    std::string presignedUrl = m_s3_client->GeneratePresignedUrlWithSSES3(m_bucket,
                                                                          key,
                                                                          Aws::Http::HttpMethod::HTTP_PUT);

    return file{presignedUrl};
  }

  models::file get_download_file_url(const std::string &name) {
    if (name.empty()) {
      throw rest::api_exception(invalid_argument, "Name is required");
    }

    auto key = get_key(m_identity->user_id, name);

    std::string presignedUrl = m_s3_client->GeneratePresignedUrl(m_bucket,
                                                                 key,
                                                                 Aws::Http::HttpMethod::HTTP_GET);

    return file{presignedUrl};
  }

  void delete_file(const std::string &name) {
    if (name.empty()) {
      throw rest::api_exception(invalid_argument, "Name is required");
    }

    auto key = get_key(m_identity->user_id, name);

    Aws::S3::Model::DeleteObjectRequest request;
    request.WithBucket(m_bucket.c_str()).WithKey(key.c_str());

    auto outcome = m_s3_client->DeleteObject(request);
    if (!outcome.IsSuccess()) {
      throw std::runtime_error(outcome.GetError().GetMessage());
    }
  }

 private:
  IS3Client m_s3_client;
  std::string m_bucket;
  IIdentity m_identity;
  IRepository m_repository;

  static std::string get_key(const std::string &user_id, const std::string &name) {
    return lambda::string::format("users/%s/receipts/%s", user_id.c_str(), name.c_str());
  }
};

}
}
