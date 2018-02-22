// STORAGE CACHE SOURCE

#include <iostream>

#include "storage_cache.h"

namespace machine {

StorageCache::StorageCache(DeviceType device_type,
                           CachingType caching_type,
                           size_t capacity,
                           double clean_fraction) :
                           device_type_(device_type),
                           caching_type_(caching_type),
                           capacity_(capacity),
                           clean_fraction_(clean_fraction){

  //std::cout << "STORAGE CACHE CAPACITY: " << capacity << "\n";

  switch(caching_type_){

    case CACHING_TYPE_FIFO:
      fifo_cache = new Cache<int, int, FIFOCachePolicy<int, int>>(capacity, clean_fraction);
      break;

    case CACHING_TYPE_LFU:
      lfu_cache = new Cache<int, int, LFUCachePolicy<int, int>>(capacity, clean_fraction);
      break;

    case CACHING_TYPE_LRU:
      lru_cache = new Cache<int, int, LRUCachePolicy<int, int>>(capacity, clean_fraction);
      break;

    case CACHING_TYPE_ARC:
      arc_cache = new Cache<int, int, ARCCachePolicy<int, int>>(capacity, clean_fraction);
      break;

    case CACHING_TYPE_INVALID:
    default:
      exit(EXIT_FAILURE);
  }

}

Block StorageCache::Put(const int& key, const int& value){

  Block victim;

  switch(caching_type_){

    case CACHING_TYPE_FIFO:
      victim = fifo_cache->Put(key, value);
      break;

    case CACHING_TYPE_LFU:
      victim = lfu_cache->Put(key, value);
      break;

    case CACHING_TYPE_LRU:
      victim = lru_cache->Put(key, value);
      break;

    case CACHING_TYPE_ARC:
      victim = arc_cache->Put(key, value);
      break;

    case CACHING_TYPE_INVALID:
    default:
      exit(EXIT_FAILURE);
  }

  if(victim.block_id != INVALID_KEY){
    if(victim.block_type != CLEAN_BLOCK &&
        victim.block_type != DIRTY_BLOCK ){
      std::cout << "Invalid block type : " << victim.block_type;
      std::cout << DeviceTypeToString(device_type_);
      std::cout << CachingTypeToString(caching_type_);
      exit(EXIT_FAILURE);
    }
  }

  return victim;

}

int StorageCache::Get(const int& key) const{

  switch(caching_type_){

    case CACHING_TYPE_FIFO:
      return fifo_cache->Get(key);

    case CACHING_TYPE_LFU:
      return lfu_cache->Get(key);

    case CACHING_TYPE_LRU:
      return lru_cache->Get(key);

    case CACHING_TYPE_ARC:
      return arc_cache->Get(key);

    case CACHING_TYPE_INVALID:
    default:
      exit(EXIT_FAILURE);
  }

}

size_t StorageCache::GetSize() const{

  switch(caching_type_){

    case CACHING_TYPE_FIFO:
      return fifo_cache->GetSize();

    case CACHING_TYPE_LFU:
      return lfu_cache->GetSize();

    case CACHING_TYPE_LRU:
      return lru_cache->GetSize();

    case CACHING_TYPE_ARC:
      return arc_cache->GetSize();

    case CACHING_TYPE_INVALID:
    default:
      exit(EXIT_FAILURE);
  }

}

std::ostream& operator<< (std::ostream& stream,
                          const StorageCache& cache){

  std::cout << "-------------------------------\n";
  std::cout << "[" << DeviceTypeToString(cache.device_type_) << "] ";
  std::cout << "[" << CachingTypeToString(cache.caching_type_) <<"] ";

  PrintCapacity(cache.capacity_);

  switch(cache.caching_type_){

    case CACHING_TYPE_FIFO:
      cache.fifo_cache->Print();
      return stream;

    case CACHING_TYPE_LFU:
      cache.lfu_cache->Print();
      return stream;

    case CACHING_TYPE_LRU:
      cache.lru_cache->Print();
      return stream;

    case CACHING_TYPE_ARC:
      cache.arc_cache->Print();
      return stream;

    case CACHING_TYPE_INVALID:
    default:
      exit(EXIT_FAILURE);
  }

}

bool StorageCache::IsSequential(const size_t& next){

  switch(caching_type_){

    case CACHING_TYPE_FIFO:
      return fifo_cache->IsSequential(next);

    case CACHING_TYPE_LFU:
      return lfu_cache->IsSequential(next);

    case CACHING_TYPE_LRU:
      return lru_cache->IsSequential(next);

    case CACHING_TYPE_ARC:
      return arc_cache->IsSequential(next);

    case CACHING_TYPE_INVALID:
    default:
      exit(EXIT_FAILURE);
  }

}

}  // End machine namespace

