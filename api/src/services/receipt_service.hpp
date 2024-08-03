//
// Created by Daniil Ryzhkov on 08/06/2024.
//

#pragma once

#include <aws/s3/S3Client.h>
#include <repository/client.hpp>
#include <lambda/string_utils.hpp>

#include "../identity.hpp"
#include "../responses/receipt_detail.hpp"
#include "../responses/receipt_list_item.hpp"
#include "../parameters/put_receipt.hpp"

#include "file_service.hpp"

namespace api {
namespace services {

class t_receipt_service {};

template<
    typename TRepository = repository::t_client,
    typename TIdentity = const identity,
    typename TFileService = t_file_service>
class receipt_service {
 public:
  using receipt = repository::models::receipt;
  using receipt_item = repository::models::receipt_item;
  using receipt_file = repository::models::receipt_file;
  using receipt_list_item = responses::receipt_list_item;
  using receipt_detail = responses::receipt_detail;
  using receipt_item_detail = responses::receipt_item_detail;

  receipt_service(TRepository repository, TIdentity identity, TFileService file_service)
      : m_repository(std::move(repository)),
        m_identity(std::move(identity)),
        m_file_service(std::move(file_service)) {}

  std::vector<responses::receipt_list_item> get_receipts() {
    auto receipts = m_repository->template select<receipt>(
            "select * from receipts where user_id = ? "
            "order by date desc")
        .with_param(m_identity->user_id)
        .all();

    auto receipt_items = m_repository->template select<receipt_item>(
            "select * from receipt_items ri "
            "join receipts r on r.id = ri.receipt_id "
            "where r.user_id = ? "
            "order by r.date desc, ri.sort_order asc, ri.amount desc")
        .with_param(m_identity->user_id)
        .all();

    std::vector<receipt_list_item> response;
    for (const auto &r : *receipts) {
      receipt_list_item item;
      item.id = r->id;
      item.date = r->date;
      item.total_amount = r->total_amount;
      item.currency = r->currency;
      item.store_name = r->store_name;

      item.category = r->category;
      if (item.category.empty()) {
        std::set<std::string> categories;
        for (const auto &ri : *receipt_items) {
          if (ri->receipt_id == r->id) {
            categories.insert(ri->category);
          }
        }
        item.category = lambda::string::join(", ", categories);
      }

      item.state = r->state;
      response.push_back(item);
    }

    return response;
  }

  responses::receipt_detail get_receipt(const guid_t &receipt_id) {
    auto receipt = get_receipt_by_id(receipt_id);
    auto receipt_items = m_repository->template select<receipt_item>(
            "select * from receipt_items ri "
            "join receipts r on r.id = ri.receipt_id "
            "where r.user_id = ? and r.id = ? "
            "order by r.date desc, ri.sort_order asc")
        .with_param(m_identity->user_id)
        .with_param(receipt_id)
        .all();

    receipt_detail rr;
    rr.id = receipt->id;
    rr.date = receipt->date;
    rr.total_amount = receipt->total_amount;
    rr.currency = receipt->currency;
    rr.store_name = receipt->store_name;

    rr.category = receipt->category;
    if (rr.category.empty()) {
      std::set<std::string> categories;
      for (const auto &ri : *receipt_items) {
        categories.insert(ri->category);
      }
      rr.category = lambda::string::join(", ", categories);
    }

    rr.state = receipt->state;

    for (const auto &ri : *receipt_items) {
      receipt_item_detail rir;
      rir.id = ri->id;
      rir.description = ri->description;
      rir.amount = ri->amount;
      rir.category = ri->category;
      rr.items.push_back(rir);
    }

    return rr;
  }

  responses::file get_receipt_file(const guid_t &receipt_id) {
    auto file = try_get_receipt_file(receipt_id);
    if (!file) {
      throw rest::api_exception(not_found, "Receipt file not found");
    }
    return m_file_service->get_download_file_url(file->file_name);
  }

  void put_receipt(const parameters::put_receipt &params) {
    receipt r;
    r.id = params.id;
    r.user_id = m_identity->user_id;
    r.date = params.date;
    r.total_amount = params.total_amount;
    r.currency = params.currency;
    r.store_name = params.store_name;

    r.category = params.category;
    if (!params.items.empty() && !params.category.empty()) {
      throw rest::api_exception(invalid_argument, "Category should be empty if items are provided");
    }

    r.state = receipt::done;

    auto existing_receipt = try_get_receipt(params.id);
    if (!existing_receipt) {
      m_repository->create(r);
    } else {
      if (existing_receipt->user_id != m_identity->user_id) {
        throw rest::api_exception(forbidden, "Access denied");
      }
      if (existing_receipt->state == receipt::processing) {
        throw rest::api_exception(forbidden, "Receipt is being processed");
      }
      m_repository->update(r);
      m_repository->execute("delete from receipt_items where receipt_id = ?").with_param(r.id).go();
    }

    for (int i = 0; i < params.items.size(); i++) {
      const auto &ri = params.items[i];
      receipt_item rii;
      rii.id = ri.id;
      rii.receipt_id = r.id;
      rii.description = ri.description;
      rii.amount = ri.amount;
      rii.category = ri.category;
      rii.sort_order = i;
      m_repository->create(rii);
    }
  }

  void delete_receipt(const guid_t &receipt_id) {
    auto receipt = get_receipt_by_id(receipt_id);
    auto file = try_get_receipt_file(receipt_id);
    m_repository->drop(*receipt);
    if (file) {
      m_file_service->delete_file(file->file_name);
    }
  }

 private:
  TRepository m_repository;
  TIdentity m_identity;
  TFileService m_file_service;

  std::shared_ptr<repository::models::receipt> try_get_receipt(const guid_t &receipt_id) {
    return m_repository->template select<receipt>(
            "select * from receipts where id = ?")
        .with_param(receipt_id)
        .first_or_default();
  }
  std::shared_ptr<repository::models::receipt> get_receipt_by_id(const guid_t &receipt_id) {
    auto receipt = try_get_receipt(receipt_id);
    if (!receipt || receipt->user_id != m_identity->user_id) {
      throw rest::api_exception(not_found, "Receipt not found");
    }
    return receipt;
  }
  std::shared_ptr<repository::models::receipt_file> try_get_receipt_file(const guid_t &receipt_id) {
    auto rf = m_repository->template select<receipt_file>(
            "select * from receipt_files where receipt_id = ?")
        .with_param(receipt_id)
        .first_or_default();
    return rf;
  }
};

}
}
