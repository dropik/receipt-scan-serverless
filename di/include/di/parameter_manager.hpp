//
// Created by Daniil Ryzhkov on 19/10/2024.
//

#pragma once

#include <aws/ssm/SSMClient.h>

namespace di {

class parameter_manager {
 public:
  parameter_manager() = default;
  parameter_manager(const std::string &stage, const Aws::Client::ClientConfiguration &client_configuration);
  std::string get(const std::string &name, const std::string &env_fallback);

 private:
  Aws::SSM::SSMClient m_ssm;
  std::string m_prefix;
};

}
