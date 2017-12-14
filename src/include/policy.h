// CACHE POLICY HEADER

#pragma once

#include <unordered_set>

#include "macros.h"

namespace machine {

struct Block{
  size_t block_id;
  size_t block_type;
};

template <typename Key, typename Value>
class ICachePolicy {
 public:

  virtual ~ICachePolicy() {}

  virtual Block Put(const Key& key, const Value& value) = 0;

  virtual Value Get(const Key& key) = 0;

  virtual size_t GetSize() const = 0;

  virtual void Print() const = 0;

};

}  // End machine namespace
