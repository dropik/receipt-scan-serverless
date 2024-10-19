//
// Created by Daniil Ryzhkov on 04/06/2024.
//

#pragma once

#include <aws/s3/S3Client.h>
#include <aws/s3/model/DeleteObjectRequest.h>
#include <aws/s3/model/DeleteObjectsRequest.h>
#include <aws/s3/model/ListObjectsV2Request.h>
#include <aws/s3/model/ListObjectsV2Result.h>
#include <aws/s3/model/ObjectIdentifier.h>

#include <rest/api_exception.hpp>

#include "../responses/file.hpp"
#include "../identity.hpp"
#include "../settings/s3_settings.hpp"
#include "../api_errors.hpp"

namespace api::services {

class t_file_service {};

template<
    typename TS3Client = Aws::S3::S3Client,
    typename TS3Settings = const settings::s3_settings,
    typename TIdentity = const identity>
class file_service {
 public:
  explicit file_service(TS3Client s3, TS3Settings s3_settings, TIdentity identity)
      : m_s3(std::move(s3)),
        m_bucket(std::move(s3_settings->bucket)),
        m_identity(std::move(identity)) {}

  responses::file get_upload_receipt_image_url(const std::string &image_name) {
    if (image_name.empty()) {
      throw rest::api_exception(invalid_argument, "Name is required");
    }

    auto key = get_receipt_image_key(m_identity->user_id, image_name);

    std::string presignedUrl = m_s3->GeneratePresignedUrlWithSSES3(m_bucket,
                                                                   key,
                                                                   Aws::Http::HttpMethod::HTTP_PUT);

    return responses::file{presignedUrl};
  }

  responses::file get_download_receipt_image_url(const std::string &image_name) {
    if (image_name.empty()) {
      throw rest::api_exception(invalid_argument, "Name is required");
    }

    auto key = get_receipt_image_key(m_identity->user_id, image_name);

    std::string presignedUrl = m_s3->GeneratePresignedUrl(m_bucket,
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

    auto outcome = m_s3->DeleteObject(request);
    if (!outcome.IsSuccess()) {
      throw std::runtime_error(outcome.GetError().GetMessage());
    }
  }

  void delete_receipt_images(const std::string &user_id) {
    auto path_prefix = lambda::string::format("users/%s", user_id.c_str());

    Aws::S3::Model::ListObjectsV2Request list_request;
    list_request.SetBucket(m_bucket);
    list_request.SetPrefix(path_prefix);

    bool more_objects = true;
    while (more_objects) {
      auto list_outcome = m_s3->ListObjectsV2(list_request);

      if (list_outcome.IsSuccess()) {
        const auto& result = list_outcome.GetResult();
        Aws::Vector<Aws::S3::Model::ObjectIdentifier> objects_to_delete;

        for (const auto& object : result.GetContents()) {
          Aws::S3::Model::ObjectIdentifier obj;
          obj.SetKey(object.GetKey());
          objects_to_delete.push_back(obj);
        }

        if (!objects_to_delete.empty()) {
          Aws::S3::Model::DeleteObjectsRequest delete_request;
          Aws::S3::Model::Delete delete_obj;
          delete_obj.SetObjects(objects_to_delete);
          delete_request.SetBucket(m_bucket);
          delete_request.SetDelete(delete_obj);

          auto delete_outcome = m_s3->DeleteObjects(delete_request);
          if (!delete_outcome.IsSuccess()) {
            lambda::log.error("Failed to delete s3 objects: %s", delete_outcome.GetError().GetMessage().c_str());
            throw rest::api_exception(internal, "Failed to delete user s3 objects");
          }
        }

        // Check if there are more objects to retrieve.
        more_objects = result.GetIsTruncated();
        if (more_objects) {
          list_request.SetContinuationToken(result.GetNextContinuationToken());
        }
      } else {
        lambda::log.error("Failed to list s3 objects: %s", list_outcome.GetError().GetMessage().c_str());
        throw rest::api_exception(internal, "Failed to list user s3 objects");
      }
    }
  }

 private:
  TS3Client m_s3;
  std::string m_bucket;
  TIdentity m_identity;

  static std::string get_receipt_image_key(const std::string &user_id, const std::string &name) {
    return lambda::string::format("users/%s/receipts/%s", user_id.c_str(), name.c_str());
  }
};

}
