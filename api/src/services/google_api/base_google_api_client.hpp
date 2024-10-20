//
// Created by Daniil Ryzhkov on 20/10/2024.
//

#pragma once

#include "google_api_auth_provider.hpp"
namespace api::services::google_api {

class base_google_api_client {
 protected:
  explicit base_google_api_client(std::shared_ptr<google_api_auth_provider> auth)
      : m_auth(std::move(auth)) {}

  template<typename TResult>
  outcome<TResult> get(const std::string &url) {
    authenticate();
    return m_client.get<TResult>(url);
  }

  template<typename TResult>
  outcome<TResult> post(const std::string &url, const std::string &body) {
    authenticate();
    return m_client.post<TResult>(url, body);
  }

 private:
  http_client m_client;
  std::shared_ptr<google_api_auth_provider> m_auth;

  void authenticate() {
    auto access_token = m_auth->get_access_token();
    m_client.set_bearer_authorization(access_token);
  }
};

}
