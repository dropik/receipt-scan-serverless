//
// Created by Daniil Ryzhkov on 03/08/2024.
//

#include "base_api_integration_test.hpp"
#include "../src/api.hpp"
#include "lambda/string_utils.hpp"

using namespace lambda;

namespace api::integration_tests {

std::shared_ptr<sql::Connection> base_api_integration_test::get_connection() {
  return services.get<repository::t_client>()->get_connection();
}

void base_api_integration_test::SetUp() {
  repository_integration_test::SetUp();
  api = std::move(create_api(services));
}

void base_api_integration_test::init_user(bool has_subscription) {
  auto repo = services.get<repository::t_client>();
  repo->create(repository::models::user {
    .id = USER_ID,
    .has_subscription = has_subscription,
  });
}

repository::models::budget base_api_integration_test::create_budget(const lambda::nullable<int>& version) {
  auto repo = services.get<repository::t_client>();
  auto b = repository::models::budget{
      .id = TEST_BUDGET,
      .user_id = USER_ID,
      .month = "2024-07-01",
      .amount = 1500.0,
      .version = version.has_value() ? version.get_value() : 0,
  };
  repo->create<repository::models::budget>(b);
  return b;
}

repository::models::category base_api_integration_test::create_category(const lambda::nullable<int>& version) {
  auto repo = services.get<repository::t_client>();
  auto c = repository::models::category{
      .id = TEST_CATEGORY,
      .user_id = USER_ID,
      .name = "category",
      .color = 29,
      .icon = 62345,
      .version = version.has_value() ? version.get_value() : 0,
  };
  repo->create<repository::models::category>(c);
  return c;
}

repository::models::receipt base_api_integration_test::create_receipt(const lambda::nullable<int>& version) {
  auto repo = services.get<repository::t_client>();
  auto r = repository::models::receipt{
      .id = TEST_RECEIPT,
      .user_id = USER_ID,
      .date = "2024-08-04",
      .total_amount = 100,
      .currency = "EUR",
      .store_name = "store",
      .category = "",
      .state = repository::models::receipt::done,
      .image_name = "image",
      .version = version.has_value() ? version.get_value() : 0,
  };
  repo->create<repository::models::receipt>(r);
  return r;
}

repository::models::receipt_item base_api_integration_test::create_receipt_item(int sort_order) {
  auto repo = services.get<repository::t_client>();
  auto r = repository::models::receipt_item{
      .id = Aws::Utils::UUID::RandomUUID(),
      .receipt_id = TEST_RECEIPT,
      .description = "item",
      .amount = 100,
      .category = "supermarket",
      .sort_order = sort_order,
  };
  repo->create<repository::models::receipt_item>(r);
  return r;
}

aws::lambda_runtime::invocation_request create_request(const std::string &method,
                                                       const std::string &path,
                                                       const std::string &body,
                                                       const std::string &origin) {
  std::map<std::string, std::string> query_string_parameters;
  auto endpoint = path;
  if (path.find_first_of('?') != std::string::npos) {
    auto parts = string::split(path, "?");
    endpoint = parts[0];
    auto query_string = parts[1];
    auto query_parts = string::split(query_string, "&");
    for (const auto &part : query_parts) {
      auto kv = string::split(part, "=");
      query_string_parameters[kv[0]] = kv[1];
    }
  }
  std::vector<std::string> query_string_params_pairs;
  query_string_params_pairs.reserve(query_string_parameters.size());
  for (const auto &pair : query_string_parameters) {
    query_string_params_pairs.push_back(string::format(R"("%s": "%s")", pair.first.c_str(), pair.second.c_str()));
  }
  auto query_string_params_str = string::join(", ", query_string_params_pairs);

  return {
      .payload = string::format(R"(
{
  "body": "%s",
  "resource": "/{proxy+}",
  "path": "%s",
  "httpMethod": "%s",
  "isBase64Encoded": false,
  "pathParameters": {
    "proxy": "%s"
  },
  "queryStringParameters": {
    %s
  },
  "headers": {
    "Accept": "application/json",
    "Accept-Encoding": "gzip, deflate, sdch",
    "Accept-Language": "en-US,en;q=0.8",
    "Cache-Control": "max-age=0",
    "User-Agent": "Custom User Agent String",
    "Origin": "%s"
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
        "sub": "%s"
      }
    },
    "path": "%s",
    "resourcePath": "/{proxy+}",
    "httpMethod": "%s",
    "apiId": "1234567890",
    "protocol": "HTTP/1.1"
  }
}
)",
                                make_body(body).c_str(),
                                endpoint.c_str(),
                                method.c_str(),
                                endpoint.c_str(),
                                query_string_params_str.c_str(),
                                origin.c_str(),
                                USER_ID,
                                endpoint.c_str(),
                                method.c_str()),
  };
}

void assert_response(const aws::lambda_runtime::invocation_response &response,
                     const std::string &expected_status,
                     const std::string &expected_body,
                     bool expect_cors) {
  ASSERT_EQ(pretty_json(response.get_payload()), expected_response(expected_status, expected_body, expect_cors));
}

std::string expected_response(const std::string &status, const std::string &body, bool expect_cors) {
  auto headers = expect_cors ? R"(
    "Access-Control-Allow-Headers": "Content-Type,X-Amz-Date,Authorization,X-Api-Key,X-Amz-Security-Token",
    "Access-Control-Allow-Methods": "OPTIONS,GET,POST,PUT,DELETE,PATCH",
    "Access-Control-Allow-Origin": "https://speza.it"
  )" : "";

  return pretty_json(string::format(R"(
{
  "body": "%s",
  "headers": {
    %s
  },
  "isBase64Encoded": false,
  "multiValueHeaders": {},
  "statusCode": %s
}
  )", make_body(body).c_str(), headers, status.c_str()));
}

std::string make_body(const std::string &body) {
  auto compact = compact_json(body);
  string::replace_all(compact, "\"", "\\\"");
  return compact;
}

std::string pretty_json(const std::string &json) {
  return Aws::Utils::Json::JsonView(json).WriteReadable();
}

std::string compact_json(const std::string &json) {
  return Aws::Utils::Json::JsonView(json).WriteCompact(false);
}

}

