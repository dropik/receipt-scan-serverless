//
// Created by Daniil Ryzhkov on 08/06/2024.
//

#pragma once

#include <iomanip>

#include <aws/s3/S3Client.h>
#include <lambda/string_utils.hpp>
#include "repository/receipt_repository.hpp"

#include "../identity.hpp"
#include "../responses/receipt.hpp"
#include "../parameters/put_receipt.hpp"

#include "file_service.hpp"
#include "../responses/change.hpp"

namespace api::services {

class t_receipt_service {};

template<
    typename TRepository = repository::t_receipt_repository,
    typename TIdentity = const identity,
    typename TFileService = t_file_service>
class receipt_service {
 public:
  receipt_service(TRepository repository, TIdentity identity, TFileService file_service)
      : m_repository(std::move(repository)),
        m_identity(std::move(identity)),
        m_file_service(std::move(file_service)) {}

  std::vector<responses::receipt> get_receipts(int year, int month) {
    auto results = m_repository->get_by_month(m_identity->user_id, year, month);
    std::vector<responses::receipt> response;
    response.reserve(results.size());
    for (const auto &item : results) {
      response.push_back(responses::receipt::from_repo(item));
    }
    return response;
  }

  responses::file get_receipt_get_image_url(const guid_t &receipt_id) {
    auto r = try_get_receipt(receipt_id);
    return m_file_service->get_download_receipt_image_url(r.image_name);
  }

  responses::file get_receipt_put_image_url(const guid_t &receipt_id) {
    verify_subscription();
    auto r = try_get_receipt(receipt_id);
    return m_file_service->get_upload_receipt_image_url(r.image_name);
  }

  void put_receipt(const parameters::put_receipt &params) {
    auto model = params.to_repo(m_identity->user_id);
    m_repository->store(model);
  }

  void delete_receipt(const guid_t &receipt_id) {
    auto receipt = try_get_receipt(receipt_id);
    m_repository->drop(receipt);
    if (!receipt.image_name.empty()) {
      m_file_service->delete_receipt_image(receipt.image_name);
    }
  }

  std::vector<responses::change<responses::receipt>> get_changes(const std::string &since) {
    auto results = m_repository->get_changed(m_identity->user_id, since);
    std::vector<responses::change<responses::receipt>> response;
    response.reserve(results.size());
    for (const auto &item : results) {
      response.push_back(responses::change<responses::receipt>{
          .action = item.is_deleted
                    ? responses::change_action::del
                    : (item.version == 0
                       ? responses::change_action::create
                       : responses::change_action::update),
          .id = item.id,
          .body = item.is_deleted
                  ? lambda::nullable<responses::receipt>{}
                  : responses::receipt::from_repo(item),
      });
    }
    return response;
  }

 private:
  TRepository m_repository;
  TIdentity m_identity;
  TFileService m_file_service;

  repository::models::receipt try_get_receipt(const guid_t &receipt_id) {
    auto result = m_repository->get(receipt_id);
    if (!result.has_value()) {
      throw rest::api_exception(not_found, "Receipt not found");
    }
    return result.get_value();
  }

  void verify_subscription() const {
    if (!m_identity->has_subscription) throw_subscription();
    if (!m_identity->subscription_expiry_time.has_value()) throw_subscription();

    auto expiry_timestamp = m_identity->subscription_expiry_time.get_value();
    std::tm tm = {};
    std::stringstream ss(expiry_timestamp.c_str());
    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
    auto now = std::time(nullptr);
    if (std::difftime(timegm(&tm), now) < 0) {
      throw_subscription();
    }
  }

  void throw_subscription() const { throw rest::api_exception(forbidden, "Subscription required"); }
};

}
