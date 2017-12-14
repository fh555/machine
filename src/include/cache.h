// CACHE HEADER

#pragma once

#include <mutex>
#include <cmath>

#include "policy.h"

#include "policy_fifo.h"
#include "policy_lfu.h"
#include "policy_lru.h"

namespace machine {

extern int super_block_factor;

void PrintCapacity(const size_t block_count);

// Base class for all caching algorithms
template <typename Key, typename Value, typename Policy>
class Cache {
 public:

  Cache(size_t capacity);

  Block Put(const Key& key, const Value& value);

  Value Get(const Key& key) const;

  size_t GetSize() const;

  void Print() const;

  bool IsSequential(const size_t& next);

 private:

  mutable Policy cache_policy_;

  size_t capacity_;

  size_t current_block_;

};

}  // End machine namespace
