// ARC TEST

#include <gtest/gtest.h>

#include <map>
#include <unordered_map>
#include <mutex>

#include "policy_arc.h"
#include "cache.h"
#include "configuration.h"

namespace machine {

template <typename Key>
using arc_t = ARCCachePolicy<Key>;
template <typename Key, typename Value>
using arc_cache_t = Cache<Key, Value, ARCCachePolicy<Key>>;

TEST(ARCCache, SimplePut) {
  size_t cache_capacity = 1;
  arc_cache_t<int, int> cache(cache_capacity);

  cache.Put(1, 666);

  EXPECT_EQ(cache.Get(1), 666);
}

TEST(ARCCache, MissingValue) {
  size_t cache_capacity = 1;
  arc_cache_t<int, int> cache(cache_capacity);

  EXPECT_THROW(cache.Get(1), std::range_error);
}

TEST(ARCCache, CheckVictim) {
  size_t cache_capacity = 3;
  arc_cache_t<int, int> cache(cache_capacity);

  cache.Put(1, 1);
  cache.Put(2, 2);
  cache.Put(3, 3);
  cache.Put(4, 4);

  EXPECT_EQ(cache.CurrentCapacity(), 3);

}

TEST(ARCCache, CheckPointerMove) {
  size_t cache_capacity = 3;
  arc_cache_t<int, int> cache(cache_capacity);

  cache.Put(1, 1);
  cache.Put(2, 2);
  cache.Put(3, 3);

  // At this stage key4 moved from T1 to T2 and
  // T1 still bigger list and lru entry is key1;
  cache.Put(4,  4);

  EXPECT_THROW(cache.Get(1), std::range_error);
}


TEST(ARCCache, CheckPointerMoveT2) {
  size_t cache_capacity = 3;
  arc_cache_t<int, int> cache(cache_capacity);

  cache.Put(1, 1);
  cache.Put(2, 2);
  cache.Put(3, 3);

  cache.Put(3, 7);
  cache.Put(3, 7);

  // T2 list is bigger now and key2 have lower reference count
  cache.Put(1, 7);

  cache.Put(4, 4);

  EXPECT_THROW(cache.Get(2), std::range_error);
}


TEST(ARCCache, CheckPointerSaveB1) {
  size_t cache_capacity = 4;
  arc_cache_t<int, int> cache(cache_capacity);

  cache.Put(1, 1);
  cache.Put(2, 2);
  cache.Put(3, 3);
  cache.Put(4, 4);
  cache.Put(5, 3);
  cache.Put(1, 5);

  //key1 is restored from B1 and key2 moved to B1 (out of cache)
  EXPECT_THROW(cache.Get(2), std::range_error);
}


TEST(ARCCache, CheckPointerSaveB2) {
  size_t cache_capacity = 4;
  arc_cache_t<int, int> cache(cache_capacity);

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
  EXPECT_THROW(cache.Get(5), std::range_error);
}

TEST(ARCCache, KeepsAllValuesWithinCapacity) {
  constexpr int CACHE_CAPACITY = 5;
  const int TEST_RECORDS = 10;
  arc_cache_t<int, int> cache(CACHE_CAPACITY);

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

}  // End machine namespace
