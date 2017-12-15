// TYPES SOURCE

#include "types.h"

namespace machine {

std::string CachingTypeToString(const CachingType& caching_type){

  switch (caching_type){
    case CACHING_TYPE_FIFO:
      return "FIFO";
    case CACHING_TYPE_LFU:
      return "LFU";
    case CACHING_TYPE_LRU:
      return "LRU";
    case CACHING_TYPE_ARC:
      return "ARC";
    case CACHING_TYPE_HARC:
      return "HARC";
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
    case DEVICE_TYPE_DISK:
      return "DISK";
    default:
      return "INVALID";
  }

}

std::string DiskModeTypeToString(const DiskModeType& disk_mode_type) {

  switch (disk_mode_type){
    case DISK_MODE_TYPE_SSD:
      return "SSD";
    case DISK_MODE_TYPE_HDD:
      return "HDD";
    default:
      return "INVALID";
  }

}

std::string HierarchyTypeToString(const HierarchyType& hierarchy_type){

  switch (hierarchy_type) {
    case HIERARCHY_TYPE_NVM:
      return "NVM";
    case HIERARCHY_TYPE_DRAM_NVM:
      return "DRAM-NVM";
    case HIERARCHY_TYPE_DRAM_DISK:
      return "DRAM-DISK";
    case HIERARCHY_TYPE_NVM_DISK:
      return "NVM-DISK";
    case HIERARCHY_TYPE_DRAM_NVM_DISK:
      return "DRAM-NVM-DISK";
    default:
      return "INVALID";
  }

}

std::string SizeTypeToString(const SizeType& size_type){

  switch (size_type) {
    case SIZE_TYPE_1:
      return "SIZE-1";
    case SIZE_TYPE_2:
      return "SIZE-2";
    case SIZE_TYPE_3:
      return "SIZE-3";
    case SIZE_TYPE_4:
      return "SIZE-4";
    default:
      return "INVALID";
  }

}

std::string SizeRatioTypeToString(const SizeRatioType& size_ratio_type){

  switch (size_ratio_type) {
    case SIZE_RATIO_TYPE_1:
      return "RATIO-1";
    case SIZE_RATIO_TYPE_2:
      return "RATIO-2";
    case SIZE_RATIO_TYPE_3:
      return "RATIO-3";
    case SIZE_RATIO_TYPE_4:
      return "RATIO-4";
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
    default:
      return "INVALID";
  }

}

DeviceType GetLastDevice(const HierarchyType& hierarchy_type){

  switch (hierarchy_type) {
    case HIERARCHY_TYPE_NVM:
    case HIERARCHY_TYPE_DRAM_NVM:
      return DeviceType::DEVICE_TYPE_NVM;

    case HIERARCHY_TYPE_DRAM_DISK:
    case HIERARCHY_TYPE_NVM_DISK:
    case HIERARCHY_TYPE_DRAM_NVM_DISK:
      return DeviceType::DEVICE_TYPE_DISK;

    default:
       return DeviceType::DEVICE_TYPE_INVALID;
   }

}


}  // End machine namespace

