#pragma once

#include "common/base_repository_configuration.hpp"

namespace scanner {
namespace repository {

template <typename T>
class repository_configuration;

template <typename T>
class repository_configuration : public common::base_repository_configuration<T> {
 public:
  repository_configuration() {
    throw std::runtime_error("No configuration is implemented for model");
  }
};

}  // namespace repository
}  // namespace scanner