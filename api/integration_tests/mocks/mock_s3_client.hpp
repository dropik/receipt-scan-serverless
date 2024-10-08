//
// Created by Daniil Ryzhkov on 03/08/2024.
//

#pragma once

#include <string>
#include <aws/core/http/HttpTypes.h>
#include <aws/s3/S3ServiceClientModel.h>

namespace api::integration_tests::mocks {

class mock_s3_client {
 public:
  mock_s3_client(const Aws::Client::ClientConfiguration &client_configuration);

  std::string GeneratePresignedUrlWithSSES3(const std::string &bucket,
                                            const std::string &key,
                                            Aws::Http::HttpMethod http_method);

  std::string GeneratePresignedUrl(const std::string &bucket,
                                   const std::string &key,
                                   Aws::Http::HttpMethod http_method);

  Aws::S3::Model::DeleteObjectOutcome DeleteObject(const Aws::S3::Model::DeleteObjectRequest &request);

  Aws::S3::Model::ListObjectsV2Outcome ListObjectsV2 (const Aws::S3::Model::ListObjectsV2Request &request) const;

  Aws::S3::Model::DeleteObjectsOutcome DeleteObjects (const Aws::S3::Model::DeleteObjectsRequest &request) const;
};

}
