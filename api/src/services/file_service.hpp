//
// Created by Daniil Ryzhkov on 04/06/2024.
//

#pragma once

#include <aws/s3/S3Client.h>
#include <repository/client.hpp>

#include "../models/file_response.hpp"
#include "../models/upload_file_params.hpp"
#include "../models/identity.hpp"

namespace api {
namespace services {

class file_service {
 public:
  explicit file_service(std::shared_ptr<Aws::S3::S3Client> s3_client,
                        std::string bucket,
                        std::shared_ptr<repository::client> repository,
                        const models::identity &identity);

  models::file_response get_upload_file_url(const models::upload_file_params &request);
  models::file_response get_download_file_url(const std::string &name);

 private:
  std::shared_ptr<Aws::S3::S3Client> m_s3_client;
  std::string m_bucket;
  std::shared_ptr<repository::client> m_repository;
  const models::identity &m_identity;
};

}
}
