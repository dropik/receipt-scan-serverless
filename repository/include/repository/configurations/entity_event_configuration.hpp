//
// Created by Daniil Ryzhkov on 31/07/2024.
//

#pragma once

#include "repository/models/entity_event.hpp"
#include "repository_configuration.hpp"

namespace repository {
namespace configurations {

template<>
class repository_configuration<models::entity_event>
    : public common::base_repository_configuration<models::entity_event> {
 public:
  repository_configuration() {
    HAS_TABLE("entity_events");

    HAS_ID(id) WITH_COLUMN("id");

    HAS_STRING(device_id) WITH_COLUMN("device_id");
    HAS_STRING(entity_type) WITH_COLUMN("entity_type");
    HAS_STRING(entity_id) WITH_COLUMN("entity_id");
    HAS_STRING(event_type) WITH_COLUMN("event_type");
    HAS_STRING(event_timestamp) WITH_COLUMN("event_timestamp");
  }
};

}
}
