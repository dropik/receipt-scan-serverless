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

  lambda::nullable<models::receipt> get(const std::string &user_id, const std::string &file_name, int doc_number) {
    auto file = m_repository->template select<models::receipt_file>(
            "select * from receipt_files rf "
            "join receipts r on r.id = rf.receipt_id "
            "where r.user_id = ? and rf.file_name = ? and rf.doc_number = ?")
        .with_param(user_id)
        .with_param(file_name)
        .with_param(doc_number)
        .first_or_default();

    if (!file) {
      return {};
    }

    auto receipt = m_repository->template select<models::receipt>(
            "select * from receipts where id = ?")
        .with_param(file->receipt_id)
        .first_or_default();

    if (!receipt) {
      return {};
    }

    auto output = *receipt;
    output.file = *file;

    auto items = m_repository->template select<models::receipt_item>(
            "select * from receipt_items where receipt_id = ? order by sort_order")
        .with_param(receipt->id)
        .all();

    for (const auto &item : *items) {
      output.items.push_back(*item);
    }

    return output;
  }

  void store(const models::receipt &receipt) {
    auto existing_receipt = m_repository->template select<models::receipt>(
            "select * from receipts where id = ?")
        .with_param(receipt.id)
        .first_or_default();
    if (!existing_receipt) {
      m_repository->template create<models::receipt>(receipt);
    } else {
      m_repository->execute(
              "delete from receipt_items where receipt_id = ?")
          .with_param(existing_receipt->id)
          .go();
      m_repository->template update<models::receipt>(receipt);
    }

    auto existing_items = m_repository->template select<models::receipt_item>(
            "select * from receipt_items where receipt_id = ?")
        .with_param(receipt.id)
        .all();
    std::vector<std::string> delete_items;
    for (const auto &item : *existing_items) {
      delete_items.push_back(item->id);
    }

    for (int i = 0; i < receipt.items.size(); i++) {
      auto item = receipt.items[i];
      item.sort_order = i;
      auto existing_item_id = std::find(delete_items.begin(), delete_items.end(), item.id);
      if (existing_item_id == delete_items.end()) {
        m_repository->template create<models::receipt_item>(item);
      } else {
        m_repository->template update<models::receipt_item>(item);
        delete_items.erase(existing_item_id);
      }
    }

    for (const auto &item_id : delete_items) {
      m_repository->execute(
              "delete from receipt_items where id = ?")
          .with_param(item_id)
          .go();
    }

    if (!receipt.file.has_value()) {
      m_repository->execute(
              "delete from receipt_files where receipt_id = ?")
          .with_param(receipt.id)
          .go();
    } else {
      auto existing_file = m_repository->template select<models::receipt_file>(
              "select * from receipt_files where id = ?")
          .with_param(receipt.file.get_value().id)
          .first_or_default();
      if (!existing_file) {
        m_repository->execute(
                "delete from receipt_files where receipt_id = ?")
            .with_param(receipt.id)
            .go();
        m_repository->template create<models::receipt_file>(receipt.file.get_value());
      } else {
        m_repository->template update<models::receipt_file>(receipt.file.get_value());
      }
    }
  }

 private:
  TRepository m_repository;
};

}
