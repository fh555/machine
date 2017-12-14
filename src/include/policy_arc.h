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

template <typename Key>
class ARCCachePolicy : public ICachePolicy<Key> {
 public:

  ARCCachePolicy(const size_t& capacity)
 : capacity(capacity),
   p(0) {
    // Nothing to do here!
  }

  ~ARCCachePolicy() = default;

  void Insert(const Key& key) override {

    DLOG(INFO) << "ARC INSERT : " << key << "\n";

    if(Contains(B1, key)){
      DLOG(INFO) << "B1 contains key\n";
      size_t size_ratio = B2.size()/B1.size();
      size_t b_ratio = MAX(size_ratio, 1);
      p = std::min(capacity, p + b_ratio);
      Replace(key);
      DequeErase(B1, key);
      T2.push_front(key);
      DLOG(INFO) << "Moved it to T2\n";
    }
    else if(Contains(B2, key)){
      DLOG(INFO) << "B2 contains key\n";
      size_t size_ratio = B1.size()/B2.size();
      size_t b_ratio = MAX(size_ratio, 1);
      if(p >= b_ratio) {
        p = MAX(0, p - b_ratio);
      }
      Replace(key);
      DequeErase(B2, key);
      T2.push_front(key);
      DLOG(INFO) << "Moved it to T2\n";
    }
    else {
      DLOG(INFO) << "Completely new\n";

      auto l1 = T1.size() + B1.size();
      auto l1_plus_l2 = T1.size() + T2.size() + B1.size() + B2.size();

      if(l1 == capacity){
        if(T1.size() < capacity){
          B1.pop_back();
          Replace(key);
          DLOG(INFO) << "Make space in B1\n";
        }
        else {
          T1.pop_back();
          DLOG(INFO) << "Make space in T1\n";
        }
      }
      else if(l1 < capacity && l1_plus_l2 >= capacity) {
        if(l1_plus_l2 == 2 * capacity){
          B2.pop_back();
          DLOG(INFO) << "Make space in B2\n";
        }
        Replace(key);
      }

      T1.push_front(key);
      DLOG(INFO) << "Moved it to T1\n";

    }

    Check();

  }

  void Touch(const Key& key) override {

    DLOG(INFO) << "ARC TOUCH : " << key << "\n";

    if (Contains(T1, key)) {
      DLOG(INFO) << "T1 contains key\n";
      DequeErase(T1, key);
      T2.push_front(key);
      DLOG(INFO) << "Moved it to T2\n";
    }
    else if (Contains(T2, key)){
      DLOG(INFO) << "T2 contains key\n";
      DequeErase(T2, key);
      T2.push_front(key);
      DLOG(INFO) << "Moved it to T2\n";
    }

    Check();

  }

  void Replace(const Key& key) {

    DLOG(INFO) << "ARC REPLACE : " << key << "\n";

    bool T1_not_empty = (T1.empty() == false);
    bool in_B2 = Contains(B2, key);
    bool len_T1_eq_P = (T1.size() == p);
    bool len_T1_gt_P = (T1.size() > p);

    if(T1_not_empty && ((in_B2 && len_T1_eq_P) || len_T1_gt_P)){
      DLOG(INFO) << "Evict from T1 to B1\n";
      auto victim = T1.back();
      T1.pop_back();
      B1.push_front(victim);
    }
    else {
      DLOG(INFO) << "Evict from T2 to B2\n";
      auto victim = T2.back();
      T2.pop_back();
      B2.push_front(victim);
    }

    Check();

  }

  void Erase(UNUSED_ATTRIBUTE const Key& key) override {
    // Nothing to do here!
  }

  // return a key of a displacement candidate
  const Key& Victim(UNUSED_ATTRIBUTE const Key& key) const override {

    bool T1_not_empty = (T1.empty() == false);
    bool in_B2 = Contains(B2, key);
    bool len_T1_eq_P = (T1.size() == p);
    bool len_T1_gt_P = (T1.size() > p);

    if(T1_not_empty && ((in_B2 && len_T1_eq_P) || len_T1_gt_P)){
      DLOG(INFO) << "ARC VICTIM : " << T1.back() << "\n";
      return T1.back();
    }
    else {
      DLOG(INFO) << "ARC VICTIM : " << T2.back() << "\n";
      return T2.back();
    }

  }

  void Check() const{

    // Disable checks for now
    return;

    // Print
    //DLOG(INFO) << "\n+++++++++++++++++++++++++++++++++++++\n";
    //Print("T1", T1);
    //Print("B1",B1);
    //Print("T2",T2);
    //Print("B2",B2);
    //DLOG(INFO) << "+++++++++++++++++++++++++++++++++++++\n";

    if(p > capacity || p < 0){
      LOG(INFO) << "p exceeds capacity \n";
      exit(EXIT_FAILURE);
    }

    if(T1.size() + B1.size() > capacity){
      LOG(INFO) << "L1 exceeds capacity \n";
      exit(EXIT_FAILURE);
    }

    if(T1.size() + B1.size() + T2.size() + B2.size() > 2 * capacity){
      LOG(INFO) << "L1 + L2 exceeds 2 * capacity \n";
      exit(EXIT_FAILURE);
    }

  }

  void Print(std::string deque_name, const std::deque<Key>& deque) const{
    std::stringstream str;
    str << deque_name << " :: ";
    for(auto entry : deque){
      str << entry << " ";
    }
    str << "\n";
    std::cout << str.str();
  }

  bool Contains(const std::deque<Key>& deque, const Key& key) const {
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

 private:

  std::deque<Key> T1;
  std::deque<Key> B1;

  std::deque<Key> T2;
  std::deque<Key> B2;

  // capacity of cache
  size_t capacity;

  // marker
  size_t p;

};

}  // End machine namespace
