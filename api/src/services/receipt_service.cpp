//
// Created by Daniil Ryzhkov on 08/06/2024.
//

#include "receipt_service.hpp"

#include <utility>

using namespace api::models;
using namespace repository::models;

namespace api {
namespace services {

receipt_service::receipt_service(std::shared_ptr<repository::client> repository,
                                 models::identity identity,
                                 std::shared_ptr<file_service> file_service)
    : m_repository(std::move(repository)), m_identity(std::move(identity)), m_file_service(std::move(file_service)) {}

std::vector<receipt_response> receipt_service::get_receipts() {
  auto receipts = m_repository->select<receipt>(
          "select * from receipts where user_id = ? "
          "order by date desc")
      .with_param(m_identity.user_id)
      .all();

  auto receipt_items = m_repository->select<receipt_item>(
          "select * from receipt_items ri "
          "join receipts r on r.id = ri.receipt_id "
          "where r.user_id = ? "
          "order by r.date desc, ri.sort_order asc")
      .with_param(m_identity.user_id)
      .all();

  std::vector<receipt_response> response;
  for (const auto &r : *receipts) {
    receipt_response rr;
    rr.id = r->id;
    rr.date = r->date;
    rr.total_amount = r->total_amount;
    rr.currency = r->currency;
    rr.store_name = r->store_name;
    rr.category = r->category;
    for (const auto &ri : *receipt_items) {
      if (ri->receipt_id == r->id) {
        receipt_item_response rir;
        rir.id = ri->id;
        rir.description = ri->description;
        rir.amount = ri->amount;
        rir.category = ri->category;
        rr.items.push_back(rir);
      }
    }

    rr.file_url = m_file_service->get_download_file_url(r->request_id).url;

    response.push_back(rr);
  }

  return response;
}

}
}
