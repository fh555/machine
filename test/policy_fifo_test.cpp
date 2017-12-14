// FIFO TEST

#include <gtest/gtest.h>

#include <map>
#include <unordered_map>
#include <mutex>

#include "policy_fifo.h"
#include "cache.h"

namespace machine {

template <typename Key, typename Value>
using fifo_cache_t = Cache<Key, Value, FIFOCachePolicy<Key, Value>>;

TEST(FIFOCache, Simple_Test) {
  size_t cache_capacity = 2;
  fifo_cache_t<int, int> fc(cache_capacity);

  fc.Put(1, 10);
  fc.Put(2, 20);

  EXPECT_EQ(fc.GetSize(), 2);
  EXPECT_EQ(fc.Get(1), 10);
  EXPECT_EQ(fc.Get(2), 20);

  fc.Put(1, 30);
  EXPECT_EQ(fc.GetSize(), 2);
  EXPECT_EQ(fc.Get(1), 30);

  fc.Put(3, 30);

  EXPECT_THROW(fc.Get(1), std::range_error);
  EXPECT_EQ(fc.Get(2), 20);
  EXPECT_EQ(fc.Get(3), 30);
}

TEST(FIFOCache, Missing_Value) {
  size_t cache_capacity = 2;
  fifo_cache_t<int, int> fc(cache_capacity);

  fc.Put(1, 10);

  EXPECT_EQ(fc.GetSize(), 1);
  EXPECT_EQ(fc.Get(1), 10);
  EXPECT_THROW(fc.Get(2), std::range_error);
}

TEST(FIFOCache, Sequence_Test) {
  constexpr int TEST_SIZE = 10;
  fifo_cache_t<int, int> fc(TEST_SIZE);

  for (size_t i = 0; i < TEST_SIZE; ++i) {
    fc.Put(i, i);
  }

  EXPECT_EQ(fc.GetSize(), TEST_SIZE);

  for (size_t i = 0; i < TEST_SIZE; ++i) {
    EXPECT_EQ(fc.Get(i), i);
  }

  // replace a half
  for (size_t i = 0; i < TEST_SIZE / 2; ++i) {
    fc.Put(i + TEST_SIZE, i);
  }

  EXPECT_EQ(fc.GetSize(), TEST_SIZE);

  for (size_t i = 0; i < TEST_SIZE / 2; ++i) {
    EXPECT_THROW(fc.Get(i), std::range_error);
  }

  for (size_t i = 0; i < TEST_SIZE / 2; ++i) {
    EXPECT_EQ(fc.Get(i + TEST_SIZE), i);
  }

  for (size_t i = TEST_SIZE / 2; i < TEST_SIZE; ++i) {
    EXPECT_EQ(fc.Get(i), i);
  }
}

TEST(FIFOCache, CheckVictim) {
  size_t cache_capacity = 3;
  fifo_cache_t<int, int> cache(cache_capacity);

  cache.Put(1, 1);
  cache.Put(2, 2);
  cache.Put(3, 3);
  cache.Put(4, 4);

  EXPECT_EQ(cache.GetSize(), 3);

}

}  // End machine namespace
