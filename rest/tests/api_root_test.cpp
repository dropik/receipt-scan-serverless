//
// Created by Daniil Ryzhkov on 27/05/2024.
//

#include <gtest/gtest.h>
#include <rest/api_root.hpp>
#include <utility>

using namespace aws::lambda_runtime;
using namespace rest;

struct test_generic_parameter {
  std::string value;

  JSON_BEGIN_SERIALIZER(test_generic_parameter)
      JSON_PROPERTY("value", value)
  JSON_END_SERIALIZER()
};

struct test_parameter {
  std::string id = "0";
  std::string name;

  JSON_BEGIN_SERIALIZER(test_parameter)
      JSON_PROPERTY("id", id)
      JSON_PROPERTY("name", name)
  JSON_END_SERIALIZER()
};

struct parameter_with_int_id {
  int id = 0;
  std::string name;

  JSON_BEGIN_SERIALIZER(parameter_with_int_id)
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

TEST(api_root, path_should_contain_slash) {
  api_root api;
  try {
    api.get("123")([]() { return test_response{.value = "3"}; });
    FAIL();
  } catch (std::exception &) {
    SUCCEED();
  }
}

TEST(api_root, path_should_start_with_slash) {
  api_root api;
  try {
    api.get("123/")([]() { return test_response{.value = "3"}; });
    FAIL();
  } catch (std::exception &) {
    SUCCEED();
  }
}

TEST(api_root, path_should_not_end_with_slash) {
  api_root api;
  try {
    api.get("/123/")([]() { return test_response{.value = "3"}; });
    FAIL();
  } catch (std::exception &) {
    SUCCEED();
  }
}

TEST(api_root, path_should_contain_one_segment) {
  api_root api;
  try {
    api.get("/123/456")([]() { return test_response{.value = "3"}; });
    FAIL();
  } catch (std::exception &) {
    SUCCEED();
  }
}

TEST(api_root, path_should_not_be_empty) {
  api_root api;
  try {
    api.get("")([]() { return test_response{.value = "3"}; });
    FAIL();
  } catch (std::exception &) {
    SUCCEED();
  }
}

// GET

TEST(api_root, get_should_return_value) {
  api_root api;
  api.get("/")([]() { return test_response{.value = "3"}; });
  api_request_t request;
  request.path = "/";
  request.http_method = "GET";
  auto response = api(request);
  EXPECT_EQ(response.status_code, 200);
  EXPECT_EQ(response.body, R"({"value":"3"})");
}

TEST(api_root, get_should_capture_int_path_parameters) {
  api_root api;
  api.get<int>()([](int id) { return test_response{.value = std::to_string(id)}; });
  api_request_t request;
  request.path = "/123";
  request.http_method = "GET";
  auto response = api(request);
  EXPECT_EQ(response.body, R"({"value":"123"})");
}

TEST(api_root, get_should_capture_string_path_parameters) {
  api_root api;
  api.get<std::string>()([](std::string id) { return test_response{.value = std::move(id)}; });
  api_request_t request;
  request.path = "/123";
  request.http_method = "GET";
  auto response = api(request);
  EXPECT_EQ(response.body, R"({"value":"123"})");
}

TEST(api_root, get_should_return_not_found_when_parameter_parse_fails) {
  auto test = [](const std::string &path) {
    api_root api;
    api.get<int>()([](int id) { return test_response{.value = std::to_string(id)}; });
    api_request_t request;
    request.path = path;
    request.http_method = "GET";
    auto response = api(request);
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

TEST(api_root, get_should_allow_only_get) {
  auto test = [](const std::string &method) {
    api_root api;
    api.get("/")([]() { return test_response{.value = "3"}; });
    api_request_t request;
    request.path = "/";
    request.http_method = method;
    auto response = api(request);
    EXPECT_EQ(response.status_code, 405);
  };

  test("POST");
  test("PUT");
  test("DELETE");
  test("PATCH");
}

TEST(api_root, get_should_not_find_mismatching_route) {
  auto test = [](const std::string &listen_path, const std::string &path) {
    api_root api;
    api.get(listen_path)([]() { return test_response{.value = "3"}; });
    api_request_t request;
    request.path = path;
    request.http_method = "GET";
    auto response = api(request);
    EXPECT_EQ(response.status_code, 404);
  };

  test("/123", "/456");
  test("/123", "/123/456");
  test("/123", "/12/");
  test("/123", "/1234/");
}

// POST

TEST(api_root, post_should_capture_body) {
  api_root api;
  api.post<test_parameter>("/")([](const test_parameter &param) { return test_response{.value = param.name}; });
  api_request_t request;
  request.body = R"({"id": "0", "name": "Daniil"})";
  request.path = "/";
  request.http_method = "POST";
  auto response = api(request);
  EXPECT_EQ(response.body, R"({"value":"Daniil"})");
}

TEST(api_root, post_should_return_empty_200_if_handler_returns_void) {
  api_root api;
  api.post<test_generic_parameter>("/")([](const test_generic_parameter &param) {});
  api_request_t request;
  request.body = R"({"value": "Daniil"})";
  request.path = "/";
  request.http_method = "POST";
  auto response = api(request);
  EXPECT_EQ(response.status_code, 200);
  EXPECT_EQ(response.body, "");
}

TEST(api_root, post_handler_should_capture_outside_variables) {
  api_root api;
  std::string captured;
  api.post<test_parameter>("/")([&captured](const test_parameter &param) { captured = param.id; });
  api_request_t request;
  request.body = R"({"id": "123", "name": "Daniil"})";
  request.path = "/";
  request.http_method = "POST";
  auto response = api(request);
  EXPECT_EQ(captured, "123");
}

// testing has_id at compile time
static_assert(has_id<test_parameter>::value == true, "has_id<> should give true on types that have id");
static_assert(has_id<test_generic_parameter>::value == false, "has_id<> should give false on types that don't have id");

TEST(api_root, post_should_return_created_at_if_payload_has_id_and_handler_returns_void) {
  auto test = [](const std::string &path, const std::string &expected_location) {
    api_root api;
    api.post<test_parameter>(path)([](const test_parameter &param) {});
    api_request_t request;
    request.body = R"({"id": "123", "name": "Daniil"})";
    request.path = path;
    request.http_method = "POST";
    auto response = api(request);
    EXPECT_EQ(response.status_code, 201);
    EXPECT_EQ(response.headers["Location"], expected_location);
    EXPECT_EQ(response.body, "");
  };

  test("/", "/123");
  test("/users", "/users/123");
}

TEST(api_root, post_parameter_should_work_with_int_id) {
  api_root api;
  api.post<parameter_with_int_id>("/")([](const parameter_with_int_id &param) {});
  api_request_t request;
  request.body = R"({"id": 123, "name": "Daniil"})";
  request.path = "/";
  request.http_method = "POST";
  auto response = api(request);
  EXPECT_EQ(response.status_code, 201);
  EXPECT_EQ(response.headers["Location"], "/123");
  EXPECT_EQ(response.body, "");
}

TEST(api_root, post_should_allow_only_post) {
  auto test = [](const std::string &method) {
    api_root api;
    api.post<test_parameter>("/")([](const test_parameter &param) {});
    api_request_t request;
    request.body = R"({"id": "123", "name": "Daniil"})";
    request.path = "/";
    request.http_method = method;
    auto response = api(request);
    EXPECT_EQ(response.status_code, 405);
  };

  test("GET");
  test("PUT");
  test("DELETE");
  test("PATCH");
}

// PUT

TEST(api_root, put_should_capture_body) {
  api_root api;
  std::string name;
  api.put<test_parameter>("/")([&name](const test_parameter &param) { name = param.name; });
  api_request_t request;
  request.body = R"({"id": "0", "name": "Daniil"})";
  request.path = "/";
  request.http_method = "PUT";
  auto response = api(request);
  EXPECT_EQ(response.status_code, 200);
}

TEST(api_root, put_should_return_empty_body) {
  api_root api;
  api.put<test_parameter>("/")([](const test_parameter &param) {});
  api_request_t request;
  request.body = R"({"id": "0", "name": "Daniil"})";
  request.path = "/";
  request.http_method = "PUT";
  auto response = api(request);
  EXPECT_EQ(response.body, "");
}

TEST(api_root, put_should_allow_only_put) {
  auto test = [](const std::string &method) {
    api_root api;
    api.put<test_parameter>("/")([](const test_parameter &param) {});
    api_request_t request;
    request.body = R"({"id": "123", "name": "Daniil"})";
    request.path = "/";
    request.http_method = method;
    auto response = api(request);
    EXPECT_EQ(response.status_code, 405);
  };

  test("GET");
  test("POST");
  test("DELETE");
  test("PATCH");
}

// PATCH

TEST(api_root, patch_should_capture_body) {
  api_root api;
  std::string name;
  api.patch<test_parameter>("/")([&name](const test_parameter &param) { name = param.name; });
  api_request_t request;
  request.body = R"({"id": "0", "name": "Daniil"})";
  request.path = "/";
  request.http_method = "PATCH";
  auto response = api(request);
  EXPECT_EQ(response.status_code, 200);
}

TEST(api_root, patch_should_return_empty_body) {
  api_root api;
  api.patch<test_parameter>("/")([](const test_parameter &param) {});
  api_request_t request;
  request.body = R"({"id": "0", "name": "Daniil"})";
  request.path = "/";
  request.http_method = "PATCH";
  auto response = api(request);
  EXPECT_EQ(response.body, "");
}

TEST(api_root, patch_should_allow_only_patch) {
  auto test = [](const std::string &method) {
    api_root api;
    api.patch<test_parameter>("/")([](const test_parameter &param) {});
    api_request_t request;
    request.body = R"({"id": "123", "name": "Daniil"})";
    request.path = "/";
    request.http_method = method;
    auto response = api(request);
    EXPECT_EQ(response.status_code, 405);
  };

  test("GET");
  test("POST");
  test("PUT");
  test("DELETE");
}

// DELETE

TEST(api_root, delete_should_capture_int_parameter) {
  api_root api;
  int id = 0;
  api.del<int>()([&id](int param) { id = param; });
  api_request_t request;
  request.path = "/123";
  request.http_method = "DELETE";
  auto response = api(request);
  EXPECT_EQ(id, 123);
}

TEST(api_root, delete_should_capture_string_parameter) {
  api_root api;
  std::string id;
  api.del<std::string>()([&id](std::string param) { id = std::move(param); });
  api_request_t request;
  request.path = "/123";
  request.http_method = "DELETE";
  auto response = api(request);
  EXPECT_EQ(id, "123");
}

TEST(api_root, delete_should_return_empty_body) {
  api_root api;
  api.del<int>()([](int param) {});
  api_request_t request;
  request.path = "/123";
  request.http_method = "DELETE";
  auto response = api(request);
  EXPECT_EQ(response.body, "");
  EXPECT_EQ(response.status_code, 200);
}

TEST(api_root, delete_should_allow_only_delete) {
  auto test = [](const std::string &method) {
    api_root api;
    api.del<int>()([](int param) {});
    api_request_t request;
    request.path = "/123";
    request.http_method = method;
    auto response = api(request);
    EXPECT_EQ(response.status_code, 405);
  };

  test("GET");
  test("POST");
  test("PUT");
  test("PATCH");
}

// Basic routing

TEST(api_root, empty_api_should_return_not_found) {
  api_root api;
  api_request_t request;
  request.path = "/";
  request.http_method = "GET";
  auto response = api(request);
  EXPECT_EQ(response.status_code, 404);
}

TEST(api_root, trailing_slash_should_be_identical) {
  api_root api;
  api.get("/123")([]() { return test_response{.value = "3"}; });
  api_request_t request;
  request.path = "/123/";
  request.http_method = "GET";
  auto response = api(request);
  EXPECT_EQ(response.status_code, 200);
  EXPECT_EQ(response.body, R"({"value":"3"})");
}

// Nested routing

TEST(api_root, any_should_configure_nested_routes) {
  api_root api;
  api.any("/123")([](api_resource &res) {
    res.get("/")([]() { return test_response{.value = "3"}; });
  });
  api_request_t request;
  request.path = "/123";
  request.http_method = "GET";
  auto response = api(request);
  EXPECT_EQ(response.status_code, 200);
  EXPECT_EQ(response.body, R"({"value":"3"})");
}

TEST(api_root, any_should_pass_parameters_to_nested_routes) {
  api_root api;
  api.any<int>()([](int id, api_resource &res) {
    res.get("/")([id]() { return test_response{.value = std::to_string(id)}; });
  });
  api_request_t request;
  request.path = "/123";
  request.http_method = "GET";
  auto response = api(request);
  EXPECT_EQ(response.status_code, 200);
  EXPECT_EQ(response.body, R"({"value":"123"})");
}

TEST(api_root, nested_route_should_be_reachable_behind_parameter) {
  api_root api;
  api.any<int>()([](int id, api_resource &res) {
    res.get("/456")([id]() { return test_response{.value = std::to_string(id)}; });
  });
  api_request_t request;
  request.path = "/123/456";
  request.http_method = "GET";
  auto response = api(request);
  EXPECT_EQ(response.status_code, 200);
  EXPECT_EQ(response.body, R"({"value":"123"})");
}

TEST(api_root, nested_route_should_capture_outside_parameters) {
  api_root api;
  api.any<int>()([](int id, api_resource &res) {
    res.get<int>()([id](int id2) { return test_response{.value = std::to_string(id + id2)}; });
  });
  api_request_t request;
  request.path = "/2/2";
  request.http_method = "GET";
  auto response = api(request);
  EXPECT_EQ(response.status_code, 200);
  EXPECT_EQ(response.body, R"({"value":"4"})");
}

TEST(api_root, any_should_support_multiple_nested_routes) {
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
  auto response = api(request);
  EXPECT_EQ(response.status_code, 200);
  EXPECT_EQ(response.body, R"({"value":"4"})");
}

TEST(api_root, any_should_support_multiple_nested_routes_with_parameters) {
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
  auto response = api(request);
  EXPECT_EQ(response.status_code, 200);
  EXPECT_EQ(response.body, R"({"value":"4"})");
}

TEST(api_root, any_should_support_empty_routes) {
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
  auto response = api(request);
  EXPECT_EQ(response.status_code, 200);
  EXPECT_EQ(response.body, R"({"value":"4"})");
}

// Middleware

TEST(api_root, middleware_should_be_executed) {
  api_root api;
  int value = 0;
  api.get("/")([]() { return test_response{.value = "123"}; });
  api.use([&value](const api_request_t &req, auto next) {
    value = 3;
    return next(req);
  });
  api_request_t request;
  request.path = "/";
  request.http_method = "GET";
  auto response = api(request);
  EXPECT_EQ(response.status_code, 200);
  EXPECT_EQ(response.body, R"({"value":"123"})");
  EXPECT_EQ(value, 3);
}

TEST(api_root, middleware_should_be_executed_in_order) {
  api_root api;
  std::vector<int> values;
  api.get("/")([]() { return test_response{.value = "123"}; });
  api.use([&values](const api_request_t &req, auto next) {
    values.push_back(2);
    return next(req);
  });
  api.use([&values](const api_request_t &req, auto next) {
    values.push_back(1);
    return next(req);
  });
  api_request_t request;
  request.path = "/";
  request.http_method = "GET";
  auto response = api(request);
  std::vector<int> expected_values = {1, 2};
  EXPECT_EQ(values, expected_values);
}

TEST(api_root, api_should_return_500_on_generic_exception) {
  api_root api;
  api.get("/")([]() {
    throw std::runtime_error("error");
    return test_response{.value = "123"};
  });
  api.use_exception_filter();
  api_request_t request;
  request.path = "/";
  request.http_method = "GET";
  auto response = api(request);
  EXPECT_EQ(response.status_code, 500);
}

TEST(api_root, api_should_return_400_if_api_exception_occured) {
  api_root api;
  api.get("/")([]() {
    throw api_exception(0, "Handled API error");
    return test_response{.value = "123"};
  });
  api.use_exception_filter();
  api_request_t request;
  request.path = "/";
  request.http_method = "GET";
  auto response = api(request);
  EXPECT_EQ(response.status_code, 400);
  EXPECT_EQ(response.body, R"({"error":0,"message":"Handled API error"})");
}