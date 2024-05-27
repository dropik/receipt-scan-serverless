//
// Created by Daniil Ryzhkov on 27/05/2024.
//

#include <gtest/gtest.h>
#include <rest/api_root.hpp>
#include <utility>

using namespace aws::lambda_runtime;
using namespace rest;

struct test_parameter {
  int id = 0;
  std::string name;

  JSON_BEGIN_SERIALIZER(test_parameter)
      JSON_PROPERTY("id", id)
      JSON_PROPERTY("name", name)
  JSON_END_SERIALIZER()
};

struct test_response {
  std::string value;

  JSON_BEGIN_SERIALIZER(test_response)
      JSON_PROPERTY("value", value)
  JSON_END_SERIALIZER()
};

TEST(api_root, does_get_return_value) {
  api_root api;
  api.get("/")([]() { return test_response{.value = "3"}; });
  api_request_t request;
  request.path = "/";
  request.http_method = "GET";
  auto response = api.route(request, request.path);
  EXPECT_EQ(response.status_code, 200);
  EXPECT_EQ(response.body, R"({"value":"3"})");
}

TEST(api_root, does_get_captures_int_path_parameters) {
  api_root api;
  api.get<int>()([](int id) { return test_response{.value = std::to_string(id)}; });
  api_request_t request;
  request.path = "/123";
  request.http_method = "GET";
  auto response = api.route(request, request.path);
  EXPECT_EQ(response.body, R"({"value":"123"})");
}

TEST(api_root, does_get_captures_string_path_parameters) {
  api_root api;
  api.get<std::string>()([](std::string id) { return test_response{.value = std::move(id)}; });
  api_request_t request;
  request.path = "/123";
  request.http_method = "GET";
  auto response = api.route(request, request.path);
  EXPECT_EQ(response.body, R"({"value":"123"})");
}

TEST(api_root, does_post_captures_body) {
  api_root api;
  api.post<test_parameter>("/")([](const test_parameter &param) { return test_response{.value = param.name}; });
  api_request_t request;
  request.body = R"({"id": 0, "name": "Daniil"})";
  request.path = "/";
  request.http_method = "POST";
  auto response = api.route(request, request.path);
  EXPECT_EQ(response.body, R"({"value":"Daniil"})");
}

TEST(api_root, is_not_found) {
  api_root api;
  api_request_t request;
  request.path = "/";
  request.http_method = "GET";
  auto response = api.route(request, request.path);
  EXPECT_EQ(response.status_code, 404);
}

TEST(api_root, is_method_not_allowed) {
  api_root api;
  api.get("/")([]() { return test_response{.value = "3"}; });
  api_request_t request;
  request.path = "/";
  request.http_method = "POST";
  auto response = api.route(request, request.path);
  EXPECT_EQ(response.status_code, 405);
}

TEST(api_root, is_route_not_found) {
  api_root api;
  api.get("/123")([]() { return test_response{.value = "3"}; });
  api_request_t request;
  request.path = "/456";
  request.http_method = "GET";
  auto response = api.route(request, request.path);
  EXPECT_EQ(response.status_code, 404);
}

TEST(api_root, is_trailing_slash_identical) {
  api_root api;
  api.get("/123")([]() { return test_response{.value = "3"}; });
  api_request_t request;
  request.path = "/123/";
  request.http_method = "GET";
  auto response = api.route(request, request.path);
  EXPECT_EQ(response.status_code, 200);
  EXPECT_EQ(response.body, R"({"value":"3"})");
}
