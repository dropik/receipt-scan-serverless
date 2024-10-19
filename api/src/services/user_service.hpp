//
// Created by Daniil Ryzhkov on 07/06/2024.
//

#pragma once

#include <memory>

#include <aws/cognito-idp/CognitoIdentityProviderClient.h>
#include <aws/cognito-idp/model/AdminGetUserRequest.h>
#include <aws/cognito-idp/model/AdminDeleteUserRequest.h>

#include <repository/client.hpp>
#include "file_service.hpp"

#include "../identity.hpp"
#include "../api_errors.hpp"
#include "../responses/user.hpp"
#include "../settings/cognito_settings.hpp"

namespace api::services {

struct t_user_service {};

template<
    typename TRepository = repository::t_client,
    typename TIdentity = const identity,
    typename TFileService = t_file_service,
    typename TCognitoIDP = Aws::CognitoIdentityProvider::CognitoIdentityProviderClient,
    typename TCognitoSettings = settings::cognito_settings>
class user_service {
  using user = repository::models::user;

 public:
  user_service(TRepository repository, TIdentity identity, TFileService file_service, TCognitoIDP cognito, TCognitoSettings cognito_settings)
      : m_repository(std::move(repository)),
        m_identity(std::move(identity)),
        m_file_service(std::move(file_service)),
        m_cognito(std::move(cognito)),
        m_user_pool_id(cognito_settings->user_pool_id) {}

  void init_user() {
    auto user_id = m_identity->user_id;

    auto existing_user =
        m_repository->template select<user>("select * from users where id = ?")
            .with_param(user_id)
            .first_or_default();

    if (existing_user) return;

    user user{.id = user_id};
    m_repository->create(user);
  }

  responses::user get_user() {
    auto users = m_repository->template select<user>("select * from users where id = ?")
        .with_param(m_identity->user_id)
        .all();
    if (users->empty()) {
      throw rest::api_exception(not_found, "User not found");
    }
    return responses::user::from_repository(*users->at(0));
  }

  void delete_user() {
    delete_data_from_database();
    m_file_service->delete_receipt_images(m_identity->user_id);
    delete_cognito_user();
  }

 private:
  TRepository m_repository;
  TIdentity m_identity;
  TFileService m_file_service;
  TCognitoIDP m_cognito;
  std::string m_user_pool_id;

  void delete_data_from_database() {
    auto user_id = m_identity->user_id;
    try {
      m_repository->execute("delete from receipts where user_id = ?").with_param(user_id).go();
      m_repository->execute("delete from categories where user_id = ?").with_param(user_id).go();
      m_repository->execute("delete from budgets where user_id = ?").with_param(user_id).go();
      m_repository->execute("delete from users where id = ?").with_param(user_id).go();
    } catch (const std::exception &e) {
      lambda::log.error("Failed to delete user data: %s", e.what());
      throw rest::api_exception(internal, "Failed to delete user data");
    }
  }

  void delete_cognito_user() {
    auto user_id = m_identity->user_id;

    // Check if the user exists in the pool
    Aws::CognitoIdentityProvider::Model::AdminGetUserRequest get_user_request;
    get_user_request.SetUserPoolId(m_user_pool_id);
    get_user_request.SetUsername(user_id);

    auto get_user_outcome = m_cognito->AdminGetUser(get_user_request);

    // If the user exists, proceed to delete
    if (get_user_outcome.IsSuccess()) {
      // User exists, proceed to delete
      Aws::CognitoIdentityProvider::Model::AdminDeleteUserRequest delete_user_request;
      delete_user_request.SetUserPoolId(m_user_pool_id);
      delete_user_request.SetUsername(user_id);

      auto delete_user_outcome = m_cognito->AdminDeleteUser(delete_user_request);

      if (!delete_user_outcome.IsSuccess()) {
        lambda::log.error("Failed to delete cognito user: %s", delete_user_outcome.GetError().GetMessage().c_str());
        throw rest::api_exception(internal, "Failed to delete user");
      }
    } else {
      // User doesn't exist, consider it idempotently successful
      if (get_user_outcome.GetError().GetErrorType() == Aws::CognitoIdentityProvider::CognitoIdentityProviderErrors::USER_NOT_FOUND) {
        lambda::log.info("User doesn't exist in cognito, skipping delete");
      } else {
        lambda::log.error("Failed to check user existence: %s", get_user_outcome.GetError().GetMessage().c_str());
        throw rest::api_exception(internal, "Failed to delete user");
      }
    }
  }
};

}
