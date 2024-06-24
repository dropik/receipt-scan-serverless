//
// Created by Daniil Ryzhkov on 22/06/2024.
//

#include <gtest/gtest.h>
#include "di/container.hpp"

struct i_a {};

struct a {
  int x = 0;
};

template<typename TA = i_a>
struct b {
  explicit b(TA a) : m_a(std::move(a)) {}

  int get_a_x() {
    return m_a->x;
  }

  void set_a_x(int x) {
    m_a->x = x;
  }

  TA m_a;
};

TEST(di_test, transient_should_return_different_instances) {
  di::container<
      di::transient<a>
  > container;

  auto a1 = container.get<a>();
  auto a2 = container.get<a>();
  a1->x = 1;
  a2->x = 2;

  EXPECT_EQ(a1->x, 1);
  EXPECT_EQ(a2->x, 2);
}

TEST(di_test, singleton_should_return_same_instance) {
  di::container<
      di::singleton<a>
  > container;

  auto a1 = container.get<a>();
  auto a2 = container.get<a>();
  a1->x = 1;
  a2->x = 2;

  EXPECT_EQ(a1->x, 2);
  EXPECT_EQ(a2->x, 2);
}

TEST(di_test, singleton_should_persist_container_destruction) {
  {
    di::container<
        di::singleton<a>
    > container;

    auto a1 = container.get<a>();
    a1->x = 123;
  }

  di::container<
      di::singleton<a>
  > container;

  auto a2 = container.get<a>();
  EXPECT_EQ(a2->x, 123);
}

TEST(di_test, scoped_should_return_same_instance) {
  di::container<
      di::scoped<a>
  > container;

  auto a1 = container.get<a>();
  auto a2 = container.get<a>();
  a1->x = 1;
  a2->x = 2;

  EXPECT_EQ(a1->x, 2);
  EXPECT_EQ(a2->x, 2);
}

TEST(di_test, scoped_should_have_lifetime_of_container) {
  di::container<
      di::scoped<a>
  > container;

  auto a1 = container.get<a>();
  a1->x = 123;

  di::container<
      di::scoped<a>
  > container2;

  auto a2 = container2.get<a>();
  a2->x = 1;

  EXPECT_EQ(a1->x, 123);
  EXPECT_EQ(a2->x, 1);
}

TEST(di_test, scoped_should_be_destroyed_with_container) {
  di::container<
      di::scoped<a>
  > container;

  auto a1 = container.get<a>();
  a1->x = 123;

  {
    di::container<
        di::scoped<a>
    > container2;

    auto a2 = container2.get<a>();
    a2->x = 1;
  }

  auto a3 = container.get<a>();
  EXPECT_EQ(a3->x, 123);
}

TEST(di_test, transient_const_should_work) {
  di::container<
      di::transient<a>
  > container;

  auto a1 = container.get<a>();
  auto a2 = container.get<const a>();
  a1->x = 1;

  EXPECT_EQ(a1->x, 1);
  EXPECT_EQ(a2->x, 0);
}

TEST(di_test, singleton_const_should_work) {
  di::container<
      di::singleton<a>
  > container;

  auto a1 = container.get<a>();
  auto a2 = container.get<const a>();
  a1->x = 1;

  EXPECT_EQ(a1->x, 1);
  EXPECT_EQ(a2->x, 1);
}

TEST(di_test, scoped_const_should_work) {
  di::container<
      di::scoped<a>
  > container;

  auto a1 = container.get<a>();
  auto a2 = container.get<const a>();
  a1->x = 1;

  EXPECT_EQ(a1->x, 1);
  EXPECT_EQ(a2->x, 1);
}

TEST(di_test, interface_should_be_resolved) {
  di::container<
      di::singleton<i_a, a>,
      di::singleton<b<>>
  > container;

  auto a1 = container.get<i_a>();
  a1->x = 1;
  auto b1 = container.get<b<>>();

  EXPECT_EQ(a1->x, 1);
  EXPECT_EQ(b1->get_a_x(), 1);
}

TEST(di_test, transient_dependency_should_be_unique) {
  di::container<
      di::transient<i_a, a>,
      di::transient<b<>>
  > container;

  auto b1 = container.get<b<>>();
  auto b2 = container.get<b<>>();

  b1->set_a_x(1);
  b2->set_a_x(2);

  EXPECT_EQ(b1->get_a_x(), 1);
  EXPECT_EQ(b2->get_a_x(), 2);
}
