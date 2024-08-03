//
// Created by Daniil Ryzhkov on 03/08/2024.
//

#include "mock_s3_client.hpp"

using namespace api::integration_tests::mocks;

mock_s3_client::mock_s3_client(const Aws::Client::ClientConfiguration &client_configuration) {}

std::string mock_s3_client::GeneratePresignedUrlWithSSES3(const std::string &bucket, const std::string &key, Aws::Http::HttpMethod http_method) {
  return "https://s3.amazonaws.com/" + bucket + "/" + key;
}

std::string mock_s3_client::GeneratePresignedUrl(const std::string &bucket,
                                                 const std::string &key,
                                                 Aws::Http::HttpMethod http_method) {
  return "https://s3.amazonaws.com/" + bucket + "/" + key;
}
Aws::S3::Model::DeleteObjectOutcome mock_s3_client::DeleteObject(const Aws::S3::Model::DeleteObjectRequest &request) {
    return Aws::S3::Model::DeleteObjectOutcome(Aws::S3::Model::DeleteObjectResult());
}

