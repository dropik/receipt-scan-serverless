//
// Created by Daniil Ryzhkov on 19/10/2024.
//

#include <utility>

#include <aws/ssm/model/GetParameterRequest.h>
#include <lambda/string_utils.hpp>

#include "di/parameter_manager.hpp"

using namespace di;
using namespace lambda;

parameter_manager::parameter_manager(
    const std::string &stage,
    const Aws::Client::ClientConfiguration &client_configuration) {
  m_ssm = Aws::SSM::SSMClient(client_configuration);
  m_prefix = string::format("/receipt-scan/%s", stage.c_str());
}

std::string parameter_manager::get(const std::string &name, const std::string &env_fallback) {
  auto conn_env = getenv(env_fallback.c_str());
  if (conn_env != nullptr) {
    return conn_env;
  }

  Aws::SSM::Model::GetParameterRequest req;
  req.WithName(string::format("%s/%s", m_prefix.c_str(), name.c_str()))
      .WithWithDecryption(true);
  Aws::SSM::Model::GetParameterOutcome outcome = m_ssm.GetParameter(req);
  if (!outcome.IsSuccess()) {
    throw std::runtime_error(
        string::format("Error occurred while obtaining parameter from ssm: %s",
                       outcome.GetError().GetMessage().c_str()));
  }
  return outcome.GetResult().GetParameter().GetValue();
}
