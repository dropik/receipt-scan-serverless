//
// Created by Daniil Ryzhkov on 08/06/2024.
//

#include "receipt_service.hpp"
#include "rest/api_exception.hpp"
#include "../api_errors.h"

#include <utility>

using namespace api::models;
using namespace repository::models;

namespace api {
namespace services {

receipt_service::receipt_service(std::shared_ptr<repository::client> repository,
                                 models::identity identity,
                                 std::shared_ptr<file_service> file_service)
    : m_repository(std::move(repository)), m_identity(std::move(identity)), m_file_service(std::move(file_service)) {}

std::vector<receipt_list_item> receipt_service::get_receipts() {
  auto receipts = m_repository->select<receipt>(
          "select * from receipts where user_id = ? "
          "order by date desc")
      .with_param(m_identity.user_id)
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
    response.push_back(item);
  }

  return response;
}

models::receipt_detail receipt_service::get_receipt(const guid_t &receipt_id) {
  auto receipt = try_get_receipt(receipt_id);
  auto receipt_items = m_repository->select<receipt_item>(
          "select * from receipt_items ri "
          "join receipts r on r.id = ri.receipt_id "
          "where r.user_id = ? and r.id = ? "
          "order by r.date desc, ri.sort_order asc")
      .with_param(m_identity.user_id)
      .with_param(receipt_id)
      .all();

  receipt_detail rr;
  rr.id = receipt->id;
  rr.date = receipt->date;
  rr.total_amount = receipt->total_amount;
  rr.currency = receipt->currency;
  rr.store_name = receipt->store_name;
  rr.category = receipt->category;

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

models::file receipt_service::get_receipt_file(const guid_t &receipt_id) {
  auto receipt = try_get_receipt(receipt_id);
  return m_file_service->get_download_file_url(receipt->file_name);
}

std::shared_ptr<receipt> receipt_service::try_get_receipt(const guid_t &receipt_id) {
  auto r = m_repository->select<receipt>(
          "select * from receipts where id = ?")
      .with_param(receipt_id)
      .first_or_default();

  if (!r) {
    throw rest::api_exception(not_found, "Receipt not found");
  }
  return r;
}

}
}
