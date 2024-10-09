//
// Created by Daniil Ryzhkov on 28/09/2024.
//

#include "mock_cognito_idp_client.hpp"

using namespace api::integration_tests::mocks;

mock_cognito_idp_client::mock_cognito_idp_client(const Aws::Client::ClientConfiguration &client_configuration) {}

Aws::CognitoIdentityProvider::Model::AdminGetUserOutcome mock_cognito_idp_client::AdminGetUser(const Aws::CognitoIdentityProvider::Model::AdminGetUserRequest &request) const {
  return Aws::CognitoIdentityProvider::Model::AdminGetUserOutcome(
      Aws::CognitoIdentityProvider::Model::AdminGetUserResult().WithUsername("test_user")
  );
}

Aws::CognitoIdentityProvider::Model::AdminDeleteUserOutcome mock_cognito_idp_client::AdminDeleteUser(const Aws::CognitoIdentityProvider::Model::AdminDeleteUserRequest &request) const {
  return Aws::CognitoIdentityProvider::Model::AdminDeleteUserOutcome(Aws::NoResult());
}
