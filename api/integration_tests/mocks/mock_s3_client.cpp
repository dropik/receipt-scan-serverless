//
// Created by Daniil Ryzhkov on 03/08/2024.
//

#include "mock_s3_client.hpp"

using namespace api::integration_tests::mocks;

mock_s3_client::mock_s3_client(const Aws::Client::ClientConfiguration &client_configuration) {}

std::string mock_s3_client::GeneratePresignedUrlWithSSES3(const std::string &bucket, const std::string &key, Aws::Http::HttpMethod http_method) {
  return "https://s3.amazonaws.com/test-bucket/" + key;
}

std::string mock_s3_client::GeneratePresignedUrl(const std::string &bucket,
                                                 const std::string &key,
                                                 Aws::Http::HttpMethod http_method) {
  return "https://s3.amazonaws.com/test-bucket/" + key;
}

Aws::S3::Model::DeleteObjectOutcome mock_s3_client::DeleteObject(const Aws::S3::Model::DeleteObjectRequest &request) {
    return Aws::S3::Model::DeleteObjectOutcome(Aws::S3::Model::DeleteObjectResult());
}

Aws::S3::Model::ListObjectsV2Outcome mock_s3_client::ListObjectsV2(const Aws::S3::Model::ListObjectsV2Request &request) const {
  auto objects = std::vector<Aws::S3::Model::Object>();
  for (int i = 0; i < 10; i++) {
    objects.push_back(Aws::S3::Model::Object().WithKey("key" + std::to_string(i)));
  }
  return Aws::S3::Model::ListObjectsV2Outcome(Aws::S3::Model::ListObjectsV2Result().WithContents(objects));
}

Aws::S3::Model::DeleteObjectsOutcome mock_s3_client::DeleteObjects(const Aws::S3::Model::DeleteObjectsRequest &request) const {
  return Aws::S3::Model::DeleteObjectsOutcome(Aws::S3::Model::DeleteObjectsResult());
}
