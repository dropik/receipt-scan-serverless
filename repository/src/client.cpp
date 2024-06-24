#include "repository/client.hpp"

#include <aws/ssm/SSMClient.h>

#include <lambda/string_utils.hpp>
#include <aws/ssm/model/GetParameterRequest.h>

using namespace repository;
using namespace lambda;

std::string repository::get_connection_string(const std::string &stage, const Aws::Client::ClientConfiguration &config) {
  auto conn_env = getenv("DB_CONNECTION_STRING");
  if (conn_env != nullptr) {
    return conn_env;
  }

  std::string ssmPrefix = string::format("/receipt-scan/%s", stage.c_str());
  Aws::SSM::SSMClient ssmClient(config);
  Aws::SSM::Model::GetParameterRequest connStrReq;
  connStrReq
      .WithName(string::format("%s/db-connection-string", ssmPrefix.c_str()))
      .WithWithDecryption(true);
  Aws::SSM::Model::GetParameterOutcome outcome =
      ssmClient.GetParameter(connStrReq);
  if (!outcome.IsSuccess()) {
    throw std::runtime_error(
        string::format("Error occurred while obtaining parameter from ssm: %s",
                   outcome.GetError().GetMessage().c_str()));
  }
  return outcome.GetResult().GetParameter().GetValue();
}
