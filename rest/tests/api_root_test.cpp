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

// General path validation

TEST(api_root, path_must_contain_slash) {
  api_root api;
  try {
    api.get("123")([]() { return test_response{.value = "3"}; });
    FAIL();
  } catch (std::exception &) {
    SUCCEED();
  }
}

TEST(api_root, path_must_start_with_slash) {
  api_root api;
  try {
    api.get("123/")([]() { return test_response{.value = "3"}; });
    FAIL();
  } catch (std::exception &) {
    SUCCEED();
  }
}

TEST(api_root, path_must_not_end_with_slash) {
  api_root api;
  try {
    api.get("/123/")([]() { return test_response{.value = "3"}; });
    FAIL();
  } catch (std::exception &) {
    SUCCEED();
  }
}

TEST(api_root, path_must_contain_one_segment) {
  api_root api;
  try {
    api.get("/123/456")([]() { return test_response{.value = "3"}; });
    FAIL();
  } catch (std::exception &) {
    SUCCEED();
  }
}

TEST(api_root, path_must_not_be_empty) {
  api_root api;
  try {
    api.get("")([]() { return test_response{.value = "3"}; });
    FAIL();
  } catch (std::exception &) {
    SUCCEED();
  }
}

// GET

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

TEST(api_root, is_not_found_when_get_parameter_parse_fails) {
  auto test = [](const std::string &path) {
    api_root api;
    api.get<int>()([](int id) { return test_response{.value = std::to_string(id)}; });
    api_request_t request;
    request.path = path;
    request.http_method = "GET";
    auto response = api.route(request, request.path);
    EXPECT_EQ(response.status_code, 404);
  };

  test("/123a");
  test("/a123");
  test("/12.3");
  test("/12,3");
  test("/12 3");
  test("/12-3");
  test("/12+3");
  test("/12*3");
}

// POST

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

// Basic routing

TEST(api_root, is_not_found_on_empty_api) {
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

// Nested routing

TEST(api_root, does_nested_routing_work) {
  api_root api;
  api.any("/123")([](api_resource &res) {
    res.get("/")([]() { return test_response{.value = "3"}; });
  });
  api_request_t request;
  request.path = "/123";
  request.http_method = "GET";
  auto response = api.route(request, request.path);
  EXPECT_EQ(response.status_code, 200);
  EXPECT_EQ(response.body, R"({"value":"3"})");
}

TEST(api_root, does_nested_routing_work_with_parameters) {
  api_root api;
  api.any<int>()([](int id, api_resource &res) {
    res.get("/")([id]() { return test_response{.value = std::to_string(id)}; });
  });
  api_request_t request;
  request.path = "/123";
  request.http_method = "GET";
  auto response = api.route(request, request.path);
  EXPECT_EQ(response.status_code, 200);
  EXPECT_EQ(response.body, R"({"value":"123"})");
}

TEST(api_root, does_nested_routing_work_with_parameters_and_path) {
  api_root api;
  api.any<int>()([](int id, api_resource &res) {
    res.get("/456")([id]() { return test_response{.value = std::to_string(id)}; });
  });
  api_request_t request;
  request.path = "/123/456";
  request.http_method = "GET";
  auto response = api.route(request, request.path);
  EXPECT_EQ(response.status_code, 200);
  EXPECT_EQ(response.body, R"({"value":"123"})");
}

TEST(api_root, does_nested_routing_work_with_parameters_and_path_and_parameters) {
  api_root api;
  api.any<int>()([](int id, api_resource &res) {
    res.get<int>()([id](int id2) { return test_response{.value = std::to_string(id + id2)}; });
  });
  api_request_t request;
  request.path = "/2/2";
  request.http_method = "GET";
  auto response = api.route(request, request.path);
  EXPECT_EQ(response.status_code, 200);
  EXPECT_EQ(response.body, R"({"value":"4"})");
}

TEST(api_root, does_deep_nested_routing_work) {
  api_root api;
  api.any("/1")([](api_resource &res) {
    res.any("/2")([](api_resource &res) {
      res.any("/3")([](api_resource &res) {
        res.any("/4")([](api_resource &res) {
          res.get("/")([]() { return test_response{.value = "4"}; });
        });
      });
    });
  });
  api_request_t request;
  request.path = "/1/2/3/4";
  request.http_method = "GET";
  auto response = api.route(request, request.path);
  EXPECT_EQ(response.status_code, 200);
  EXPECT_EQ(response.body, R"({"value":"4"})");
}

TEST(api_root, does_deep_nested_routing_work_with_parameters) {
  api_root api;
  api.any<int>()([](int id, api_resource &res) {
    res.any<int>()([](int id2, api_resource &res) {
      res.any<int>()([](int id3, api_resource &res) {
        res.any<int>()([](int id4, api_resource &res) {
          res.get("/")([id4]() { return test_response{.value = std::to_string(id4)}; });
        });
      });
    });
  });
  api_request_t request;
  request.path = "/1/2/3/4";
  request.http_method = "GET";
  auto response = api.route(request, request.path);
  EXPECT_EQ(response.status_code, 200);
  EXPECT_EQ(response.body, R"({"value":"4"})");
}

TEST(api_root, does_nesting_work_with_multiple_empty_routes) {
  api_root api;
  api.any("/")([](api_resource &res) {
    res.any("/")([](api_resource &res) {
      res.any("/")([](api_resource &res) {
        res.any("/")([](api_resource &res) {
          res.get("/")([]() { return test_response{.value = "4"}; });
        });
      });
    });
  });
  api_request_t request;
  request.path = "////";
  request.http_method = "GET";
  auto response = api.route(request, request.path);
  EXPECT_EQ(response.status_code, 200);
  EXPECT_EQ(response.body, R"({"value":"4"})");
}
