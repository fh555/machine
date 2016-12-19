// FIFO TEST

#include <gtest/gtest.h>

#include <map>
#include <unordered_map>
#include <mutex>

#include "cache.h"
#include "fifo_policy.h"

namespace machine {

template <typename Key>
using fifo_t = FIFOCachePolicy<Key>;
template <typename Key, typename Value>
using fifo_cache_t = Cache<Key, Value, FIFOCachePolicy<Key>>;

TEST(FIFOCache, Simple_Test) {
  fifo_cache_t<int, int> fc(2, fifo_t<int>(2));

  fc.Put(1, 10);
  fc.Put(2, 20);

  EXPECT_EQ(fc.Size(), 2);
  EXPECT_EQ(fc.Get(1), 10);
  EXPECT_EQ(fc.Get(2), 20);

  fc.Put(1, 30);
  EXPECT_EQ(fc.Size(), 2);
  EXPECT_EQ(fc.Get(1), 30);

  fc.Put(3, 30);
  EXPECT_THROW(fc.Get(1), std::range_error);
  EXPECT_EQ(fc.Get(2), 20);
  EXPECT_EQ(fc.Get(3), 30);
}

TEST(FIFOCache, Missing_Value) {
  fifo_cache_t<int, int> fc(2, fifo_t<int>(2));

  fc.Put(1, 10);

  EXPECT_EQ(fc.Size(), 1);
  EXPECT_EQ(fc.Get(1), 10);
  EXPECT_THROW(fc.Get(2), std::range_error);
}

TEST(FIFOCache, Sequence_Test) {
  constexpr int TEST_SIZE = 10;
  fifo_cache_t<std::string, int> fc(TEST_SIZE, fifo_t<std::string>(TEST_SIZE));

  for (size_t i = 0; i < TEST_SIZE; ++i) {
    fc.Put(std::to_string('0' + i), i);
  }

  EXPECT_EQ(fc.Size(), TEST_SIZE);

  for (size_t i = 0; i < TEST_SIZE; ++i) {
    EXPECT_EQ(fc.Get(std::to_string('0' + i)), i);
  }

  // replace a half
  for (size_t i = 0; i < TEST_SIZE / 2; ++i) {
    fc.Put(std::to_string('a' + i), i);
  }

  EXPECT_EQ(fc.Size(), TEST_SIZE);

  for (size_t i = 0; i < TEST_SIZE / 2; ++i) {
    EXPECT_THROW(fc.Get(std::to_string('0' + i)), std::range_error);
  }

  for (size_t i = 0; i < TEST_SIZE / 2; ++i) {
    EXPECT_EQ(fc.Get(std::to_string('a' + i)), i);
  }

  for (size_t i = TEST_SIZE / 2; i < TEST_SIZE; ++i) {
    EXPECT_EQ(fc.Get(std::to_string('0' + i)), i);
  }
}

}  // End machine namespace
