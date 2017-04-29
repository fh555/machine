// LRU HEADER

#pragma once

#include <list>
#include <unordered_map>

#include "macros.h"
#include "policy.h"

namespace machine {

template <typename Key>
class LRUCachePolicy : public ICachePolicy<Key> {
 public:
  using lru_iterator = typename std::list<Key>::const_iterator;

  LRUCachePolicy(UNUSED_ATTRIBUTE const size_t& capacity){
    // Nothing to do here!
  }

  ~LRUCachePolicy() = default;

  void Insert(const Key& key) override {

    DLOG(INFO) << "LRU INSERT: " << key << "\n";

    lru_queue.emplace_front(key);
    key_finder[key] = lru_queue.cbegin();

  }

  void Touch(const Key& key) override {

    // move the touched element at the beginning of the lru_queue
    lru_queue.erase(key_finder[key]);
    lru_queue.emplace_front(key);
    key_finder[key] = lru_queue.cbegin();

  }

  void Erase(UNUSED_ATTRIBUTE const Key& key) override {

    DLOG(INFO) << "LRU ERASE: " << lru_queue.back() << "\n";

    // remove the least recently used element
    key_finder.erase(lru_queue.back());
    lru_queue.pop_back();

  }

  // return a key of a displacement candidate
  const Key& Victim(UNUSED_ATTRIBUTE const Key& key) const override {

    DLOG(INFO) << "LRU VICTIM: " << lru_queue.back() << "\n";

    return lru_queue.back();

  }

 private:

  std::list<Key> lru_queue;

  std::unordered_map<Key, lru_iterator> key_finder;

};

}  // End machine namespace
