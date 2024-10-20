//
// Created by Daniil Ryzhkov on 20/10/2024.
//

#pragma once

#include "../../types.hpp"

namespace api::services::google_api::purchases_subscriptions_v2::models {

struct subscription_state {
  STR_ENUM unspecified               = "SUBSCRIPTION_STATE_UNSPECIFIED";
  STR_ENUM pending                   = "SUBSCRIPTION_STATE_PENDING";
  STR_ENUM active                    = "SUBSCRIPTION_STATE_ACTIVE";
  STR_ENUM paused                    = "SUBSCRIPTION_STATE_PAUSED";
  STR_ENUM grace_period              = "SUBSCRIPTION_STATE_IN_GRACE_PERIOD";
  STR_ENUM on_hold                   = "SUBSCRIPTION_STATE_ON_HOLD";
  STR_ENUM canceled                  = "SUBSCRIPTION_STATE_CANCELED";
  STR_ENUM expired                   = "SUBSCRIPTION_STATE_EXPIRED";
  STR_ENUM pending_purchase_canceled = "SUBSCRIPTION_STATE_PENDING_PURCHASE_CANCELED";
};

struct acknowledgement_state {
  STR_ENUM unspecified  = "ACKNOWLEDGEMENT_STATE_UNSPECIFIED";
  STR_ENUM pending      = "ACKNOWLEDGEMENT_STATE_PENDING";
  STR_ENUM acknowledged = "ACKNOWLEDGEMENT_STATE_ACKNOWLEDGED";
};

struct price_change_mode {
  STR_ENUM unspecified      = "PRICE_CHANGE_MODE_UNSPECIFIED";
  STR_ENUM decrease         = "PRICE_DECREASE";
  STR_ENUM increase         = "PRICE_INCREASE";
  STR_ENUM opt_out_increase = "OPT_OUT_PRICE_INCREASE";
};

struct price_change_state {
  STR_ENUM unspecified = "PRICE_CHANGE_STATE_UNSPECIFIED";
  STR_ENUM outstanding = "OUTSTANDING";
  STR_ENUM confirmed   = "CONFIRMED";
  STR_ENUM applied     = "APPLIED";
};

struct cancel_survey_reason {
  STR_ENUM unspecified      = "CANCEL_SURVEY_REASON_UNSPECIFIED";
  STR_ENUM not_enough_usage = "CANCEL_SURVEY_REASON_NOT_ENOUGH_USAGE";
  STR_ENUM technical_issues = "CANCEL_SURVEY_REASON_TECHNICAL_ISSUES";
  STR_ENUM cost_related     = "CANCEL_SURVEY_REASON_COST_RELATED";
  STR_ENUM found_better_app = "CANCEL_SURVEY_REASON_FOUND_BETTER_APP";
  STR_ENUM others           = "CANCEL_SURVEY_REASON_OTHERS";
};

}
