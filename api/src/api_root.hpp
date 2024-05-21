//
// Created by Daniil Ryzhkov on 20/05/2024.
//

#pragma once

#include <aws/lambda-runtime/runtime.h>

#include <aws-lambda-cpp/models/lambda_payloads/gateway_proxy.hpp>
#include <aws-lambda-cpp/models/lambda_responses/gateway_proxy.hpp>
#include <utility>

namespace api {

struct message_response {
  std::string message;

  JSON_BEGIN_SERIALIZER(message_response)
      JSON_PROPERTY("message", message)
  JSON_END_SERIALIZER()
};

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

class base_route_handler {
 public:
  virtual api_response_t operator()(const api_request_t &request) = 0;
};

template<typename THandler>
class route_handler : public base_route_handler {
 public:
  explicit route_handler(THandler &&handler) : m_handler(std::move(handler)) {}

  api_response_t operator()(const api_request_t &request) override {
    auto result = m_handler(request);
    return ok(result);
  }

 private:
  THandler m_handler;
};

template<typename THandler>
static std::shared_ptr<base_route_handler> make_handler(THandler &&h) {
  return std::make_shared<route_handler<THandler>>(std::forward<THandler>(h));
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

class route_match {
 public:
  virtual bool operator()(const std::string &next_segment) = 0;
};

class exact_route_match : public route_match {
 public:
  explicit exact_route_match(std::string segment) : m_segment(std::move(segment)) {}

  bool operator()(const std::string &next_segment) override {
    return m_segment == next_segment;
  }

 private:
  std::string m_segment;
};

template<typename TParam>
class param_route_match : public route_match {
 public:
  typedef typename std::remove_reference<TParam(const std::string&)>::type param_parser_t;

  explicit param_route_match(param_parser_t &&parser) : m_parser(parser) {}

  bool operator()(const std::string &next_segment) override {
    try {
      m_last_parsed_param = m_parser(next_segment);
      return true;
    } catch (std::exception &e) {
      return false;
    }
  }

  TParam get_param() const {
    return m_last_parsed_param;
  }

 private:
  param_parser_t&& m_parser;
  TParam m_last_parsed_param;
};

struct route_map {
  std::shared_ptr<route_match> match;
  std::string method;
  std::shared_ptr<base_route_handler> handler;
};

//template<typename TMatch>
//class static_route_map_builder {
// public:
//  explicit static_route_map_builder(TMatch &&map) : m_match(std::move(map)) {}
//
//  template<typename THandler>
//  static_route_map_builder &get(THandler &&h) {
//    m_map.method = "GET";
//    m_map.handler = make_handler(std::forward<THandler>(h));
//    return *this;
//  }
//
// private:
//  TMatch m_match;
//};

template<typename TParam>
class param_route_map_builder {
 public:
  explicit param_route_map_builder(route_map &&map) : m_map(std::move(map)) {}

  template<typename THandler>
  param_route_map_builder<TParam> &get(THandler &&h) {
    m_map.method = "GET";
    std::weak_ptr<param_route_match<TParam>> m(std::dynamic_pointer_cast<param_route_match<TParam>>(m_map.match));
    m_map.handler = make_handler([m, &h] (const api_request_t &request) {
      return h(request, m.lock()->get_param());
    });
    return *this;
  }

 private:
  route_map &&m_map;
};

static std::string get_next_segment(const std::string &path) {
  auto next_pos = path.find('/', 1);
  if (next_pos == std::string::npos) {
    return path;
  } else {
    return path.substr(0, next_pos - 1);
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

class api_root {
 public:
  auto get(const std::string &path) {
    validate_path(path);

    return [this, path](auto &&h) {
      this->m_routes.push_back([path, h](const api_request_t &request, const std::string &p) {
        auto next_segment = get_next_segment(p);
        if (next_segment != path) {
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
        auto next_segment = get_next_segment(p);
        if (next_segment != path) {
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

  template<typename TParam>
  auto get() {
    static_assert(std::is_function<decltype(parser<TParam>::parse)>::value, "No parser found for type");

    return [this](auto &&h) {
      this->m_routes.push_back([h](const api_request_t &request, const std::string &p) {
        auto next_segment = get_next_segment(p);
        if (next_segment.empty()) {
          return not_found();
        }
        if (request.http_method != "GET") {
          return method_not_allowed();
        }
        TParam param;
        try {
          param = parser<TParam>::parse(next_segment);
        } catch (std::exception &e) {
          return not_found();
        }
        return ok(h(param));
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

  aws::lambda_runtime::invocation_response operator()(const aws::lambda_runtime::invocation_request &request) {
    api_request_t gpr = aws_lambda_cpp::json::deserialize<api_request_t>(request.payload);
    api_response_t response = (*this)(gpr, gpr.path);
    auto response_json = aws_lambda_cpp::json::serialize(response, true);
    return aws::lambda_runtime::invocation_response::success(response_json, "application/json");
  }

 private:
  std::vector<std::function<api_response_t(const api_request_t&, const std::string&)>> m_routes;
};

}