// TYPES SOURCE

#include "types.h"

namespace machine {

std::string CachingTypeToString(const CachingType& caching_type){

  switch (caching_type){
    case CACHING_TYPE_FIFO:
      return "FIFO";
    case CACHING_TYPE_LRU:
      return "LRU";
    case CACHING_TYPE_LFU:
      return "LFU";
    case CACHING_TYPE_ARC:
      return "ARC";
    default:
      return "INVALID";
  }

}

std::string DeviceTypeToString(const DeviceType& device_type){

  switch (device_type){
    case DEVICE_TYPE_CACHE:
      return "CACHE";
    case DEVICE_TYPE_DRAM:
      return "DRAM";
    case DEVICE_TYPE_NVM:
      return "NVM";
    case DEVICE_TYPE_SSD:
      return "SSD";
    default:
      return "INVALID";
  }

}

std::string HierarchyTypeToString(const HierarchyType& hierarchy_type){

  switch (hierarchy_type) {
    case HIERARCHY_TYPE_NVM:
      return "CACHE-NVM";
    case HIERARCHY_TYPE_DRAM_NVM:
      return "CACHE-DRAM-NVM";
    case HIERARCHY_TYPE_DRAM_SSD:
      return "CACHE-DRAM-SSD";
    case HIERARCHY_TYPE_NVM_SSD:
      return "CACHE-NVM-SSD";
    case HIERARCHY_TYPE_DRAM_NVM_SSD:
      return "CACHE-DRAM-NVM-SSD";
    case HIERARCHY_TYPE_DRAM_HDD:
      return "CACHE-DRAM-HDD";
    case HIERARCHY_TYPE_NVM_HDD:
      return "CACHE-NVM-HDD";
    case HIERARCHY_TYPE_DRAM_NVM_HDD:
      return "CACHE-DRAM-NVM-HDD";
    default:
      return "INVALID";
  }

}

std::string SizeTypeToString(const SizeType& size_type){

  switch (size_type) {
    case SIZE_TYPE_1:
      return "1";
    case SIZE_TYPE_2:
      return "2";
    case SIZE_TYPE_3:
      return "3";
    case SIZE_TYPE_4:
      return "4";
    case SIZE_TYPE_5:
      return "5";
    case SIZE_TYPE_6:
      return "6";
    case SIZE_TYPE_7:
      return "7";
    case SIZE_TYPE_8:
      return "8";
    default:
      return "INVALID";
  }

}

std::string LatencyTypeToString(const LatencyType& latency_type){

  switch (latency_type) {
    case LATENCY_TYPE_1:
      return "1";
    case LATENCY_TYPE_2:
      return "2";
    case LATENCY_TYPE_3:
      return "3";
    case LATENCY_TYPE_4:
      return "4";
    case LATENCY_TYPE_5:
      return "5";
    case LATENCY_TYPE_6:
      return "6";
    default:
      return "INVALID";
  }

}

DeviceType GetLastDevice(const HierarchyType& hierarchy_type){

  switch (hierarchy_type) {
     case HIERARCHY_TYPE_NVM:
       return DeviceType::DEVICE_TYPE_NVM;
     case HIERARCHY_TYPE_DRAM_NVM:
       return DeviceType::DEVICE_TYPE_NVM;
     case HIERARCHY_TYPE_DRAM_SSD:
       return DeviceType::DEVICE_TYPE_SSD;
     case HIERARCHY_TYPE_DRAM_NVM_SSD:
       return DeviceType::DEVICE_TYPE_SSD;
     default:
       return DeviceType::DEVICE_TYPE_INVALID;
   }

}


}  // End machine namespace

