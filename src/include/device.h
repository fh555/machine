// DEVICE HEADER

#pragma once

#include <iostream>
#include <algorithm>

#include "storage_cache.h"
#include "timer.h"

namespace machine {

extern size_t scale_factor;

#define BLOCK_SIZE 1024

class configuration;

struct Device {


  Device(const DeviceType& device_type,
         const CachingType& caching_type,
         const size_t& device_size,
         const double& clean_fraction)
  : device_type(device_type),
    device_size(device_size),
    cache(device_type,
          caching_type,
          device_size / super_block_factor,
          clean_fraction){
    // Nothing to do here!

    std::cout << "Initialize Device: " << DeviceTypeToString(device_type)
        << " Capacity: ";
    PrintCapacity(device_size / super_block_factor);
    std::cout << "\n";

  }

  // type of the device
  DeviceType device_type = DEVICE_TYPE_INVALID;

  // size of the device (in pages)
  size_t device_size = 0;

  // storage cache
  StorageCache cache;

};

// Physical Timer
extern Timer<std::ratio<1, 1000 * 1000 * 1000>> physical_timer;

size_t GetWriteLatency(std::vector<Device>& devices,
                       DeviceType device_type,
                       const size_t& block_id,
                       const bool& flush_block);

size_t GetReadLatency(std::vector<Device>& devices,
                      DeviceType device_type,
                      const size_t& block_id);

void BootstrapDeviceMetrics(const configuration &state);

void BootstrapFileSystemForEmulation(const configuration &state);

extern bool emulate;

void Copy(std::vector<Device>& devices,
          DeviceType destination,
          DeviceType source,
          const size_t& block_id,
          const size_t& block_status,
          const bool& flush_block,
          double& total_duration);

DeviceType LocateInDevices(std::vector<Device> devices,
                           const size_t& block_id);

bool DeviceExists(std::vector<Device>& devices,
                  const DeviceType& device_type);

size_t GetDeviceOffset(std::vector<Device>& devices,
                       const DeviceType& device_type);

class DeviceFactory {
 public:
  DeviceFactory();
  virtual ~DeviceFactory();

  static Device GetDevice(const DeviceType& device_type,
                          const configuration& state,
                          const DeviceType& last_device_type);

};

}  // End machine namespace
