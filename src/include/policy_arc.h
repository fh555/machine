// ARC HEADER

#pragma once

#include <deque>
#include <vector>
#include <algorithm>
#include <iostream>

#include <glog/logging.h>

#include "macros.h"
#include "policy.h"

namespace machine {

#define MAX(a,b) (((a)>(b))?(a):(b))

template <typename Key, typename Value>
class ARCCachePolicy : public ICachePolicy<Key, Value> {
 public:

  ARCCachePolicy(const size_t& capacity)
 : capacity_(capacity),
   p(0){
    // Nothing to do here!
  }

  ~ARCCachePolicy() = default;

  bool DequeContains(const std::deque<Key>& deque, const Key& key) const {
    auto location = std::find(deque.begin(), deque.end(), key);
    if (location != deque.end()) {
      return true;
    }
    return false;
  }

  void DequeErase(std::deque<Key>& deque, const Key& key){
    auto location = std::find(deque.begin(), deque.end(), key);
    if (location != deque.end()) {
      deque.erase(location);
    }
  }

  void DequePrint(std::string deque_name, const std::deque<Key>& deque) const{
    std::stringstream str;
    str << deque_name << " :: ";
    for(auto entry : deque){
      str << entry << " ";
    }
    str << "\n";
    DLOG(INFO) << str.str();
  }

  void Check(){

    // Print
    DLOG(INFO) << "\n+++++++++++++++++++++++++++++++++++++\n";
    //DequePrint("T1", T1);
    //DequePrint("B1",B1);
    //DequePrint("T2",T2);
    //DequePrint("B2",B2);
    DLOG(INFO) << "+++++++++++++++++++++++++++++++++++++\n";

    if (GetSize() > capacity_) {
      DLOG(INFO) << "Capacity exceeded \n";
      exit(EXIT_FAILURE);
    }

    if(p > capacity_ || p < 0){
      DLOG(INFO) << "p exceeds capacity \n";
      exit(EXIT_FAILURE);
    }

    if(T1.size() + B1.size() > capacity_){
      DLOG(INFO) << "L1 exceeds capacity \n";
      exit(EXIT_FAILURE);
    }

    if(T1.size() + B1.size() + T2.size() + B2.size() > 2 * capacity_){
      DLOG(INFO) << "L1 + L2 exceeds 2 * capacity \n";
      exit(EXIT_FAILURE);
    }

  }

  void Touch(const Key& key) {

    // check if key exists
    if(cache_items_map.count(key) == 0){
      DLOG(INFO) << "KEY NOT FOUND: " << key << "\n";
      exit(EXIT_FAILURE);
    }

    if (DequeContains(T1, key)) {
      DLOG(INFO) << "T1 contains key\n";
      DequeErase(T1, key);
      T2.push_front(key);
      DLOG(INFO) << "Move from T1 to T2\n";
    }
    else if (DequeContains(T2, key)){
      DLOG(INFO) << "T2 contains key\n";
      DequeErase(T2, key);
      T2.push_front(key);
      DLOG(INFO) << "Move from T1 to T2\n";
    }
    else{
      DLOG(INFO) << "KEY NOT FOUND IN T1 & T2: " << key << "\n";
      exit(EXIT_FAILURE);
    }

  }

  Key Replace(const Key& key) {
    DLOG(INFO) << "ARC REPLACE : " << key << "\n";

    Key victim_key = INVALID_KEY;
    bool T1_not_empty = (T1.empty() == false);
    bool in_B2 = DequeContains(B2, key);
    bool len_T1_eq_P = (T1.size() == p);
    bool len_T1_gt_P = (T1.size() > p);

    if(T1_not_empty && ((len_T1_eq_P && in_B2) || len_T1_gt_P)){
      DLOG(INFO) << "Evict from T1 to B1\n";
      victim_key = T1.back();
      T1.pop_back();
      B1.push_front(victim_key);
    }
    else {
      DLOG(INFO) << "Evict from T2 to B2\n";
      victim_key = T2.back();
      T2.pop_back();
      B2.push_front(victim_key);
    }

    return victim_key;
  }

  Block Put(const Key& key, const Value& value) {
    Block victim;
    Key victim_key = INVALID_KEY;
    Value victim_value = INVALID_KEY;

    // Check if key in T1
    if (DequeContains(T1, key)) {
      DLOG(INFO) << "T1 contains key\n";
      DequeErase(T1, key);
      T2.push_front(key);
      DLOG(INFO) << "Move from T1 to T2\n";
    }
    // Check if key in T2
    else if (DequeContains(T2, key)){
      DLOG(INFO) << "T2 contains key\n";
      DequeErase(T2, key);
      T2.push_front(key);
      DLOG(INFO) << "Keep in T2\n";
    }
    // Check if key in B1
    else if(DequeContains(B1, key)){
      DLOG(INFO) << "B1 contains key\n";
      size_t size_ratio = B2.size()/B1.size();
      size_t b_ratio = MAX(size_ratio, 1);
      p = std::min(capacity_, p + b_ratio);
      DLOG(INFO) << "Adapt p\n";

      victim_key = Replace(key);

      DequeErase(B1, key);
      T2.push_front(key);
      DLOG(INFO) << "Move from B1 to T2\n";
    }
    // Check if key in B2
    else if(DequeContains(B2, key)){
      DLOG(INFO) << "B2 contains key\n";
      size_t size_ratio = B1.size()/B2.size();
      size_t b_ratio = MAX(size_ratio, 1);
      if(p >= b_ratio) {
        p = MAX(0, p - b_ratio);
      }
      DLOG(INFO) << "Adapt p\n";

      victim_key = Replace(key);

      DequeErase(B2, key);
      T2.push_front(key);
      DLOG(INFO) << "Move from B2 to T2\n";
    }
    // Key not found in T1, T2, B1, and B2
    else {
      DLOG(INFO) << "Completely new key\n";

      auto l1 = T1.size() + B1.size();
      auto l2 = T2.size() + B2.size();
      auto l1_plus_l2 = l1 + l2;

      // CASE 1: |L1| = c
      if(l1 == capacity_){
        if(T1.size() < capacity_){
          B1.pop_back();
          victim_key = Replace(key);
          DLOG(INFO) << "Make space in B1\n";
        }
        else {
          victim_key = T1.back();
          T1.pop_back();
          DLOG(INFO) << "Make space in T1\n";
        }
      }
      // CASE 2: |L1|< c and |L1|+ |L2|≥ c
      else if(l1 < capacity_ && l1_plus_l2 >= capacity_) {
        if(l1_plus_l2 == 2 * capacity_){
          B2.pop_back();
          DLOG(INFO) << "Make space in B2\n";
        }

        victim_key = Replace(key);
      }

      T1.push_front(key);
      DLOG(INFO) << "Move to T1\n";
    }

    // update value of element
    cache_items_map[key] = value;

    // evict victim
    if(victim_key != INVALID_KEY){
      victim_value = cache_items_map[victim_key];
      cache_items_map.erase(victim_key);
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

    // Run integrity checks
    //Check();

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

  std::deque<Key> T1;
  std::deque<Key> B1;

  std::deque<Key> T2;
  std::deque<Key> B2;

  std::unordered_map<Key, Value> cache_items_map;

  size_t capacity_;

  // marker
  size_t p;

};

}  // End machine namespace
