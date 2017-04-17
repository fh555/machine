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

  // size type
  SizeType size_type;

  // list of devices in hierarchy
  std::vector<Device> devices;

  // list of memory devices in hierarchy
  std::vector<Device> memory_devices;

  // list of storage devices in hierarchy
  std::vector<Device> storage_devices;

  // logging type
  LoggingType logging_type;

  // caching type
  CachingType caching_type;

  // file name
  std::string file_name;

  // upwards migration frequency
  size_t migration_frequency;

  // machine size
  size_t machine_size;

  // operation count
  size_t operation_count;

  // nvm read latency
  size_t nvm_read_latency;

  // nvm write latency
  size_t nvm_write_latency;

  // Verbose output
  bool verbose;

};

void Usage(FILE *out);

void ParseArguments(int argc, char *argv[], configuration &state);

void ConstructDeviceList(configuration &state);

}  // namespace machine
