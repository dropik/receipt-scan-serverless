//
// Created by Daniil Ryzhkov on 19/10/2024.
//

#pragma once

#include "../parameters/rtdn.hpp"
#include "../parameters/gp_notification.hpp"
#include "../http_request.hpp"
#include "lambda/log.hpp"

namespace api::services {

struct t_rtdn_service {};

template<typename THttpRequest = http_request>
class rtdn_service {
 public:
  explicit rtdn_service(THttpRequest http_request) : m_http_request(std::move(http_request)) {}

  void process_message(const parameters::rtdn<parameters::gp_notification> &message) {
    auto &req = get_http_request();
    auto req_body = req.current.get_body();
    lambda::log.info("Received RTDN message: %s", req_body.c_str());

    auto data = message.message.data;
    lambda::log.info("RTDN message data: %s", data.c_str());

    auto notification = message.get_message();
    auto notification_json = lambda::json::serialize(notification);
    lambda::log.info("RTDN notification: %s", notification_json.c_str());
  }

 private:
  THttpRequest m_http_request;

  [[nodiscard]] const http_request & get_http_request() const {
    return *m_http_request;
  }
};

}
