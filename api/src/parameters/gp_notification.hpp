//
// Created by Daniil Ryzhkov on 19/10/2024.
//

#pragma once

#include <string>
#include <lambda/json.hpp>

namespace api::parameters {

struct one_time_product_notification {
  std::string version;
  int notification_type = 0;
  std::string purchase_token;
  std::string sku;

  JSON_BEGIN_SERIALIZER(one_time_product_notification)
      JSON_PROPERTY("version", version)
      JSON_PROPERTY("notificationType", notification_type)
      JSON_PROPERTY("purchaseToken", purchase_token)
      JSON_PROPERTY("sku", sku)
  JSON_END_SERIALIZER()
};

struct subscription_notification {
  std::string version;
  int notification_type = 0;
  std::string purchase_token;
  std::string subscription_id;

  JSON_BEGIN_SERIALIZER(subscription_notification)
      JSON_PROPERTY("version", version)
      JSON_PROPERTY("notificationType", notification_type)
      JSON_PROPERTY("purchaseToken", purchase_token)
      JSON_PROPERTY("subscriptionId", subscription_id)
  JSON_END_SERIALIZER()
};

struct voided_purchase_notification {
  std::string purchase_token;
  std::string order_id;
  int product_type = 0;
  int refund_type = 0;

  JSON_BEGIN_SERIALIZER(voided_purchase_notification)
      JSON_PROPERTY("purchaseToken", purchase_token)
      JSON_PROPERTY("orderId", order_id)
      JSON_PROPERTY("productType", product_type)
      JSON_PROPERTY("refundType", refund_type)
  JSON_END_SERIALIZER()
};

struct test_notification {
  std::string version;

  JSON_BEGIN_SERIALIZER(test_notification)
      JSON_PROPERTY("version", version)
  JSON_END_SERIALIZER()
};

struct gp_notification {
  std::string version;
  std::string package_name;
  long long event_time_millis = 0;
  lambda::nullable<one_time_product_notification> one_time_product_notification;
  lambda::nullable<subscription_notification> subscription_notification;
  lambda::nullable<voided_purchase_notification> voided_purchase_notification;
  lambda::nullable<test_notification> test_notification;

  JSON_BEGIN_SERIALIZER(gp_notification)
      JSON_PROPERTY("version", version)
      JSON_PROPERTY("packageName", package_name)
      JSON_PROPERTY("eventTimeMillis", event_time_millis)
      JSON_PROPERTY("oneTimeProductNotification", one_time_product_notification)
      JSON_PROPERTY("subscriptionNotification", subscription_notification)
      JSON_PROPERTY("voidedPurchaseNotification", voided_purchase_notification)
      JSON_PROPERTY("testNotification", test_notification)
  JSON_END_SERIALIZER()
};

}
