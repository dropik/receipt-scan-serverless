//
// Created by Daniil Ryzhkov on 04/06/2024.
//

#pragma once

#include <aws/s3/S3Client.h>
#include <aws/s3/model/DeleteObjectRequest.h>
#include <rest/api_exception.hpp>

#include "../responses/file.hpp"
#include "../identity.hpp"
#include "../s3_settings.hpp"
#include "../api_errors.hpp"

namespace api {
namespace services {

class t_file_service {};

template<
    typename TS3Client = Aws::S3::S3Client,
    typename TS3Settings = const s3_settings,
    typename TIdentity = const identity>
class file_service {
 public:
  explicit file_service(TS3Client s3_client, TS3Settings s3_settings, TIdentity identity)
      : m_s3_client(std::move(s3_client)),
        m_bucket(std::move(s3_settings->bucket)),
        m_identity(std::move(identity)) {}

  responses::file get_upload_receipt_image_url(const std::string &image_name) {
    if (image_name.empty()) {
      throw rest::api_exception(invalid_argument, "Name is required");
    }

    auto key = get_receipt_image_key(m_identity->user_id, image_name);

    std::string presignedUrl = m_s3_client->GeneratePresignedUrlWithSSES3(m_bucket,
                                                                          key,
                                                                          Aws::Http::HttpMethod::HTTP_PUT);

    return responses::file{presignedUrl};
  }

  responses::file get_download_receipt_image_url(const std::string &image_name) {
    if (image_name.empty()) {
      throw rest::api_exception(invalid_argument, "Name is required");
    }

    auto key = get_receipt_image_key(m_identity->user_id, image_name);

    std::string presignedUrl = m_s3_client->GeneratePresignedUrl(m_bucket,
                                                                 key,
                                                                 Aws::Http::HttpMethod::HTTP_GET);

    return responses::file{presignedUrl};
  }

  void delete_receipt_image(const std::string &image_name) {
    if (image_name.empty()) {
      throw rest::api_exception(invalid_argument, "Name is required");
    }

    auto key = get_receipt_image_key(m_identity->user_id, image_name);

    Aws::S3::Model::DeleteObjectRequest request;
    request.WithBucket(m_bucket.c_str()).WithKey(key.c_str());

    auto outcome = m_s3_client->DeleteObject(request);
    if (!outcome.IsSuccess()) {
      throw std::runtime_error(outcome.GetError().GetMessage());
    }
  }

 private:
  TS3Client m_s3_client;
  std::string m_bucket;
  TIdentity m_identity;

  static std::string get_receipt_image_key(const std::string &user_id, const std::string &name) {
    return lambda::string::format("users/%s/receipts/%s", user_id.c_str(), name.c_str());
  }
};

}
}
