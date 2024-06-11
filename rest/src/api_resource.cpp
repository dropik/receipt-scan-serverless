//
// Created by Daniil Ryzhkov on 02/06/2024.
//

#include <rest/api_resource.hpp>

namespace rest {

api_response_t api_resource::route(const api_request_t &request, const std::string &p) {
  if (m_routes.empty()) {
    return not_found();
  }

  // sanitizing trailing slashes
  auto path = p;
  if (path.size() > 1 && path[path.size() - 1] == '/') {
    path = path.substr(0, path.size() - 1);
  }

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

}