//
// Created by Daniil Ryzhkov on 03/08/2024.
//

#include "base_api_integration_test.hpp"
#include "../src/api.hpp"
#include "mocks/factories.hpp"
#include "lambda/string_utils.hpp"

using namespace lambda;

namespace api {
namespace integration_tests {

std::shared_ptr<sql::Connection> base_api_integration_test::get_connection() {
  return services.get<repository::t_client>()->get_connection();
}

void base_api_integration_test::SetUp() {
  repository_integration_test::SetUp();
  api = std::move(create_api(services));
}

aws::lambda_runtime::invocation_request create_request(const std::string &method,
                                                       const std::string &path,
                                                       const std::string &body) {
  return {
      .payload = string::format(R"(
{
  "body": %s,
  "resource": "/{proxy+}",
  "path": "%s",
  "httpMethod": "%s",
  "isBase64Encoded": false,
  "pathParameters": {
    "proxy": "%s"
  },
  "headers": {
    "Accept": "application/json",
    "Accept-Encoding": "gzip, deflate, sdch",
    "Accept-Language": "en-US,en;q=0.8",
    "Cache-Control": "max-age=0",
    "User-Agent": "Custom User Agent String"
  },
  "requestContext": {
    "accountId": "123456789012",
    "resourceId": "123456",
    "stage": "dev",
    "requestId": "c6af9ac6-7b61-11e6-9a41-93e8deadbeef",
    "requestTime": "09/Apr/2015:12:34:56 +0000",
    "requestTimeEpoch": 1428582896000,
    "identity": {
      "cognitoIdentityPoolId": null,
      "accountId": null,
      "cognitoIdentityId": null,
      "caller": null,
      "accessKey": null,
      "sourceIp": "127.0.0.1",
      "cognitoAuthenticationType": null,
      "cognitoAuthenticationProvider": null,
      "userArn": null,
      "userAgent": "Custom User Agent String",
      "user": null
    },
    "authorizer": {
      "claims": {
        "sub": "d394a832-4011-7023-c519-afe3adaf0233"
      }
    },
    "path": "%s",
    "resourcePath": "/{proxy+}",
    "httpMethod": "%s",
    "apiId": "1234567890",
    "protocol": "HTTP/1.1"
  }
}
)", body.c_str(), path.c_str(), method.c_str(), path.c_str(), path.c_str(), method.c_str()),
  };
}

void assert_response(const aws::lambda_runtime::invocation_response &response,
                     const std::string &expected_status,
                     const std::string &expected_body) {
  ASSERT_EQ(pretty_json(response.get_payload()), expected_response(expected_status, expected_body));
}

std::string expected_response(const std::string &status, const std::string &body) {
  auto compact = compact_json(body);
  string::replace_all(compact, "\"", "\\\"");

  return pretty_json(string::format(R"(
{
  "body": "%s",
  "headers": {},
  "isBase64Encoded": false,
  "multiValueHeaders": {},
  "statusCode": %s
}
  )", compact.c_str(), status.c_str()));
}

std::string pretty_json(const std::string &json) {
  return Aws::Utils::Json::JsonView(json).WriteReadable();
}

std::string compact_json(const std::string &json) {
  return Aws::Utils::Json::JsonView(json).WriteCompact(false);
}

}
}
