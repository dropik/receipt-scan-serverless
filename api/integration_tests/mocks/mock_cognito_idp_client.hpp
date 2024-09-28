//
// Created by Daniil Ryzhkov on 28/09/2024.
//

#pragma once

#include <aws/cognito-idp/CognitoIdentityProviderServiceClientModel.h>

namespace api::integration_tests::mocks {

class mock_cognito_idp_client {
 public:
  mock_cognito_idp_client(const Aws::Client::ClientConfiguration &client_configuration);

  Aws::CognitoIdentityProvider::Model::AdminGetUserOutcome AdminGetUser (const Aws::CognitoIdentityProvider::Model::AdminGetUserRequest &request) const;

  Aws::CognitoIdentityProvider::Model::AdminDeleteUserOutcome AdminDeleteUser (const Aws::CognitoIdentityProvider::Model::AdminDeleteUserRequest &request) const;
};

}
