// STATS HEADER

#pragma once

#include <map>
#include <utility>

#include "types.h"

namespace machine {

// STATS

class Stats{

 public:

  void Reset();

  void IncrementReadCount(DeviceType device_type);

  void IncrementWriteCount(DeviceType device_type);

  void IncrementFlushCount(DeviceType device_type);

  void IncrementSyncCount(DeviceType device_type);

  void IncrementOpCount(DeviceType source_device_type, DeviceType destination_device_type);

  friend std::ostream& operator<< (std::ostream& stream, const Stats& stats);

 //private:

  // Read op count
  std::map<DeviceType, size_t> read_ops;

  // Write op count
  std::map<DeviceType, size_t> write_ops;

  // Flush op count
  std::map<DeviceType, size_t> flush_ops;

  // Sync op count
  std::map<DeviceType, size_t> sync_ops;

  // Op tracker
  std::map<DeviceType, std::map<DeviceType, size_t>> movement_ops;

};

}  // End machine namespace
