// CACHE POLICY HEADER

#pragma once

#include <unordered_set>

#include "macros.h"

namespace machine {

template <typename Key>
class ICachePolicy {
 public:

  virtual ~ICachePolicy() {}

  // handle element insertion in a cache
  virtual void Insert(const Key& key) = 0;

  // handle request to the key-element in a cache
  virtual void Touch(const Key& key) = 0;

  // handle element deletion from a cache
  virtual void Erase(const Key& key) = 0;

  // return a key of a replacement candidate
  virtual const Key& Victim() const = 0;

};

template <typename Key>
class NoCachePolicy : public ICachePolicy<Key> {
 public:

  NoCachePolicy(UNUSED_ATTRIBUTE const size_t& max_size){
    // Nothing to do here!
  }

  ~NoCachePolicy() override = default;

  void Insert(const Key& key) override {
    key_storage.emplace(key);
  }

  void Touch(const Key& key) override {
    // Nothing to do here!
  }

  void Erase(const Key& key) override {
    key_storage.erase(key);
  }

  // return a key of a displacement candidate
  const Key& Victim() const override {
    return *key_storage.crbegin();
  }

 private:

  std::unordered_set<Key> key_storage;

};

}  // End machine namespace
