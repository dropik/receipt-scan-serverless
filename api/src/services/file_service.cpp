//
// Created by Daniil Ryzhkov on 04/06/2024.
//

#include "file_service.hpp"

#include <utility>

using namespace api::models;

namespace api {
namespace services {

file_service::file_service(std::shared_ptr<Aws::S3::S3Client> s3_client, std::string bucket) :
    m_s3_client(std::move(s3_client)),
    m_bucket(std::move(bucket)) {}

upload_file_response file_service::get_upload_file_url(const std::string &user_id, const upload_file_params &params) {
  std::string presignedUrl = m_s3_client->GeneratePresignedUrlWithSSES3(m_bucket,
                                                                        params.name,
                                                                        Aws::Http::HttpMethod::HTTP_PUT);

  return upload_file_response{presignedUrl};
}

}
}