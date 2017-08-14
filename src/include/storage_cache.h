// STORAGE CACHE HEADER

#pragma once

#include "cache.h"
#include "types.h"

namespace machine {

class StorageCache {

 public:

  StorageCache(DeviceType device_type,
               CachingType caching_type,
               size_t capacity);

  Block Put(const int& key, const int& value);

  const int& Get(const int& key, bool touch = true) const;

  void Erase(const int& key);

  size_t CurrentCapacity() const;

  double GetOccupiedFraction() const;

  bool IsSequential(const size_t& next);

  friend std::ostream& operator<< (std::ostream& stream,
                                   const StorageCache& cache);

  DeviceType device_type_ = DeviceType::DEVICE_TYPE_INVALID;

  CachingType caching_type_ = CachingType::CACHING_TYPE_INVALID;

  Cache<int, int, FIFOCachePolicy<int>>* fifo_cache = nullptr;

  Cache<int, int, LRUCachePolicy<int>>* lru_cache = nullptr;

  Cache<int, int, LFUCachePolicy<int>>* lfu_cache = nullptr;

  Cache<int, int, ARCCachePolicy<int>>* arc_cache = nullptr;

  // current block accessed
  size_t current_ = 0;

  // capacity
  size_t capacity_ = 0;

};


}  // End machine namespace
