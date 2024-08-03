//
// Created by Daniil Ryzhkov on 22/06/2024.
//

#include <gtest/gtest.h>
#include <di/container.hpp>
#include <repository/factories.hpp>
#include <repository/receipt_repository.hpp>
#include "integration_tests_common/base_repository_integration_test.hpp"

using namespace di;
using namespace repository::models;

class receipt_repository_test : public base_repository_integration_test {
 protected:
  std::shared_ptr<sql::Connection> get_connection() override {
    return services.get<repository::t_client>()->get_connection();
  }

  container<
      singleton<Aws::Client::ClientConfiguration>,
      singleton<repository::connection_settings>,
      scoped<repository::t_client, repository::client<>>,
      transient<repository::t_receipt_repository, repository::receipt_repository<>>
  > services;
};

static receipt create_receipt() {
  receipt r;
  r.id = "12345";
  r.user_id = DEFAULT_USER_ID;
  r.date = "2024-06-22";
  r.total_amount = 1.0;
  r.currency = "EUR";
  r.store_name = "store_name";
  r.category = "category";
  r.state = receipt::done;
  return r;
}

static user_device create_user_device() {
  user_device ud;
  ud.id = "12345";
  ud.user_id = DEFAULT_USER_ID;
  return ud;
}

TEST_F(receipt_repository_test, should_create_receipt) {
  auto receipt_repository = services.get<repository::t_receipt_repository>();
  auto r = create_receipt();
  receipt_repository->store(r);
  auto repo = services.get<repository::t_client>();
  auto stored_receipt = repo->get<receipt>(r.id);
  ASSERT_EQ(r.id, stored_receipt->id);
}

TEST_F(receipt_repository_test, should_update_receipt) {
  auto receipt_repository = services.get<repository::t_receipt_repository>();
  auto r = create_receipt();
  receipt_repository->store(r);
  r.store_name = "new_store_name";
  receipt_repository->store(r);
  auto repo = services.get<repository::t_client>();
  auto stored_receipt = repo->get<receipt>(r.id);
  ASSERT_EQ(r.store_name, stored_receipt->store_name);
}

TEST_F(receipt_repository_test, should_create_receipt_items) {
  auto receipt_repository = services.get<repository::t_receipt_repository>();
  auto r = create_receipt();
  r.items.push_back({ "item_id", r.id, "description", 1.0, "category", 0 });
  receipt_repository->store(r);
  auto repo = services.get<repository::t_client>();
  auto items = repo->select<receipt_item>("select * from receipt_items where receipt_id = ?").with_param(r.id).all();
  ASSERT_EQ(1, items->size());
  ASSERT_EQ("item_id", items->operator[](0)->id);
}

TEST_F(receipt_repository_test, should_delete_old_items) {
  auto receipt_repository = services.get<repository::t_receipt_repository>();
  auto r = create_receipt();
  r.items.push_back({ "item_id", r.id, "description", 1.0, "category", 0 });
  receipt_repository->store(r);
  r.items.clear();
  receipt_repository->store(r);
  auto repo = services.get<repository::t_client>();
  auto items = repo->select<receipt_item>("select * from receipt_items where receipt_id = ?").with_param(r.id).all();
  ASSERT_EQ(0, items->size());
}

TEST_F(receipt_repository_test, should_maintain_items_order) {
  auto receipt_repository = services.get<repository::t_receipt_repository>();
  auto r = create_receipt();
  r.items.push_back({ "item_id_1", r.id, "description", 1.0, "category", 0 });
  r.items.push_back({ "item_id_2", r.id, "description", 1.0, "category", -1 });
  receipt_repository->store(r);
  auto repo = services.get<repository::t_client>();
  auto items = repo->select<receipt_item>("select * from receipt_items where receipt_id = ? order by sort_order").with_param(r.id).all();
  ASSERT_EQ(2, items->size());
  ASSERT_EQ("item_id_1", items->operator[](0)->id);
  ASSERT_EQ("item_id_2", items->operator[](1)->id);
}

TEST_F(receipt_repository_test, should_update_receipt_file) {
  auto receipt_repository = services.get<repository::t_receipt_repository>();
  auto r = create_receipt();
  r.file = receipt_file{ "file_id", r.id, "file_name", 0 };
  receipt_repository->store(r);
  auto repo = services.get<repository::t_client>();
  auto file = repo->select<receipt_file>("select * from receipt_files where receipt_id = ?").with_param(r.id).first_or_default();
  ASSERT_TRUE(file != nullptr);
  ASSERT_EQ("file_id", file->id);
}

TEST_F(receipt_repository_test, should_delete_file) {
  auto receipt_repository = services.get<repository::t_receipt_repository>();
  auto r = create_receipt();
  r.file = receipt_file{ "file_id", r.id, "file_name", 0 };
  receipt_repository->store(r);
  r.file.clear();
  receipt_repository->store(r);
  auto repo = services.get<repository::t_client>();
  auto file = repo->select<receipt_file>("select * from receipt_files where receipt_id = ?").with_param(r.id).first_or_default();
  ASSERT_TRUE(file == nullptr);
}

TEST_F(receipt_repository_test, should_clear_old_files_if_id_changed) {
  auto receipt_repository = services.get<repository::t_receipt_repository>();
  auto r = create_receipt();
  r.file = receipt_file{ "file_id", r.id, "file_name", 0 };
  receipt_repository->store(r);
  r.file = receipt_file{ "new_id", r.id, "file_name", 0 };
  receipt_repository->store(r);
  auto repo = services.get<repository::t_client>();
  auto files = repo->select<receipt_file>("select * from receipt_files where receipt_id = ?").with_param(r.id).all();
  ASSERT_EQ(1, files->size());
  ASSERT_EQ("new_id", files->operator[](0)->id);
}

TEST_F(receipt_repository_test, should_find_receipt_by_file_name_and_doc_number) {
  auto receipt_repository = services.get<repository::t_receipt_repository>();
  auto r = create_receipt();
  r.file = receipt_file{ "file_id", r.id, "file_name", 0 };
  receipt_repository->store(r);
  auto receipt = receipt_repository->get(DEFAULT_USER_ID, "file_name", 0);
  ASSERT_TRUE(receipt.has_value());
  ASSERT_EQ(r.id, receipt.get_value().id);
}

TEST_F(receipt_repository_test, should_not_find_receipt_if_file_name_and_doc_number_do_not_match) {
  auto receipt_repository = services.get<repository::t_receipt_repository>();
  auto r = create_receipt();
  r.file = receipt_file{ "file_id", r.id, "file_name", 0 };
  receipt_repository->store(r);
  auto receipt = receipt_repository->get(DEFAULT_USER_ID, "file_name", 1);
  ASSERT_FALSE(receipt.has_value());
}

TEST_F(receipt_repository_test, should_store_receipt_version_as_0_when_created) {
  auto receipt_repository = services.get<repository::t_receipt_repository>();
  auto r = create_receipt();
  receipt_repository->store(r);
  auto repo = services.get<repository::t_client>();
  auto stored_receipt = repo->get<receipt>(r.id);
  ASSERT_EQ(0, stored_receipt->version);
}

TEST_F(receipt_repository_test, should_udpate_version_if_receipt_exists) {
  auto receipt_repository = services.get<repository::t_receipt_repository>();
  auto r = create_receipt();
  receipt_repository->store(r);
  r.store_name = "new_store_name";
  receipt_repository->store(r);
  auto repo = services.get<repository::t_client>();
  auto stored_receipt = repo->get<receipt>(r.id);
  ASSERT_EQ(1, stored_receipt->version);
}

TEST_F(receipt_repository_test, should_handle_optimistic_concurrency) {
  auto receipt_repository = services.get<repository::t_receipt_repository>();
  auto r = create_receipt();
  receipt_repository->store(r);
  auto repo = services.get<repository::t_client>();
  auto stored_receipt = repo->get<receipt>(r.id);
  stored_receipt->store_name = "new_store_name";
  receipt_repository->store(*stored_receipt);
  r.store_name = "new_store_name_2";
  try {
    receipt_repository->store(r);
    FAIL() << "Expected an exception";
  } catch (const std::exception &e) {
    ASSERT_STREQ("Optimistic concurrency error", e.what());
  }
  auto stored_receipt_2 = repo->get<receipt>(r.id);
  ASSERT_EQ(1, stored_receipt_2->version);
  ASSERT_EQ("new_store_name", stored_receipt_2->store_name);
}

TEST_F(receipt_repository_test, should_create_create_event_when_receipt_created) {
  auto receipt_repository = services.get<repository::t_receipt_repository>();
  auto r = create_receipt();
  auto ud = create_user_device();
  auto repo = services.get<repository::t_client>();
  repo->create(ud);
  receipt_repository->store(r);
  auto events = repo->select<entity_event>("select * from entity_events where entity_id = ?").with_param(r.id).all();
  ASSERT_EQ(1, events->size());
  ASSERT_EQ(r.id, events->operator[](0)->entity_id);
  ASSERT_EQ(ud.id, events->operator[](0)->device_id);
  ASSERT_EQ("receipt", events->operator[](0)->entity_type);
  auto expected_event = entity_event::create;
  ASSERT_EQ(expected_event, events->operator[](0)->event_type);
}

TEST_F(receipt_repository_test, should_create_update_event_when_receipt_updated) {
  auto receipt_repository = services.get<repository::t_receipt_repository>();
  auto r = create_receipt();
  auto ud = create_user_device();
  auto repo = services.get<repository::t_client>();
  repo->create(ud);
  receipt_repository->store(r);
  r.store_name = "new_store_name";
  receipt_repository->store(r);
  auto events =
      repo->select<entity_event>("select * from entity_events where entity_id = ? and event_type = 'update'").with_param(
          r.id).all();
  ASSERT_EQ(1, events->size());
  ASSERT_EQ(r.id, events->operator[](0)->entity_id);
  ASSERT_EQ(ud.id, events->operator[](0)->device_id);
  ASSERT_EQ("receipt", events->operator[](0)->entity_type);
  auto expected_event = entity_event::update;
  ASSERT_EQ(expected_event, events->operator[](0)->event_type);
}

TEST_F(receipt_repository_test, should_create_delete_event_when_receipt_deleted) {
  auto receipt_repository = services.get<repository::t_receipt_repository>();
  auto r = create_receipt();
  auto ud = create_user_device();
  auto repo = services.get<repository::t_client>();
  repo->create(ud);
  repo->create(r);
  repo->drop(r);
  auto events =
      repo->select<entity_event>("select * from entity_events where entity_id = ? and event_type = 'delete'").with_param(
          r.id).all();
  ASSERT_EQ(1, events->size());
  ASSERT_EQ(r.id, events->operator[](0)->entity_id);
  ASSERT_EQ(ud.id, events->operator[](0)->device_id);
  ASSERT_EQ("receipt", events->operator[](0)->entity_type);
  auto expected_event = entity_event::del;
  ASSERT_EQ(expected_event, events->operator[](0)->event_type);
}
