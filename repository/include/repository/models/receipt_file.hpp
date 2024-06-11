//
// Created by Daniil Ryzhkov on 09/06/2024.
//

#pragma once

#include "common.hpp"

namespace repository {
namespace models {

struct receipt_file {
  guid id;
  guid receipt_id;
  std::string file_name;
  int doc_number;
};

}
}
