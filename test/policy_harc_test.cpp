// HARC TEST

#include <gtest/gtest.h>

#include <map>
#include <unordered_map>
#include <mutex>

#include "policy_harc.h"
#include "cache.h"
#include "configuration.h"

namespace machine {

template <typename Key, typename Value>
using harc_t = HARCCachePolicy<Key, Value>;
template <typename Key, typename Value>
using harc_cache_t = Cache<Key, Value, HARCCachePolicy<Key, Value>>;

TEST(HARCCache, SimplePut) {
  size_t cache_capacity = 1;
  harc_cache_t<int, int> cache(cache_capacity);

  cache.Put(1, 666);

  EXPECT_EQ(cache.Get(1), 666);
}

TEST(HARCCache, MissingValue) {
  size_t cache_capacity = 1;
  harc_cache_t<int, int> cache(cache_capacity);

  EXPECT_EQ(cache.Get(1), INVALID_VALUE);
}

TEST(HARCCache, CheckVictim) {
  size_t cache_capacity = 3;
  harc_cache_t<int, int> cache(cache_capacity);

  cache.Put(1, 1);
  cache.Put(2, 2);
  cache.Put(3, 3);
  cache.Put(4, 4);

  EXPECT_EQ(cache.GetSize(), 3);

}

TEST(HARCCache, CheckPointerMove) {
  size_t cache_capacity = 3;
  harc_cache_t<int, int> cache(cache_capacity);

  cache.Put(1, 1);
  cache.Put(2, 2);
  cache.Put(3, 3);

  // At this stage key4 moved from T1 to T2 and
  // T1 still bigger list and lru entry is key1;
  cache.Put(4,  4);

  EXPECT_EQ(cache.Get(1), INVALID_VALUE);
}


TEST(HARCCache, CheckPointerMoveT2) {
  size_t cache_capacity = 3;
  harc_cache_t<int, int> cache(cache_capacity);

  cache.Put(1, 1);
  cache.Put(2, 2);
  cache.Put(3, 3);

  cache.Put(3, 7);
  cache.Put(3, 7);

  // T2 list is bigger now and key2 have lower reference count
  cache.Put(1, 7);

  cache.Put(4, 4);

  EXPECT_EQ(cache.Get(2), INVALID_VALUE);
}


TEST(HARCCache, CheckPointerSaveB1) {
  size_t cache_capacity = 4;
  harc_cache_t<int, int> cache(cache_capacity);

  cache.Put(1, 1);
  cache.Put(2, 2);
  cache.Put(3, 3);
  cache.Put(4, 4);
  cache.Put(5, 3);
  cache.Put(1, 5);

  //key1 is restored from B1 and key2 moved to B1 (out of cache)
  EXPECT_EQ(cache.Get(2), INVALID_VALUE);
}

TEST(HARCCache, CheckPointerSaveB2) {
  size_t cache_capacity = 4;
  harc_cache_t<int, int> cache(cache_capacity);

  cache.Put(1, 1);
  cache.Put(2, 2);
  cache.Put(3, 3);
  cache.Put(4, 4);

  cache.Put(1,  7);
  cache.Put(2,  7);
  cache.Put(2,  7);
  cache.Put(3,  7);
  cache.Put(3,  7);
  cache.Put(4,  7);
  cache.Put(4,  7);

  cache.Put(5, 5);
  cache.Put(1, 3);

  //key1 is restored from B1 and key2 moved to B1 (out of cache)
  EXPECT_EQ(cache.Get(5), INVALID_VALUE);
}

TEST(HARCCache, KeepsAllValuesWithinCapacity) {
  constexpr int CACHE_CAPACITY = 5;
  const int TEST_RECORDS = 10;
  harc_cache_t<int, int> cache(CACHE_CAPACITY);

  for (int i = 0; i < TEST_RECORDS; ++i) {
    cache.Put(i, i);
  }

  for (int i = 0; i < TEST_RECORDS - CACHE_CAPACITY; ++i) {
    EXPECT_EQ(cache.Get(i), INVALID_VALUE);
  }

  for (int i = TEST_RECORDS - CACHE_CAPACITY; i < TEST_RECORDS; ++i) {
    EXPECT_EQ(i, cache.Get(i));
  }
}

TEST(HARCCache, RecencyCheck) {
  size_t cache_capacity = 3;
  harc_cache_t<int, int> cache(cache_capacity);

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
  EXPECT_EQ(cache.Get(1), INVALID_VALUE);
}

TEST(HARCCache, FrequencyCheck) {
  size_t cache_capacity = 3;
  harc_cache_t<int, int> cache(cache_capacity);

  cache.Put(1, 1);
  cache.Put(1, 1);
  cache.Put(2, 2);
  cache.Put(2, 2);
  cache.Put(3, 3);
  cache.Put(3, 3);
  cache.Put(1, 1);
  cache.Put(1, 1);
  cache.Put(4, 4);

  EXPECT_EQ(cache.GetSize(), 3);
  EXPECT_EQ(cache.Get(2), INVALID_VALUE);
}

TEST(HARCCache, DirtyPageTest1) {
  size_t cache_capacity = 4;
  double clean_search_fraction = 0;
  harc_cache_t<int, int> cache(cache_capacity, clean_search_fraction);

  cache.Put(1, DIRTY_BLOCK);
  cache.Put(2, CLEAN_BLOCK);
  cache.Put(3, CLEAN_BLOCK);
  cache.Put(4, CLEAN_BLOCK);
  cache.Put(5, CLEAN_BLOCK);

  EXPECT_EQ(cache.GetSize(), cache_capacity);
  EXPECT_EQ(cache.Get(1), INVALID_VALUE);
}

TEST(HARCCache, DirtyPageTest2) {
  size_t cache_capacity = 4;
  double clean_search_fraction = 0.5;
  harc_cache_t<int, int> cache(cache_capacity, clean_search_fraction);

  cache.Put(1, DIRTY_BLOCK);
  cache.Put(2, CLEAN_BLOCK);
  cache.Put(3, CLEAN_BLOCK);
  cache.Put(4, CLEAN_BLOCK);
  cache.Put(5, CLEAN_BLOCK);

  EXPECT_EQ(cache.GetSize(), cache_capacity);
  EXPECT_EQ(cache.Get(2), INVALID_VALUE);
}


}  // End machine namespace
