// LRU HEADER

#pragma once

#include <list>
#include <unordered_map>

#include "macros.h"
#include "policy.h"

namespace machine {

template <typename Key, typename Value>
class LRUCachePolicy : public ICachePolicy<Key, Value> {
 public:
  using lru_iterator = typename std::list<Key>::iterator;

  LRUCachePolicy(const size_t& capacity)
  : capacity_(capacity){
    // Nothing to do here!
  }

  ~LRUCachePolicy() = default;

  void Check(){

    if (GetSize() > capacity_) {
      std::cout << "Capacity exceeded \n";
      exit(EXIT_FAILURE);
    }

  }

  void Touch(const Key& key) {

    // check if key exists
    if(key_finder.count(key) == 0){
      std::cout << "KEY NOT FOUND: " << key << "\n";
      exit(EXIT_FAILURE);
    }

    // move the touched element at the beginning of the lru_queue
    lru_queue.erase(key_finder[key]);
    lru_queue.emplace_front(key);
    key_finder[key] = lru_queue.begin();

  }

  Block Put(const Key& key, const Value& value){
    Block victim;
    Key victim_key = INVALID_KEY;
    Value victim_value = INVALID_KEY;

    auto entry_location = cache_items_map.find(key);
    if (entry_location == cache_items_map.end()) {
      // add new element to the cache

      // check capacity
      if (GetSize() + 1 > capacity_) {
        victim_key = lru_queue.back();
        victim_value = cache_items_map[victim_key];
        //std::cout << "Victim: " << victim_key << "\n";

        // evict victim
        key_finder.erase(victim_key);
        lru_queue.pop_back();
        cache_items_map.erase(victim_key);
      }

      // insert new element
      lru_queue.emplace_front(key);
      key_finder[key] = lru_queue.begin();
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
      throw std::range_error{"No such element in the cache"};
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

  std::list<Key> lru_queue;

  std::unordered_map<Key, lru_iterator> key_finder;

  std::unordered_map<Key, Value> cache_items_map;

  size_t capacity_;

};

}  // End machine namespace
