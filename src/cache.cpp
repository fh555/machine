// CACHE SOURCE

#include <iostream>
#include <cstddef>
#include <limits>
#include <memory>
#include <unordered_map>
#include <type_traits>

#include "cache.h"
#include <stdlib.h>

namespace machine {

int super_block_factor = 512;

void PrintCapacity(const size_t block_count){

  // 1 block == 4 KB
  size_t capacity = block_count * 4 * super_block_factor;

  if(capacity < 1024) {
    std::cout << "[" << capacity <<" KB] ";
  }
  else if(capacity < 1024 * 1024){
    std::cout << "[" << capacity/1024 <<" MB] ";
  }
  else {
    std::cout << "[" << capacity/(1024 * 1024) <<" GB] ";
  }

}

#define CACHE_TEMPLATE_ARGUMENT \
    template <typename Key, typename Value, typename Policy>

#define CACHE_TEMPLATE_TYPE \
    Cache<Key, Value, Policy>

CACHE_TEMPLATE_ARGUMENT
CACHE_TEMPLATE_TYPE::Cache(size_t capacity)
: cache_policy_(Policy(capacity)),
  capacity_{capacity},
  current_block_{0} {

  PL_ASSERT(capacity_ > 0);

}

CACHE_TEMPLATE_ARGUMENT
Block CACHE_TEMPLATE_TYPE::Put(const Key& key,
                               const Value& value) {
  return cache_policy_.Put(key, value);
}

CACHE_TEMPLATE_ARGUMENT
Value CACHE_TEMPLATE_TYPE::Get(const Key& key) const {
  return cache_policy_.Get(key);
}

CACHE_TEMPLATE_ARGUMENT
size_t CACHE_TEMPLATE_TYPE::GetSize() const {
  return cache_policy_.GetSize();
}


CACHE_TEMPLATE_ARGUMENT
void CACHE_TEMPLATE_TYPE::Print() const {
  cache_policy_.Print();
}

CACHE_TEMPLATE_ARGUMENT
bool CACHE_TEMPLATE_TYPE::IsSequential(const size_t& next) {

  bool status = false;
  size_t distance = abs(current_block_ - next);
  DLOG(INFO) << "CURRENT: " << current_block_ << " NEXT: " << next << "\n";

  if(distance == 1){
    status = true;
  }
  else {
    status = false;
  }

  current_block_ = next;
  return status;
}

// Instantiations

// FIFO
template class Cache<int, int, FIFOCachePolicy<int, int>>;

// LFU
template class Cache<int, int, LFUCachePolicy<int, int>>;

// LRU
template class Cache<int, int, LRUCachePolicy<int, int>>;

// ARC
template class Cache<int, int, ARCCachePolicy<int, int>>;


}  // End machine namespace

