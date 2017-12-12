// STATS SOURCE

#include "stats.h"

#include <ostream>
#include <iomanip>

namespace machine {

void Stats::Reset(){
  read_ops.clear();
  write_ops.clear();
  flush_ops.clear();
  sync_ops.clear();
  movement_ops.clear();

  read_ops[DeviceType::DEVICE_TYPE_CACHE] = 0;
  read_ops[DeviceType::DEVICE_TYPE_DRAM] = 0;
  read_ops[DeviceType::DEVICE_TYPE_NVM] = 0;
  read_ops[DeviceType::DEVICE_TYPE_DISK] = 0;

  write_ops[DeviceType::DEVICE_TYPE_CACHE] = 0;
  write_ops[DeviceType::DEVICE_TYPE_DRAM] = 0;
  write_ops[DeviceType::DEVICE_TYPE_NVM] = 0;
  write_ops[DeviceType::DEVICE_TYPE_DISK] = 0;

  flush_ops[DeviceType::DEVICE_TYPE_CACHE] = 0;
  flush_ops[DeviceType::DEVICE_TYPE_DRAM] = 0;
  flush_ops[DeviceType::DEVICE_TYPE_NVM] = 0;
  flush_ops[DeviceType::DEVICE_TYPE_DISK] = 0;

  sync_ops[DeviceType::DEVICE_TYPE_CACHE] = 0;
  sync_ops[DeviceType::DEVICE_TYPE_DRAM] = 0;
  sync_ops[DeviceType::DEVICE_TYPE_NVM] = 0;
  sync_ops[DeviceType::DEVICE_TYPE_DISK] = 0;

  movement_ops[DeviceType::DEVICE_TYPE_INVALID][DeviceType::DEVICE_TYPE_CACHE] = 0;
  movement_ops[DeviceType::DEVICE_TYPE_CACHE][DeviceType::DEVICE_TYPE_DRAM] = 0;
  movement_ops[DeviceType::DEVICE_TYPE_CACHE][DeviceType::DEVICE_TYPE_NVM] = 0;
  movement_ops[DeviceType::DEVICE_TYPE_CACHE][DeviceType::DEVICE_TYPE_DISK] = 0;

  movement_ops[DeviceType::DEVICE_TYPE_DRAM][DeviceType::DEVICE_TYPE_CACHE] = 0;
  movement_ops[DeviceType::DEVICE_TYPE_DRAM][DeviceType::DEVICE_TYPE_NVM] = 0;
  movement_ops[DeviceType::DEVICE_TYPE_DRAM][DeviceType::DEVICE_TYPE_DISK] = 0;

  movement_ops[DeviceType::DEVICE_TYPE_NVM][DeviceType::DEVICE_TYPE_CACHE] = 0;
  movement_ops[DeviceType::DEVICE_TYPE_NVM][DeviceType::DEVICE_TYPE_DRAM] = 0;
  movement_ops[DeviceType::DEVICE_TYPE_NVM][DeviceType::DEVICE_TYPE_DISK] = 0;

  movement_ops[DeviceType::DEVICE_TYPE_DISK][DeviceType::DEVICE_TYPE_CACHE] = 0;
  movement_ops[DeviceType::DEVICE_TYPE_DISK][DeviceType::DEVICE_TYPE_DRAM] = 0;
  movement_ops[DeviceType::DEVICE_TYPE_DISK][DeviceType::DEVICE_TYPE_NVM] = 0;

}

void Stats::IncrementReadCount(DeviceType device_type){
  read_ops[device_type]++;
}

void Stats::IncrementWriteCount(DeviceType device_type){
  write_ops[device_type]++;
}

void Stats::IncrementFlushCount(DeviceType device_type){
  flush_ops[device_type]++;
}

void Stats::IncrementSyncCount(DeviceType device_type){
  sync_ops[device_type]++;
}

void Stats::IncrementOpCount(DeviceType source_device_type, DeviceType destination_device_type){
  movement_ops[source_device_type][destination_device_type]++;
}

std::ostream& operator<< (std::ostream& os, const Stats& stats){

  os << "READ OPS: \n";
  for(auto entry: stats.read_ops){
    os << std::setw(10) << DeviceTypeToString(entry.first) << " :: " << entry.second/1000 << " K ops\n";
  }

  os << "WRITE OPS: \n";
  for(auto entry: stats.write_ops){
    os << std::setw(10) << DeviceTypeToString(entry.first) << " :: " << entry.second/1000 << " K ops\n";
  }

  os << "FLUSH OPS: \n";
  for(auto entry: stats.flush_ops){
    os << std::setw(10) << DeviceTypeToString(entry.first) << " :: " << entry.second/1000 << " K ops\n";
  }

  os << "MOVEMENT OPS: \n";
  for(auto device_map: stats.movement_ops){
    for(auto entry: device_map.second){
      if(entry.second != 0){
        os << std::setw(10) << DeviceTypeToString(device_map.first) << " ~ " <<
            DeviceTypeToString(entry.first) << " :: " << entry.second << " ops\n";
      }
    }
  }

  return os;
}

}  // End machine namespace

