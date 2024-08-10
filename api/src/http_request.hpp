//
// Created by Daniil Ryzhkov on 10/08/2024.
//

#pragma once

#include <lambda/models/payloads/gateway_proxy.hpp>

namespace api {

struct http_request {
  lambda::models::payloads::base_gateway_proxy_request current;
};

}
