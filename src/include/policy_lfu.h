// LFU HEADER

#pragma once

#include <cstddef>
#include <unordered_map>
#include <map>
#include <iostream>

#include "macros.h"
#include "policy.h"

namespace machine {

template <typename Key, typename Value>
class LFUCachePolicy : public ICachePolicy<Key, Value> {
 public:
  using lfu_iterator = typename std::multimap<std::size_t, Key>::iterator;

  LFUCachePolicy(const size_t& capacity,
                 UNUSED_ATTRIBUTE const double& clean_fraction)
  : capacity_(capacity){
    // Nothing to do here!
  }

  ~LFUCachePolicy() override = default;

  void Check(){

    if (GetSize() > capacity_) {
      std::cout << "Capacity exceeded \n";
      exit(EXIT_FAILURE);
    }

  }

  void Touch(const Key& key){

    // check if key exists
    if(lfu_storage.count(key) == 0){
      std::cout << "KEY NOT FOUND: " << key << "\n";
      exit(EXIT_FAILURE);
    }

    // get the previous frequency value of a key
    auto elem_for_update = lfu_storage[key];
    auto updated_elem = std::make_pair(elem_for_update->first + 1,
                                       elem_for_update->second);

    // update the previous value
    frequency_storage.erase(elem_for_update);
    lfu_storage[key] = frequency_storage.emplace_hint(frequency_storage.cend(),
                                                      std::move(updated_elem));

  }

  Block Put(const Key& key, const Value& value){
    Block victim;
    Key victim_key = INVALID_KEY;
    Value victim_value = INVALID_VALUE;

    auto entry_location = cache_items_map.find(key);
    if (entry_location == cache_items_map.end()) {
      // add new element to the cache

      // check capacity
      if (GetSize() + 1 > capacity_) {
        victim_key = frequency_storage.cbegin()->second;
        victim_value = cache_items_map[victim_key];
        //std::cout << "Victim: " << victim_key << "\n";

        // evict victim
        frequency_storage.erase(lfu_storage[victim_key]);
        lfu_storage.erase(victim_key);
        cache_items_map.erase(victim_key);
      }

      // insert new element
      constexpr std::size_t INIT_FREQUENCY = 1;
      lfu_storage[key] = frequency_storage.emplace_hint(frequency_storage.cbegin(),
                                                        INIT_FREQUENCY,
                                                        key);
      cache_items_map[key] = value;

    }
    else {

      // update previous value of element
      cache_items_map[key] = value;

      // Touch element
      Touch(key);

    }

    // Run integrity checks
    Check();

    // return victim
    victim.block_id = victim_key;
    victim.block_type = victim_value;
    return victim;
  }

  Value Get(const Key& key){

    auto elem_it = cache_items_map.find(key);
    if (elem_it == cache_items_map.end()) {
      return INVALID_VALUE;
    }

    // Touch element
    Touch(key);

    return elem_it->second;
  }

  size_t GetSize() const{
    return cache_items_map.size();
  }

  void Print() const{

    auto current_size = cache_items_map.size();
    std::cout << "OCCUPIED: " << (current_size * 100)/capacity_ << " %\n";

    size_t block_itr = 0;
    size_t print_block_count = 100;
    for(auto& cache_item : cache_items_map){
      if(block_itr++ >= print_block_count){
        break;
      }
      std::cout << cache_item.first << CleanStatus(cache_item.second, false) << " ";
    }

    if(print_block_count > 0) {
      std::cout << "\n-------------------------------\n";
    }

  }

 private:

  std::multimap<std::size_t, Key> frequency_storage;

  std::unordered_map<Key, lfu_iterator> lfu_storage;

  std::unordered_map<Key, Value> cache_items_map;

  size_t capacity_;

};

}  // End machine namespace
