//
// Created by Daniil Ryzhkov on 03/08/2024.
//

#pragma once

#include <exception>

namespace repository {

struct concurrency_exception : public std::exception {};
struct entity_not_found_exception : public std::exception {};

}
