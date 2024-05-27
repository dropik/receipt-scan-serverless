//
// Created by Daniil Ryzhkov on 20/05/2024.
//

#pragma once

#include <aws/lambda-runtime/runtime.h>

#include <aws-lambda-cpp/models/lambda_payloads/gateway_proxy.hpp>
#include <aws-lambda-cpp/models/lambda_responses/gateway_proxy.hpp>
#include <utility>

namespace api {

typedef aws_lambda_cpp::models::lambda_payloads::base_gateway_proxy_request api_request_t;
typedef aws_lambda_cpp::models::lambda_responses::base_gateway_proxy_response api_response_t;

static bool is_status_ok(int status_code) {
  return status_code >= 200 && status_code < 300;
}

template<typename T>
static api_response_t ok(const T &payload) {
  api_response_t response;
  response.status_code = 200;
  response.set_body(aws_lambda_cpp::json::serialize(payload, true), false);
  return response;
}

static api_response_t bad_request() {
  api_response_t response;
  response.status_code = 400;
  response.set_body("Bad request", false);
  return response;
}

static api_response_t not_found() {
  api_response_t response;
  response.status_code = 404;
  response.set_body("Not found", false);
  return response;
}

static api_response_t method_not_allowed() {
  api_response_t response;
  response.status_code = 405;
  response.set_body("Method not allowed", false);
  return response;
}

static std::string remove_slashes(const std::string &s) {
  std::string text = s;
  std::string::size_type pos = 0;
  while ((pos = text.find('/', pos)) != std::string::npos) {
    text.erase(pos, 1);
  }
  return text;
}

int parse_int(const std::string &s) {
  std::string int_text = remove_slashes(s);
  return std::stoi(int_text);
}

std::string parse_string(const std::string &s) {
  return remove_slashes(s);
}

template<typename TParam>
struct parser {
  constexpr static auto parse = 0;
};

template<>
struct parser<int> {
  static int parse(const std::string &s) {
    return parse_int(s);
  }
};

template<>
struct parser<std::string> {
  static std::string parse(const std::string &s) {
    return parse_string(s);
  }
};

static std::string get_next_segment(const std::string &path) {
  auto next_pos = path.find('/', 1);
  if (next_pos == std::string::npos) {
    return path;
  } else {
    return path.substr(0, next_pos);
  }
}

static void validate_path(const std::string &path) {
  if (path.empty()) {
    throw std::invalid_argument("Path cannot be empty");
  }
  if (path[0] != '/') {
    throw std::invalid_argument("Path must start with /");
  }
  if (path.size() > 1 && path.find('/', 1) != std::string::npos) {
    throw std::invalid_argument("Path cannot contain more than one segment");
  }
}

class api_resource {
 public:
  auto get(const std::string &path) {
    validate_path(path);

    return [this, path](auto &&h) {
      this->m_routes.push_back([path, h](const api_request_t &request, const std::string &p) {
        if (p != path) {
          return not_found();
        }
        if (request.http_method != "GET") {
          return method_not_allowed();
        }
        return ok(h());
      });
    };
  }

  template<typename TBody>
  auto post(const std::string &path) {
    validate_path(path);

    return [this, path](auto &&h) {
      this->m_routes.push_back([path, h](const api_request_t &request, const std::string &p) {
        if (p != path) {
          return not_found();
        }
        if (request.http_method != "POST") {
          return method_not_allowed();
        }
        TBody body;
        try {
          body = aws_lambda_cpp::json::deserialize<TBody>(request.get_body());
        } catch (std::exception &e) {
          return bad_request();
        }
        return ok(h(body));
      });
    };
  }

  auto any(const std::string &path) {
    validate_path(path);

    return [this, path](auto &&config_function) {
      this->m_routes.push_back([path, config_function](const api_request_t &request, const std::string &p) {
        auto next_segment = get_next_segment(p);
        if (next_segment != path) {
          return not_found();
        }

        api_resource nested;
        config_function(nested);

        auto nested_path = p.size() == path.size() ? "/" : p.substr(path.size());
        return nested(request, nested_path);
      });
    };
  }

  template<typename TParam>
  auto get() {
    using TRawParam = typename std::decay<TParam>::type;
    static_assert(std::is_function<decltype(parser<TRawParam>::parse)>::value, "No parser found for type");

    return [this](auto &&h) {
      this->m_routes.push_back([h](const api_request_t &request, const std::string &p) {
        auto next_segment = get_next_segment(p);
        if (next_segment.empty()) {
          return not_found();
        }
        if (next_segment.size() != p.size()) {
          // this is indeed not found, because a non ANY route should not have any segments after the path
          return not_found();
        }
        if (request.http_method != "GET") {
          return method_not_allowed();
        }
        TRawParam param;
        try {
          param = parser<TRawParam>::parse(next_segment);
        } catch (std::exception &e) {
          return not_found();
        }
        return ok(h(param));
      });
    };
  }

  template<typename TParam>
  auto any() {
    using TRawParam = typename std::decay<TParam>::type;
    static_assert(std::is_function<decltype(parser<TRawParam>::parse)>::value, "No parser found for type");

    return [this](auto &&config_function) {
      this->m_routes.push_back([config_function](const api_request_t &request, const std::string &p) {
        auto next_segment = get_next_segment(p);
        if (next_segment.empty()) {
          return not_found();
        }

        TRawParam param;
        try {
          param = parser<TRawParam>::parse(next_segment);
        } catch (std::exception &e) {
          return not_found();
        }

        api_resource nested;
        config_function(param, nested);

        auto nested_path = next_segment.size() >= p.size() ? "/" : p.substr(next_segment.size());
        return nested(request, nested_path);
      });
    };
  }

  api_response_t operator()(const api_request_t &request, const std::string &path) {
    bool met404 = false;
    bool met405 = false;
    for (const auto &route : m_routes) {
      auto response = route(request, path);
      if (response.status_code == 404) {
        met404 = true;
        continue;
      } else if (response.status_code == 405) {
        met405 = true;
        continue;
      } else {
        return response;
      }
    }

    if (met405) {
      return method_not_allowed();
    } else if (met404) {
      return not_found();
    } else {
      return bad_request();
    }
  }

 private:
  std::vector<std::function<api_response_t(const api_request_t &, const std::string &)>> m_routes;
};

class api_root : public api_resource {
 public:
  aws::lambda_runtime::invocation_response operator()(const aws::lambda_runtime::invocation_request &request) {
    api_request_t gpr = aws_lambda_cpp::json::deserialize<api_request_t>(request.payload);

    auto path = gpr.path;
    // sanitizing trailing slashes
    if (path.size() > 1 && path[path.size() - 1] == '/') {
      path = path.substr(0, path.size() - 1);
    }

    api_response_t response = this->api_resource::operator()(gpr, path);

    auto response_json = aws_lambda_cpp::json::serialize(response, true);
    return aws::lambda_runtime::invocation_response::success(response_json, "application/json");
  }
};

}