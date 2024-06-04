//
// Created by Daniil Ryzhkov on 04/06/2024.
//

#pragma once

#include <aws/s3/S3Client.h>
#include "../models/upload_file_response.hpp"
#include "../models/upload_file_params.hpp"

namespace api {
namespace services {

class file_service {
 public:
  explicit file_service(std::shared_ptr<Aws::S3::S3Client> s3_client, std::string bucket);

  models::upload_file_response get_upload_file_url(const std::string &user_id, const models::upload_file_params &request);

 private:
  std::shared_ptr<Aws::S3::S3Client> m_s3_client;
  std::string m_bucket;
};

}
}
