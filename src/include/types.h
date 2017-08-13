// TYPES HEADER

#pragma once

#include <string>

namespace machine {

enum HierarchyType {
  HIERARCHY_TYPE_INVALID = 0,

  HIERARCHY_TYPE_NVM = 1,
  HIERARCHY_TYPE_SSD = 2,
  HIERARCHY_TYPE_NVM_SSD = 3,

  HIERARCHY_TYPE_MAX = 3
};

enum SizeType {
  SIZE_TYPE_INVALID = 0,

  SIZE_TYPE_1 = 1,
  SIZE_TYPE_2 = 2,
  SIZE_TYPE_3 = 3,
  SIZE_TYPE_4 = 4,

  SIZE_TYPE_MAX = 4
};

enum SizeRatioType {
  SIZE_RATIO_TYPE_INVALID = 0,

  SIZE_RATIO_TYPE_1 = 1,
  SIZE_RATIO_TYPE_2 = 2,
  SIZE_RATIO_TYPE_3 = 3,
  SIZE_RATIO_TYPE_4 = 4,

  SIZE_RATIO_TYPE_MAX = 4
};

enum LatencyType {
  LATENCY_TYPE_INVALID = 0,

  LATENCY_TYPE_1 = 1,
  LATENCY_TYPE_2 = 2,
  LATENCY_TYPE_3 = 3,
  LATENCY_TYPE_4 = 4,

  LATENCY_TYPE_MAX = 4
};

enum CachingType {
  CACHING_TYPE_INVALID = 0,

  CACHING_TYPE_FIFO = 1,
  CACHING_TYPE_LRU = 2,
  CACHING_TYPE_LFU = 3,
  CACHING_TYPE_ARC = 4,

  CACHING_TYPE_MAX = 4
};

enum DeviceType {
  DEVICE_TYPE_INVALID = 0,

  DEVICE_TYPE_CACHE = 1,
  DEVICE_TYPE_NVM = 2,
  DEVICE_TYPE_SSD = 3

};

DeviceType GetLastDevice(const HierarchyType& hierarchy_type);

std::string HierarchyTypeToString(const HierarchyType& hierarchy_type);

std::string SizeTypeToString(const SizeType& size_type);

std::string SizeRatioTypeToString(const SizeRatioType& size_type);

std::string LatencyTypeToString(const LatencyType& latency_type);

std::string CachingTypeToString(const CachingType& caching_type);

std::string DeviceTypeToString(const DeviceType& device_type);


}  // End machine namespace
