// CACHE SOURCE

#include <iostream>
#include <cstddef>
#include <limits>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <type_traits>

#include "cache.h"

namespace machine {

#define CACHE_TEMPLATE_ARGUMENT \
    template <typename Key, typename Value, typename Policy>

#define CACHE_TEMPLATE_TYPE \
    Cache<Key, Value, Policy>

CACHE_TEMPLATE_ARGUMENT
CACHE_TEMPLATE_TYPE::Cache(size_t capacity)
: cache_policy_(Policy(capacity)),
  capacity_{capacity} {

  PL_ASSERT(capacity_ > 0);

}

CACHE_TEMPLATE_ARGUMENT
Block CACHE_TEMPLATE_TYPE::Put(const Key& key,
                               const Value& value) {

  operation_guard{cache_mutex_};
  auto entry_location = LocateEntry(key);
  Block victim;
  Key victim_key = INVALID_KEY;
  Value victim_value = INVALID_KEY;

  if (entry_location == cache_items_map.end()) {

    // add new element to the cache
    if (CurrentCapacity() + 1 > capacity_) {
      victim_key = cache_policy_.Victim(key);
      DLOG(INFO) << "Victim: " << victim_key;
      victim_value = Get(victim_key, false);
      Erase(victim_key);
    }

    Insert(key, value);

    if (CurrentCapacity() > capacity_) {
      LOG(INFO) << "Capacity exceeded";
      exit(EXIT_FAILURE);
    }

  }
  else {

    // update previous value
    Update(key, value);

  }

  // return victim
  victim.block_id = victim_key;
  victim.block_type = victim_value;
  return victim;
}

CACHE_TEMPLATE_ARGUMENT
const Value& CACHE_TEMPLATE_TYPE::Get(const Key& key,
                                      bool touch) const {

  operation_guard{cache_mutex_};
  auto elem_it = LocateEntry(key);

  if (elem_it == cache_items_map.end()) {
    throw std::range_error{"No such element in the cache"};
  }

  if(touch == true){
    cache_policy_.Touch(key);
  }

  return elem_it->second;
}

CACHE_TEMPLATE_ARGUMENT
size_t CACHE_TEMPLATE_TYPE::CurrentCapacity() const {

  operation_guard{cache_mutex_};

  return cache_items_map.size();
}


CACHE_TEMPLATE_ARGUMENT
void CACHE_TEMPLATE_TYPE::Insert(const Key& key,
                                 const Value& value) {

  cache_policy_.Insert(key);
  cache_items_map.emplace(std::make_pair(key, value));

}

CACHE_TEMPLATE_ARGUMENT
void CACHE_TEMPLATE_TYPE::Erase(const Key& key) {

  cache_policy_.Erase(key);
  cache_items_map.erase(key);

}

CACHE_TEMPLATE_ARGUMENT
void CACHE_TEMPLATE_TYPE::Update(const Key& key,
                                 const Value& value) {

  cache_policy_.Touch(key);
  cache_items_map[key] = value;

}

CACHE_TEMPLATE_ARGUMENT
typename CACHE_TEMPLATE_TYPE::const_iterator
CACHE_TEMPLATE_TYPE::LocateEntry(const Key& key) const {

  return cache_items_map.find(key);

}

CACHE_TEMPLATE_ARGUMENT
void CACHE_TEMPLATE_TYPE::Print() const {

  std::cout << "SIZE: " << cache_items_map.size() << "\n";

  size_t block_itr = 0;
  for(auto& cache_item : cache_items_map){
    if(cache_item.second == CLEAN_BLOCK){
      std::cout << cache_item.first << " ";
    }
    else if(cache_item.second == DIRTY_BLOCK){
      std::cout << cache_item.first << "● ";
    }
    else {
      LOG(INFO) << "Invalid block type : " << cache_item.second;
      exit(EXIT_FAILURE);
    }

    if(block_itr++ > 100){
      break;
    }
  }

  std::cout << "\n-------------------------------\n";

}

// Instantiations

// LRU
template class Cache<int, int, LRUCachePolicy<int>>;

// LFU
template class Cache<int, int, LFUCachePolicy<int>>;

// FIFO
template class Cache<int, int, FIFOCachePolicy<int>>;

// ARC
template class Cache<int, int, ARCCachePolicy<int>>;

}  // End machine namespace

