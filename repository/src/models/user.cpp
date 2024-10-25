//
// Created by Daniil Ryzhkov on 25/10/2024.
//

#include <ctime>
#include <iomanip>
#include <sstream>

#include <repository/models/user.hpp>

using namespace repository::models;

bool user::verify_subscription() const {
  if (!this->has_subscription) return false;
  if (!this->subscription_expiry_time.has_value()) return false;

  auto expiry_timestamp = this->subscription_expiry_time.get_value();
  std::tm tm = {};
  std::stringstream ss(expiry_timestamp.c_str());
  ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
  auto now = std::time(nullptr);
  return std::difftime(timegm(&tm), now) >= 0;
}
