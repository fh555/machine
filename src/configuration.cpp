// CONFIGURATION SOURCE

#include <algorithm>
#include <iomanip>

#include "configuration.h"
#include "cache.h"
#include "device.h"

namespace machine {

void Usage() {
  std::cout <<
      "\n"
      "Command line options : machine <options>\n"
      "   -a --hierarchy_type                 :  hierarchy type\n"
      "   -s --size_type                      :  size type\n"
      "   -l --latency_type                   :  latency type\n"
      "   -c --caching_type                   :  caching type\n"
      "   -f --file_name                      :  file name\n"
      "   -m --migration_frequency            :  migration frequency\n"
      "   -o --operation_count                :  operation count\n"
      "   -v --verbose                        :  verbose\n";
  exit(EXIT_FAILURE);
}

static struct option opts[] = {
    {"hierarchy_type", optional_argument, NULL, 'a'},
    {"size_type", optional_argument, NULL, 's'},
    {"latency_type", optional_argument, NULL, 'l'},
    {"caching_type", optional_argument, NULL, 'c'},
    {"file_name", optional_argument, NULL, 'f'},
    {"migration_frequency", optional_argument, NULL, 'm'},
    {"operation_count", optional_argument, NULL, 'o'},
    {"verbose", optional_argument, NULL, 'v'},
    {NULL, 0, NULL, 0}
};

static void ValidateHierarchyType(const configuration &state) {
  if (state.hierarchy_type < 1 || state.hierarchy_type > 8) {
    printf("Invalid hierarchy_type :: %d\n", state.hierarchy_type);
    exit(EXIT_FAILURE);
  }
  else {
    printf("%30s : %s\n", "caching_type",
           HierarchyTypeToString(state.hierarchy_type).c_str());
  }
}

static void ValidateSizeType(const configuration &state) {
  if (state.size_type < 1 || state.size_type > 8) {
    printf("Invalid size_type :: %d\n", state.size_type);
    exit(EXIT_FAILURE);
  }
  else {
    printf("%30s : %s\n", "size_type",
           SizeTypeToString(state.size_type).c_str());
  }
}

static void ValidateCachingType(const configuration &state) {
  if (state.caching_type < 1 || state.caching_type > 4) {
    printf("Invalid caching_type :: %d\n", state.caching_type);
    exit(EXIT_FAILURE);
  }
  else {
    printf("%30s : %s\n", "caching_type",
           CachingTypeToString(state.caching_type).c_str());
  }
}

static void ValidateFileName(const configuration &state){
  printf("%30s : %s\n", "file_name", state.file_name.c_str());
}

static void ValidateLatencyType(const configuration &state) {
  if (state.latency_type < 1 || state.latency_type > 5) {
    printf("Invalid latency_type :: %d\n", state.latency_type);
    exit(EXIT_FAILURE);
  }
  else {
    printf("%30s : %s\n", "latency_type",
           LatencyTypeToString(state.latency_type).c_str());
  }
}

static void ValidateMigrationFrequency(const configuration &state){
  printf("%30s : %lu\n", "migration_frequency", state.migration_frequency);
}

static void ValidateNVMReadLatency(const configuration &state){
  printf("%30s : %lu\n", "nvm_read_latency", state.nvm_read_latency);
}

static void ValidateNVMWriteLatency(const configuration &state){
  printf("%30s : %lu\n", "nvm_write_latency", state.nvm_write_latency);
}

static void ValidateOperationCount(const configuration &state){
  if(state.operation_count > 0) {
    printf("%30s : %lu\n", "operation_count", state.operation_count);
  }
}

void SetupNVMLatency(configuration &state){

  switch(state.latency_type){
    case LATENCY_TYPE_1: {
      state.nvm_read_latency = 2;
      state.nvm_write_latency = 4;
      break;
    }

    case LATENCY_TYPE_2: {
      state.nvm_read_latency = 2;
      state.nvm_write_latency = 10;
      break;
    }

    case LATENCY_TYPE_3: {
      state.nvm_read_latency = 4;
      state.nvm_write_latency = 4;
      break;
    }

    case LATENCY_TYPE_4: {
      state.nvm_read_latency = 4;
      state.nvm_write_latency = 8;
      break;
    }

    case LATENCY_TYPE_5: {
      state.nvm_read_latency = 8;
      state.nvm_write_latency = 8;
      break;
    }

    default:
      std::cout << "Invalid latency type: " << state.latency_type << "\n";
      exit(EXIT_FAILURE);
      break;
  }

}


void ConstructDeviceList(configuration &state){

  auto last_device_type = GetLastDevice(state.hierarchy_type);
  Device cache_device = DeviceFactory::GetDevice(DEVICE_TYPE_CACHE,
                                                 state.size_type,
                                                 state.caching_type,
                                                 last_device_type);
  Device dram_device = DeviceFactory::GetDevice(DEVICE_TYPE_DRAM,
                                                state.size_type,
                                                state.caching_type,
                                                last_device_type);
  Device nvm_device = DeviceFactory::GetDevice(DEVICE_TYPE_NVM,
                                               state.size_type,
                                               state.caching_type,
                                               last_device_type);
  Device ssd_device = DeviceFactory::GetDevice(DEVICE_TYPE_SSD,
                                               state.size_type,
                                               state.caching_type,
                                               last_device_type);

  switch (state.hierarchy_type) {
    case HIERARCHY_TYPE_NVM: {
      state.devices = {cache_device, nvm_device};
      state.memory_devices = {cache_device, nvm_device};
      state.storage_devices = {nvm_device};
    }
    break;
    case HIERARCHY_TYPE_DRAM_NVM: {
      state.devices = {cache_device, dram_device, nvm_device};
      state.memory_devices = {cache_device, dram_device, nvm_device};
      state.storage_devices = {nvm_device};
    }
    break;
    case HIERARCHY_TYPE_DRAM_SSD:
    case HIERARCHY_TYPE_DRAM_HDD: {
      state.devices = {cache_device, dram_device, ssd_device};
      state.memory_devices = {cache_device, dram_device};
      state.storage_devices = {ssd_device};
    }
    break;
    case HIERARCHY_TYPE_NVM_SSD:
    case HIERARCHY_TYPE_NVM_HDD: {
      state.devices = {cache_device, nvm_device, ssd_device};
      state.memory_devices = {cache_device, nvm_device};
      state.storage_devices = {ssd_device};
    }
    break;
    case HIERARCHY_TYPE_DRAM_NVM_SSD:
    case HIERARCHY_TYPE_DRAM_NVM_HDD: {
      state.devices = {cache_device, dram_device, nvm_device, ssd_device};
      state.memory_devices = {cache_device, dram_device, nvm_device};
      state.storage_devices = {nvm_device, ssd_device};
    }
    break;
    default:
      break;
  }

}


void ParseArguments(int argc, char *argv[], configuration &state) {

  // Default Values
  state.verbose = false;

  state.hierarchy_type = HIERARCHY_TYPE_DRAM_NVM_SSD;
  state.size_type = SIZE_TYPE_1;
  state.caching_type = CACHING_TYPE_LRU;
  state.latency_type = LATENCY_TYPE_1;
  state.migration_frequency = 3;
  state.file_name = "";
  state.operation_count = 0;

  // Parse args
  while (1) {
    int idx = 0;
    int c = getopt_long(argc, argv,
                        "a:c:f:m:l:o:s:vh",
                        opts, &idx);

    if (c == -1) break;

    switch (c) {
      case 'a':
        state.hierarchy_type = (HierarchyType)atoi(optarg);
        break;
      case 'c':
        state.caching_type = (CachingType)atoi(optarg);
        break;
      case 'f':
        state.file_name = optarg;
        break;
      case 'm':
        state.migration_frequency = atoi(optarg);
        break;
      case 'l':
        state.latency_type = (LatencyType)atoi(optarg);
        break;
      case 'o':
        state.operation_count = atoi(optarg);
        break;
      case 's':
        state.size_type = (SizeType)atoi(optarg);
        break;
      case 'v':
        state.verbose = atoi(optarg);
        break;
      case 'h':
        Usage();
        break;

      default:
        printf("Unknown option: -%c-\n", c);
        Usage();
    }
  }

  // Run validators
  printf("//===----------------------------------------------------------------------===//\n");
  printf("//                               MACHINE                                      //\n");
  printf("//===----------------------------------------------------------------------===//\n");

  ValidateHierarchyType(state);
  ValidateSizeType(state);
  ValidateLatencyType(state);
  ValidateCachingType(state);
  ValidateFileName(state);
  ValidateMigrationFrequency(state);
  SetupNVMLatency(state);
  ValidateNVMReadLatency(state);
  ValidateNVMWriteLatency(state);
  ValidateOperationCount(state);

  printf("//===----------------------------------------------------------------------===//\n");

}

}  // namespace machine
