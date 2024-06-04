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
                           std::shared_ptr<repository::client> repository) :
    m_s3_client(std::move(s3_client)),
    m_bucket(std::move(bucket)),
    m_repository(std::move(repository)) {}

upload_file_response file_service::get_upload_file_url(const std::string &user_id, const upload_file_params &params) {
  if (params.name.empty()) {
    throw rest::api_exception(invalid_argument, "Name is required");
  }

  auto user = m_repository->select<repository::models::user>("select * from users u where u.id = ?")
      .with_param(user_id)
      .first_or_default();
  if (!user) {
    throw rest::api_exception(not_found, "User not found");
  }

  std::string presignedUrl = m_s3_client->GeneratePresignedUrlWithSSES3(m_bucket,
                                                                        params.name,
                                                                        Aws::Http::HttpMethod::HTTP_PUT);

  return upload_file_response{presignedUrl};
}

}
}