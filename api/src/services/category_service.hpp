//
// Created by Daniil Ryzhkov on 09/06/2024.
//

#pragma once

#include <repository/client.hpp>
#include "../models/identity.hpp"
#include "../models/category.hpp"

namespace api {
namespace services {

class category_service {
 public:
  category_service(std::shared_ptr<repository::client> repository, const models::identity &identity);

  std::vector<models::category> get_categories();
  void put_category(const models::category &category);
  void delete_category(const models::guid_t &category_id);

 private:
  std::shared_ptr<repository::client> m_repository;
  const models::identity &m_identity;

  std::shared_ptr<repository::models::category> try_get_category(const models::guid_t &category_id);
};

}
}