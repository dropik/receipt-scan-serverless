//
// Created by Daniil Ryzhkov on 08/06/2024.
//

#pragma once

#include <aws/s3/S3Client.h>
#include "repository/client.hpp"
#include "../models/identity.hpp"
#include "../models/receipt_response.hpp"
#include "file_service.hpp"

namespace api {
namespace services {

class receipt_service {
 public:
  receipt_service(std::shared_ptr<repository::client> repository,
                  models::identity identity,
                  std::shared_ptr<file_service> file_service);

  std::vector<models::receipt_response> get_receipts();

 private:
  std::shared_ptr<repository::client> m_repository;
  const models::identity m_identity;
  std::shared_ptr<file_service> m_file_service;
};

}
}
