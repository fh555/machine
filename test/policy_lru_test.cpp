// LRU TEST

#include <gtest/gtest.h>

#include <map>
#include <unordered_map>
#include <mutex>

#include "policy_lru.h"
#include "cache.h"

namespace machine {

template <typename Key, typename Value>
using lru_cache_t = Cache<Key, Value, LRUCachePolicy<Key, Value>>;

TEST(LRUCache, SimplePut) {
  size_t cache_capacity = 1;
  lru_cache_t<int, int> cache(cache_capacity);

  cache.Put(1, 666);

  EXPECT_EQ(cache.Get(1), 666);
}

TEST(LRUCache, MissingValue) {
  size_t cache_capacity = 1;
  lru_cache_t<int, int> cache(cache_capacity);

  EXPECT_THROW(cache.Get(0), std::range_error);
}

TEST(LRUCache, KeepsAllValuesWithinCapacity) {
  constexpr int CACHE_CAPACITY = 50;
  const int TEST_RECORDS = 100;
  lru_cache_t<int, int> cache(CACHE_CAPACITY);

  for (int i = 0; i < TEST_RECORDS; ++i) {
    cache.Put(i, i);
  }

  for (int i = 0; i < TEST_RECORDS - CACHE_CAPACITY; ++i) {
    EXPECT_THROW(cache.Get(i), std::range_error);
  }

  for (int i = TEST_RECORDS - CACHE_CAPACITY; i < TEST_RECORDS; ++i) {
    EXPECT_EQ(i, cache.Get(i));
  }

}

TEST(LRUCache, CheckVictim) {
  size_t cache_capacity = 3;
  lru_cache_t<int, int> cache(cache_capacity);

  cache.Put(1, 1);
  cache.Put(2, 2);
  cache.Put(3, 3);
  cache.Put(4, 4);

  EXPECT_EQ(cache.GetSize(), 3);

}

TEST(LRUCache, RecencyCheck){
  size_t cache_capacity = 3;
  lru_cache_t<int, int> cache(cache_capacity);

  cache.Put(1, 1);
  cache.Put(2, 2);
  cache.Put(3, 3);
  cache.Put(1, 1);
  cache.Put(1, 1);
  cache.Put(1, 1);
  cache.Put(1, 1);
  cache.Put(2, 2);
  cache.Put(3, 3);
  cache.Put(4, 4);

  EXPECT_EQ(cache.GetSize(), 3);
  EXPECT_THROW(cache.Get(1), std::range_error);
  cache.Print();
}


}  // End machine namespace
