//
// Created by Daniil Ryzhkov on 22/06/2024.
//

#pragma once

#include "client.hpp"

namespace repository {

struct t_receipt_repository {};

template<typename TRepository = t_client>
class receipt_repository {
 public:
  explicit receipt_repository(TRepository repository) : m_repository(std::move(repository)) {}

  lambda::nullable<models::receipt> get(const std::string &user_id, const std::string &image_name) {
    auto receipt = m_repository->template select<models::receipt>(
            "select * from receipts where user_id = ? and image_name = ?")
        .with_param(user_id)
        .with_param(image_name)
        .first_or_default();

    if (!receipt) {
      return {};
    }

    return assemble_model(receipt);
  }

  lambda::nullable<models::receipt> get(const models::guid &receipt_id) {
    auto receipt = m_repository->template get<models::receipt>(receipt_id);
    if (!receipt) {
      return {};
    }
    if (receipt->is_deleted) {
      return {};
    }
    return assemble_model(receipt);
  }

  std::vector<models::receipt> get_by_month(const models::guid &user_id, int year, int month) {
    auto receipts = m_repository->template select<models::receipt>(
            "select * from receipts "
            "where user_id = ? and year(date) = ? and month(date) = ? and is_deleted = 0 "
            "order by date desc")
        .with_param(user_id)
        .with_param(year)
        .with_param(month)
        .all();

    auto receipt_items = m_repository->template select<models::receipt_item>(
            "select ri.* from receipt_items ri "
            "join receipts r on ri.receipt_id = r.id "
            "where r.user_id = ? and year(r.date) = ? and month(r.date) = ? and r.is_deleted = 0")
        .with_param(user_id)
        .with_param(year)
        .with_param(month)
        .all();

    return assemble_models(receipts, receipt_items);
  }

  void store(const models::receipt &receipt) {
    m_repository->execute("start transaction").go();

    auto existing_receipt = m_repository->template select<models::receipt>(
            "select * from receipts where id = ?")
        .with_param(receipt.id)
        .first_or_default();
    if (!existing_receipt) {
      m_repository->template create<models::receipt>(receipt);
    } else {
      if (existing_receipt->is_deleted) {
        throw concurrency_exception();
      }
      m_repository->template update<models::receipt>(receipt);
    }

    m_repository->execute("delete from receipt_items where receipt_id = ?")
        .with_param(receipt.id)
        .go();

    for (int i = 0; i < receipt.items.size(); i++) {
      auto item = receipt.items[i];
      item.sort_order = i;
      m_repository->template create<models::receipt_item>(item);
    }

    m_repository->execute("commit").go();
  }

  void drop(const models::receipt &receipt) {
    auto existing_receipt = m_repository->template get<models::receipt>(receipt.id);
    if (!existing_receipt) {
      return;
    }
    existing_receipt->is_deleted = true;
    existing_receipt->version = receipt.version + 1;
    m_repository->template update<models::receipt>(*existing_receipt);
  }

  std::vector<models::receipt> get_changed(const models::guid &user_id, const std::string &since) {
    auto receipts = m_repository->template select<models::receipt>(
            "select * from receipts where user_id = ? and modified_timestamp > ?")
        .with_param(user_id)
        .with_param(since)
        .all();

    auto receipt_items = m_repository->template select<models::receipt_item>(
            "select ri.* from receipt_items ri "
            "join receipts r on ri.receipt_id = r.id "
            "where r.user_id = ? and r.modified_timestamp > ?")
        .with_param(user_id)
        .with_param(since)
        .all();

    return assemble_models(receipts, receipt_items);
  }

 private:
  TRepository m_repository;

  models::receipt assemble_model(const std::shared_ptr<models::receipt> &receipt) {
    auto output = *receipt;

    auto items = m_repository->template select<models::receipt_item>(
            "select * from receipt_items where receipt_id = ? order by sort_order")
        .with_param(receipt->id)
        .all();

    for (const auto &item : *items) {
      output.items.push_back(*item);
    }

    return output;
  }

  std::vector<models::receipt> assemble_models(
      const std::shared_ptr<std::vector<std::shared_ptr<models::receipt>>> &receipts,
      const std::shared_ptr<std::vector<std::shared_ptr<models::receipt_item>>> &receipt_items) {
    std::vector<models::receipt> output;
    output.reserve(receipts->size());
    for (const auto &receipt : *receipts) {
      output.push_back(*receipt);
      for (const auto &item : *receipt_items) {
        if (item->receipt_id != receipt->id) continue;
        output.back().items.push_back(*item);
      }
    }

    return output;
  }
};

}
