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

class http_client {
 public:
  http_client() : m_client(Aws::Client::ClientConfiguration{}) {}

  template<typename TResult>
  outcome<TResult> post(const std::string &url,
                        const std::string &body,
                        const std::string &content_type = content_type::json) {
    auto request = std::make_shared<http_request>(uri(url), http_method::HTTP_POST);
    request->SetContentType(content_type);
    auto body_stream = std::make_shared<std::stringstream>(body.c_str());
    request->AddContentBody(body_stream);

    auto response = m_client.MakeRequest(request);
    auto &response_stream = response->GetResponseBody();
    std::string response_body((std::istreambuf_iterator<char>(response_stream)), {});

    if (response->GetResponseCode() == http_status::OK) {
      return outcome<TResult>(lambda::json::deserialize<TResult>(response_body));
    } else {
      return outcome<TResult>((int) response->GetResponseCode(), response_body);
    }
  }

 private:
  Aws::Http::CurlHttpClient m_client;

  using http_request = Aws::Http::Standard::StandardHttpRequest;
  using uri = Aws::Http::URI;
  using http_method = Aws::Http::HttpMethod;
  using http_status = Aws::Http::HttpResponseCode;
};

}
