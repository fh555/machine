// STORAGE CACHE HEADER

#pragma once

#include "cache.h"
#include "types.h"

namespace machine {

class StorageCache {

 public:

  StorageCache(DeviceType device_type,
               CachingType caching_type,
               size_t capacity,
               double clean_fraction = 0);

  Block Put(const int& key, const int& value);

  int Get(const int& key) const;

  size_t GetSize() const;

  size_t GetCapacity() const{
    return capacity_;
  }

  bool IsSequential(const size_t& next);

  friend std::ostream& operator<< (std::ostream& stream,
                                   const StorageCache& cache);

  DeviceType device_type_ = DeviceType::DEVICE_TYPE_INVALID;

  CachingType caching_type_ = CachingType::CACHING_TYPE_INVALID;

  Cache<int, int, FIFOCachePolicy<int, int>>* fifo_cache = nullptr;

  Cache<int, int, LFUCachePolicy<int, int>>* lfu_cache = nullptr;

  Cache<int, int, LRUCachePolicy<int, int>>* lru_cache = nullptr;

  Cache<int, int, ARCCachePolicy<int, int>>* arc_cache = nullptr;

  Cache<int, int, HARCCachePolicy<int, int>>* harc_cache = nullptr;

  // current block accessed
  size_t current_ = 0;

  // capacity
  size_t capacity_ = 0;

  // clean fraction
  double clean_fraction_ = 0;

};


}  // End machine namespace
