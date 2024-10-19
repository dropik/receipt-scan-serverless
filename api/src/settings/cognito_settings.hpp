//
// Created by Daniil Ryzhkov on 28/09/2024.
//

#pragma once

#include <string>

namespace api::settings {

struct cognito_settings {
    cognito_settings() = default;

    explicit cognito_settings(std::string user_pool_id)
        : user_pool_id(std::move(user_pool_id)) {}

    std::string user_pool_id;
};

}
