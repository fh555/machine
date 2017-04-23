// STATS SOURCE

#include "stats.h"

#include <ostream>
#include <iomanip>

namespace machine {

void Stats::Reset(){
  read_ops.clear();
  write_ops.clear();

  read_ops[DeviceType::DEVICE_TYPE_CACHE] = 0;
  read_ops[DeviceType::DEVICE_TYPE_DRAM] = 0;
  read_ops[DeviceType::DEVICE_TYPE_NVM] = 0;
  read_ops[DeviceType::DEVICE_TYPE_SSD] = 0;

  write_ops[DeviceType::DEVICE_TYPE_CACHE] = 0;
  write_ops[DeviceType::DEVICE_TYPE_DRAM] = 0;
  write_ops[DeviceType::DEVICE_TYPE_NVM] = 0;
  write_ops[DeviceType::DEVICE_TYPE_SSD] = 0;
}

void Stats::IncrementReadCount(DeviceType device_type){
  read_ops[device_type]++;
}

void Stats::IncrementWriteCount(DeviceType device_type){
  write_ops[device_type]++;
}

std::ostream& operator<< (std::ostream& os, const Stats& stats){

  os << "READ OPS: \n";
  for(auto entry: stats.read_ops){
    os << std::setw(10) << DeviceTypeToString(entry.first) << " :: " << entry.second << "\n";
  }

  os << "WRITE OPS: \n";
  for(auto entry: stats.write_ops){
    os << std::setw(10) << DeviceTypeToString(entry.first) << " :: " << entry.second << "\n";
  }

  return os;
}

}  // End machine namespace

