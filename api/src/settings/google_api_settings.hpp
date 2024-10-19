//
// Created by Daniil Ryzhkov on 19/10/2024.
//

#pragma once

#include <string>
#include <utility>

namespace api::settings {

struct google_api_settings {
  std::string private_key_id;
  std::string private_key;
  std::string client_email;

  google_api_settings() = default;

  google_api_settings(std::string private_key_id, std::string private_key, std::string client_email)
      : private_key_id(std::move(private_key_id)),
        private_key(std::move(private_key)),
        client_email(std::move(client_email)) {}
};

}
