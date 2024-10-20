//
// Created by Daniil Ryzhkov on 20/10/2024.
//

#pragma once

#include <lambda/nullable.hpp>
#include <lambda/string_utils.hpp>
#include "../../types.hpp"

namespace api::services::google_api::purchases_subscriptions_v2::models {

struct subscription_item_price_change_details {
  money new_price;
  std::string price_change_mode;
  std::string price_change_state;
  lambda::nullable<std::string> expected_new_price_charge_time;

  JSON_BEGIN_SERIALIZER(subscription_item_price_change_details)
      JSON_PROPERTY("newPrice", new_price)
      JSON_PROPERTY("priceChangeMode", price_change_mode)
      JSON_PROPERTY("priceChangeState", price_change_state)
      JSON_PROPERTY("expectedNewPriceChargeTime", expected_new_price_charge_time)
  JSON_END_SERIALIZER()
};

struct pending_cancellation {
  JSON_BEGIN_SERIALIZER(pending_cancellation)
  JSON_END_SERIALIZER()
};

struct installment_plan {
  int initial_committed_payments_count = 0;
  int subsequent_committed_payments_count = 0;
  int remaining_committed_payments_count = 0;
  lambda::nullable<pending_cancellation> pending_cancellation;

  JSON_BEGIN_SERIALIZER(installment_plan)
      JSON_PROPERTY("initialCommittedPaymentsCount", initial_committed_payments_count)
      JSON_PROPERTY("subsequentCommittedPaymentsCount", subsequent_committed_payments_count)
      JSON_PROPERTY("remainingCommittedPaymentsCount", remaining_committed_payments_count)
      JSON_PROPERTY("pendingCancellation", pending_cancellation)
  JSON_END_SERIALIZER()
};

struct auto_renewing_plan {
  bool auto_renew_enabled = false;
  lambda::nullable<subscription_item_price_change_details> price_change_details;
  lambda::nullable<installment_plan> installment_details;

  JSON_BEGIN_SERIALIZER(auto_renewing_plan)
      JSON_PROPERTY("autoRenewEnabled", auto_renew_enabled)
      JSON_PROPERTY("priceChangeDetails", price_change_details)
      JSON_PROPERTY("installmentDetails", installment_details)
  JSON_END_SERIALIZER()
};

struct prepaid_plan {
  lambda::nullable<std::string> allow_extend_after_time;

  JSON_BEGIN_SERIALIZER(prepaid_plan)
      JSON_PROPERTY("allowExtendAfterTime", allow_extend_after_time)
  JSON_END_SERIALIZER()
};

struct offer_details {
  std::vector<std::string> offer_tags;
  std::string base_plan_id;
  std::string offer_id;

  JSON_BEGIN_SERIALIZER(offer_details)
      JSON_PROPERTY("offerTags", offer_tags)
      JSON_PROPERTY("basePlanId", base_plan_id)
      JSON_PROPERTY("offerId", offer_id)
  JSON_END_SERIALIZER()
};

struct deferred_item_replacement {
  std::string product_id;

  JSON_BEGIN_SERIALIZER(deferred_item_replacement)
      JSON_PROPERTY("productId", product_id)
  JSON_END_SERIALIZER()
};

struct subscription_purchase_line_item {
  std::string product_id;
  lambda::nullable<std::string> expiry_time;
  lambda::nullable<auto_renewing_plan> auto_renewing_plan;
  lambda::nullable<prepaid_plan> prepaid_plan;
  lambda::nullable<offer_details> offer_details;
  lambda::nullable<deferred_item_replacement> deferred_item_replacement;

  JSON_BEGIN_SERIALIZER(subscription_purchase_line_item)
      JSON_PROPERTY("productId", product_id)
      JSON_PROPERTY("expiryTime", expiry_time)
      JSON_PROPERTY("autoRenewingPlan", auto_renewing_plan)
      JSON_PROPERTY("prepaidPlan", prepaid_plan)
      JSON_PROPERTY("offerDetails", offer_details)
      JSON_PROPERTY("deferredItemReplacement", deferred_item_replacement)
  JSON_END_SERIALIZER()
};

struct paused_state_context {
  std::string auto_resume_time;

  JSON_BEGIN_SERIALIZER(paused_state_context)
      JSON_PROPERTY("autoResumeTime", auto_resume_time)
  JSON_END_SERIALIZER()
};

struct cancel_survey_result {
  std::string reason;
  lambda::nullable<std::string> reason_user_input;

  JSON_BEGIN_SERIALIZER(cancel_survey_result)
      JSON_PROPERTY("reason", reason)
      JSON_PROPERTY("reasonUserInput", reason_user_input)
  JSON_END_SERIALIZER()
};

struct user_initiated_cancellation {
  lambda::nullable<cancel_survey_result> cancel_survey_result;
  std::string cancel_time;

  JSON_BEGIN_SERIALIZER(user_initiated_cancellation)
      JSON_PROPERTY("cancelSurveyResult", cancel_survey_result)
      JSON_PROPERTY("cancelTime", cancel_time)
  JSON_END_SERIALIZER()
};

struct system_initiated_cancellation {
  JSON_BEGIN_SERIALIZER(system_initiated_cancellation)
  JSON_END_SERIALIZER()
};

struct developer_initiated_cancellation {
  JSON_BEGIN_SERIALIZER(developer_initiated_cancellation)
  JSON_END_SERIALIZER()
};

struct replacement_cancellation {
  JSON_BEGIN_SERIALIZER(replacement_cancellation)
  JSON_END_SERIALIZER()
};

struct canceled_state_context {
  lambda::nullable<user_initiated_cancellation> user_initiated_cancellation;
  lambda::nullable<system_initiated_cancellation> system_initiated_cancellation;
  lambda::nullable<developer_initiated_cancellation> developer_initiated_cancellation;
  lambda::nullable<replacement_cancellation> replacement_cancellation;

  JSON_BEGIN_SERIALIZER(canceled_state_context)
      JSON_PROPERTY("userInitiatedCancellation", user_initiated_cancellation)
      JSON_PROPERTY("systemInitiatedCancellation", system_initiated_cancellation)
      JSON_PROPERTY("developerInitiatedCancellation", developer_initiated_cancellation)
      JSON_PROPERTY("replacementCancellation", replacement_cancellation)
  JSON_END_SERIALIZER()
};

struct test_purchase {
  JSON_BEGIN_SERIALIZER(test_purchase)
  JSON_END_SERIALIZER()
};

struct external_account_identifiers {
  lambda::nullable<std::string> external_account_id;
  lambda::nullable<std::string> obfuscated_external_account_id;
  lambda::nullable<std::string> obfuscated_external_profile_id;

  JSON_BEGIN_SERIALIZER(external_account_identifiers)
      JSON_PROPERTY("externalAccountId", external_account_id)
      JSON_PROPERTY("obfuscatedExternalAccountId", obfuscated_external_account_id)
      JSON_PROPERTY("obfuscatedExternalProfileId", obfuscated_external_profile_id)
  JSON_END_SERIALIZER()
};

struct subscribe_with_google_info {
  lambda::nullable<std::string> profile_id;
  lambda::nullable<std::string> profile_name;
  lambda::nullable<std::string> email_address;
  lambda::nullable<std::string> given_name;
  lambda::nullable<std::string> family_name;

  JSON_BEGIN_SERIALIZER(subscribe_with_google_info)
      JSON_PROPERTY("profileId", profile_id)
      JSON_PROPERTY("profileName", profile_name)
      JSON_PROPERTY("emailAddress", email_address)
      JSON_PROPERTY("givenName", given_name)
      JSON_PROPERTY("familyName", family_name)
  JSON_END_SERIALIZER()
};

struct subscription_purchase_v2 {
  std::string kind;
  std::string region_code;
  std::vector<subscription_purchase_line_item> line_items;
  lambda::nullable<std::string> start_time;
  std::string subscription_state;
  std::string latest_order_id;
  lambda::nullable<std::string> linked_purchase_token;
  lambda::nullable<paused_state_context> paused_state_context;
  lambda::nullable<canceled_state_context> canceled_state_context;
  lambda::nullable<test_purchase> test_purchase;
  std::string acknowledgement_state;
  lambda::nullable<external_account_identifiers> external_account_identifiers;
  lambda::nullable<subscribe_with_google_info> subscribe_with_google_info;

  JSON_BEGIN_SERIALIZER(subscription_purchase_v2)
      JSON_PROPERTY("kind", kind)
      JSON_PROPERTY("regionCode", region_code)
      JSON_PROPERTY("lineItems", line_items)
      JSON_PROPERTY("startTime", start_time)
      JSON_PROPERTY("subscriptionState", subscription_state)
      JSON_PROPERTY("latestOrderId", latest_order_id)
      JSON_PROPERTY("linkedPurchaseToken", linked_purchase_token)
      JSON_PROPERTY("pausedStateContext", paused_state_context)
      JSON_PROPERTY("canceledStateContext", canceled_state_context)
      JSON_PROPERTY("testPurchase", test_purchase)
      JSON_PROPERTY("acknowledgementState", acknowledgement_state)
      JSON_PROPERTY("externalAccountIdentifiers", external_account_identifiers)
      JSON_PROPERTY("subscribeWithGoogleInfo", subscribe_with_google_info)
  JSON_END_SERIALIZER()
};

}
