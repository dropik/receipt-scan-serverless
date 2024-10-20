//
// Created by Daniil Ryzhkov on 19/10/2024.
//

#pragma once

#include <optional>

#include <aws/core/http/curl/CurlHttpClient.h>
#include <aws/core/http/standard/StandardHttpRequest.h>
#include <aws/core/http/HttpResponse.h>

#include <lambda/json.hpp>

namespace api::services {

template<typename T>
struct outcome {
  bool is_success;
  int status_code = 0;
  std::optional<std::string> error;
  T result;

  outcome() : is_success(false), result(T{}) {}
  explicit outcome(const T &result) : is_success(true), status_code(200), result(result) {}
  explicit outcome(const int status_code, const std::string &error)
      : is_success(false), status_code(status_code), error(error) {}
};

struct content_type {
  static inline const std::string json = "application/json";
  static inline const std::string form = "application/x-www-form-urlencoded";
};

struct no_result {};

class http_client {
 public:
  http_client() : m_client(Aws::Client::ClientConfiguration{}) {}

  template<typename TResult>
  outcome<TResult> get(const std::string &url) {
    return make_request<TResult>(url, {}, {}, http_method::HTTP_GET);
  }

  template<typename TResult>
  outcome<TResult> post(const std::string &url,
                        const std::string &body,
                        const std::string &content_type = content_type::json) {
    return make_request<TResult>(url, body, content_type, http_method::HTTP_POST);
  }

  void set_bearer_authorization(const std::string &token) {
    m_authorization = "Bearer " + token;
  }

 private:
  Aws::Http::CurlHttpClient m_client;

  using http_request = Aws::Http::Standard::StandardHttpRequest;
  using uri = Aws::Http::URI;
  using http_method = Aws::Http::HttpMethod;
  using http_status = Aws::Http::HttpResponseCode;

  std::optional<std::string> m_authorization;

  template<typename TResult>
  outcome<TResult> make_request(const std::string &url,
                                const std::optional<std::string> &body,
                                const std::optional<std::string> &content_type,
                                const http_method &method) {
    auto request = std::make_shared<http_request>(uri(url), method);

    if (content_type.has_value()) {
      request->SetContentType(content_type.value());
    }

    if (body.has_value()) {
      auto body_stream = std::make_shared<std::stringstream>(body.value().c_str());
      request->AddContentBody(body_stream);
    }

    if (m_authorization.has_value()) {
      request->SetAuthorization(m_authorization.value());
    }

    auto response = m_client.MakeRequest(request);
    auto &response_stream = response->GetResponseBody();
    std::string response_body((std::istreambuf_iterator<char>(response_stream)), {});

    if (response->GetResponseCode() == http_status::OK) {
      if constexpr (std::is_same_v<TResult, no_result>) {
        return outcome<TResult>(no_result{});
      } else {
        return outcome<TResult>(lambda::json::deserialize<TResult>(response_body));
      }
    } else {
      return outcome<TResult>((int) response->GetResponseCode(), response_body);
    }
  }
};

}
