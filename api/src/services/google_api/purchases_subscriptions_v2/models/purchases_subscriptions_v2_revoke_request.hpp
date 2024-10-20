//
// Created by Daniil Ryzhkov on 20/10/2024.
//

#pragma once

#include <lambda/json.hpp>

namespace api::services::google_api::purchases_subscriptions_v2::models {

struct full_refund {
  JSON_BEGIN_SERIALIZER(full_refund)
  JSON_END_SERIALIZER()
};

struct prorated_refund {
  JSON_BEGIN_SERIALIZER(prorated_refund)
  JSON_END_SERIALIZER()
};

struct revocation_context {
  lambda::nullable<full_refund> full_refund;
  lambda::nullable<prorated_refund> prorated_refund;

  JSON_BEGIN_SERIALIZER(revocation_context)
      JSON_PROPERTY("fullRefund", full_refund)
      JSON_PROPERTY("proratedRefund", prorated_refund)
  JSON_END_SERIALIZER()
};

struct purchases_subscriptions_v2_revoke_request {
  revocation_context revocation_context;

  JSON_BEGIN_SERIALIZER(purchases_subscriptions_v2_revoke_request)
      JSON_PROPERTY("revocationContext", revocation_context)
  JSON_END_SERIALIZER()
};

}
