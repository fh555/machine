// CONFIGURATION HEADER

#pragma once

#include <getopt.h>
#include <sys/time.h>
#include <iostream>
#include <string>
#include <vector>

#include "types.h"
#include "device.h"

namespace machine {

static const int generator_seed = 50;

class configuration {
 public:

  // hierarchy type
  HierarchyType hierarchy_type;

  // disk mode type
  DiskModeType disk_mode_type;

  // size type
  SizeType size_type;

  // size ratio type
  SizeRatioType size_ratio_type;

  // latency type
  LatencyType latency_type;

  // caching type
  CachingType caching_type;

  // file name
  std::string file_name;

  // summary file
  std::string summary_file;

  // migration frequency
  size_t migration_frequency;

  // operation count
  size_t operation_count;

  // Verbose output
  bool verbose;

  // HDD mode
  bool hdd_mode;

  // DERIVED BASED ON HIERARCHY TYPE

  // list of devices in hierarchy
  std::vector<Device> devices;

  // list of memory devices in hierarchy
  std::vector<Device> memory_devices;

  // list of storage devices in hierarchy
  std::vector<Device> storage_devices;

  // DERIVED BASED ON LATENCY TYPE

  // nvm read latency
  size_t nvm_read_latency;

  // nvm write latency
  size_t nvm_write_latency;

};

void Usage(FILE *out);

void ParseArguments(int argc, char *argv[], configuration &state);

void ConstructDeviceList(configuration &state);

}  // namespace machine
