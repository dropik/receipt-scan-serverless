//
// Created by Daniil Ryzhkov on 03/08/2024.
//

#include "s3_mock_client.hpp"

using namespace api::integration_tests::mocks;

s3_mock_client::s3_mock_client(Aws::Client::ClientConfiguration &client_configuration) {}

std::string s3_mock_client::GeneratePresignedUrlWithSSES3(const std::string &bucket, const std::string &key, Aws::Http::HttpMethod http_method) {
  return "https://s3.amazonaws.com/" + bucket + "/" + key;
}

std::string s3_mock_client::GeneratePresignedUrl(const std::string &bucket,
                                                 const std::string &key,
                                                 Aws::Http::HttpMethod http_method) {
  return "https://s3.amazonaws.com/" + bucket + "/" + key;
}
Aws::S3::Model::DeleteObjectOutcome s3_mock_client::DeleteObject(const Aws::S3::Model::DeleteObjectRequest &request) {
    return Aws::S3::Model::DeleteObjectOutcome(Aws::S3::Model::DeleteObjectResult());
}

