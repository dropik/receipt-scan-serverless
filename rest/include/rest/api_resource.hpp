//
// Created by Daniil Ryzhkov on 02/06/2024.
//

#pragma once

#include <utility>

#include "api_exception.hpp"
#include "types.hpp"
#include "utils.hpp"
#include "responses.hpp"
#include "parsing.hpp"

namespace rest {

class api_resource {
 public:
  auto get(const std::string &path) {
    rest::validate_path(path);

    return [this, path](const auto &&h) {
      this->m_routes.push_back([path, h](const rest::api_request_t &request, const std::string &p) {
        if (p != path) {
          return rest::not_found();
        }
        if (request.http_method != "GET") {
          return rest::method_not_allowed();
        }
        return rest::ok(h());
      });
    };
  }

  template<typename TBody>
  auto post(const std::string &path) {
    rest::validate_path(path);

    return [this, path](const auto &&h) {
      this->m_routes.push_back([path, h](const rest::api_request_t &request, const std::string &p) {
        if (p != path) {
          return rest::not_found();
        }
        if (request.http_method != "POST") {
          return rest::method_not_allowed();
        }
        TBody body;
        try {
          body = lambda::json::deserialize<TBody>(request.get_body());
        } catch (std::exception &e) {
          return rest::bad_request();
        }

        using THandler = decltype(h);
        return post_response<THandler, TBody>()(request, std::forward<THandler>(h), body);
      });
    };
  }

  auto post(const std::string &path) {
    rest::validate_path(path);

    return [this, path](const auto &&h) {
      this->m_routes.push_back([path, h](const rest::api_request_t &request, const std::string &p) {
        if (p != path) {
          return rest::not_found();
        }
        if (request.http_method != "POST") {
          return rest::method_not_allowed();
        }

        using THandler = decltype(h);
        return post_response<THandler>()(request, std::forward<THandler>(h));
      });
    };
  }

  template<typename TBody>
  auto put(const std::string &path) {
    validate_path(path);

    return [this, path](const auto &&h) {
      this->m_routes.push_back([path, h](const api_request_t &request, const std::string &p) {
        if (p != path) {
          return not_found();
        }
        if (request.http_method != "PUT") {
          return method_not_allowed();
        }
        TBody body;
        try {
          body = lambda::json::deserialize<TBody>(request.get_body());
        } catch (std::exception &e) {
          return bad_request();
        }

        h(body);
        return ok();
      });
    };
  }

  template<typename TBody>
  auto patch(const std::string &path) {
    validate_path(path);

    return [this, path](const auto &&h) {
      this->m_routes.push_back([path, h](const api_request_t &request, const std::string &p) {
        if (p != path) {
          return not_found();
        }
        if (request.http_method != "PATCH") {
          return method_not_allowed();
        }
        TBody body;
        try {
          body = lambda::json::deserialize<TBody>(request.get_body());
        } catch (std::exception &e) {
          return bad_request();
        }

        h(body);
        return ok();
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
        return nested.route(request, nested_path);
      });
    };
  }

  template<typename TParam>
  auto get() {
    using TRawParam = typename std::decay<TParam>::type;
    static_assert(std::is_function<decltype(parser<TRawParam>::parse)>::value, "No parser found for type");

    return [this](const auto &&h) {
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
  auto del() {
    using TRawParam = typename std::decay<TParam>::type;
    static_assert(std::is_function<decltype(parser<TRawParam>::parse)>::value, "No parser found for type");

    return [this](const auto &&h) {
      this->m_routes.push_back([h](const api_request_t &request, const std::string &p) {
        auto next_segment = get_next_segment(p);
        if (next_segment.empty()) {
          return not_found();
        }
        if (next_segment.size() != p.size()) {
          // this is indeed not found, because a non ANY route should not have any segments after the path
          return not_found();
        }
        if (request.http_method != "DELETE") {
          return method_not_allowed();
        }
        TRawParam param;
        try {
          param = parser<TRawParam>::parse(next_segment);
        } catch (std::exception &e) {
          return not_found();
        }
        h(param);
        return ok();
      });
    };
  }

  auto del(const std::string &path) {
    validate_path(path);

    return [this, path](const auto &&h) {
      this->m_routes.push_back([h, path](const api_request_t &request, const std::string &p) {
        auto next_segment = get_next_segment(p);

        if (next_segment != path) {
          return not_found();
        }

        if (next_segment.size() != p.size()) {
          // this is indeed not found, because a non ANY route should not have any segments after the path
          return not_found();
        }

        if (request.http_method != "DELETE") {
          return method_not_allowed();
        }

        h();
        return ok();
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
        return nested.route(request, nested_path);
      });
    };
  }

  api_response_t route(const api_request_t &request, const std::string &p);

 private:
  std::vector<std::function<api_response_t(const api_request_t &, const std::string &)>> m_routes;
};

}