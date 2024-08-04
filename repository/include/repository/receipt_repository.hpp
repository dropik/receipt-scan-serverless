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
    return assemble_model(receipt);
  }

  std::vector<models::receipt> get_by_month(const models::guid &user_id, int year, int month) {
    auto receipts = m_repository->template select<models::receipt>(
            "select * from receipts "
            "where user_id = ? and year(date) = ? and month(date) = ? "
            "order by date desc")
        .with_param(user_id)
        .with_param(year)
        .with_param(month)
        .all();

    auto receipt_items = m_repository->template select<models::receipt_item>(
            "select ri.* from receipt_items ri "
            "join receipts r on ri.receipt_id = r.id "
            "where r.user_id = ? and year(r.date) = ? and month(r.date) = ?")
        .with_param(user_id)
        .with_param(year)
        .with_param(month)
        .all();

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

  void store(const models::receipt &receipt) {
    auto existing_receipt = m_repository->template select<models::receipt>(
            "select * from receipts where id = ?")
        .with_param(receipt.id)
        .first_or_default();
    if (!existing_receipt) {
      m_repository->template create<models::receipt>(receipt);
    } else {
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
  }

  void drop(const models::receipt &receipt) {
    m_repository->drop(receipt);
  }

 private:
  TRepository m_repository;

  models::receipt assemble_model(const std::shared_ptr<models::receipt>& receipt) {
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
};

}
