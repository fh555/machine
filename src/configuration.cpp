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
      "   -c --caching_type                   :  caching type\n"
      "   -d --disk_mode_type                 :  disk mode type\n"
      "   -e --emulate                        :  emulate\n"
      "   -f --file_name                      :  file name\n"
      "   -l --latency_type                   :  latency type\n"
      "   -m --migration_frequency            :  migration frequency\n"
      "   -o --operation_count                :  operation count\n"
      "   -r --size_ratio_type                :  size ratio type\n"
      "   -s --size_type                      :  size type\n"
      "   -v --verbose                        :  verbose\n"
      "   -y --large_file_mode                :  large file mode\n"
      "   -z --summary_file                   :  summary file\n";
      exit(EXIT_FAILURE);
}

static struct option opts[] = {
    {"hierarchy_type", optional_argument, NULL, 'a'},
    {"caching_type", optional_argument, NULL, 'c'},
    {"disk_mode_type", optional_argument, NULL, 'd'},
    {"emulate", optional_argument, NULL, 'e'},
    {"file_name", optional_argument, NULL, 'f'},
    {"latency_type", optional_argument, NULL, 'l'},
    {"migration_frequency", optional_argument, NULL, 'm'},
    {"operation_count", optional_argument, NULL, 'o'},
    {"size_ratio_type", optional_argument, NULL, 'r'},
    {"size_type", optional_argument, NULL, 's'},
    {"verbose", optional_argument, NULL, 'v'},
    {"large_file_mode", optional_argument, NULL, 'y'},
    {"summary_file", optional_argument, NULL, 'z'},
    {NULL, 0, NULL, 0}
};

static void ValidateHierarchyType(const configuration &state) {
  if (state.hierarchy_type < 1 || state.hierarchy_type > HIERARCHY_TYPE_MAX) {
    printf("Invalid hierarchy_type :: %d\n", state.hierarchy_type);
    exit(EXIT_FAILURE);
  }
  else {
    printf("%30s : %s\n", "hierarchy_type",
           HierarchyTypeToString(state.hierarchy_type).c_str());
  }
}

static void ValidateDiskModeType(const configuration &state) {
  if (state.disk_mode_type < 1 || state.disk_mode_type > DISK_MODE_TYPE_MAX) {
    printf("Invalid disk_mode_type :: %d\n", state.disk_mode_type);
    exit(EXIT_FAILURE);
  }
  else {
    printf("%30s : %s\n", "disk_mode_type",
           DiskModeTypeToString(state.disk_mode_type).c_str());
  }
}

static void ValidateSizeType(const configuration &state) {
  if (state.size_type < 1 || state.size_type > SIZE_TYPE_MAX) {
    printf("Invalid size_type :: %d\n", state.size_type);
    exit(EXIT_FAILURE);
  }
  else {
    printf("%30s : %s\n", "size_type",
           SizeTypeToString(state.size_type).c_str());
  }
}

static void ValidateSizeRatioType(const configuration &state) {
  if (state.size_ratio_type < 1 || state.size_ratio_type > SIZE_RATIO_TYPE_MAX) {
    printf("Invalid size_ratio_type :: %d\n", state.size_ratio_type);
    exit(EXIT_FAILURE);
  }
  else {
    printf("%30s : %s\n", "size_ratio_type",
           SizeRatioTypeToString(state.size_ratio_type).c_str());
  }
}

static void ValidateCachingType(const configuration &state) {
  if (state.caching_type < 1 || state.caching_type > CACHING_TYPE_MAX) {
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

static void ValidateSummaryFile(const configuration &state){
  printf("%30s : %s\n", "summary_file", state.summary_file.c_str());
}

static void ValidateLatencyType(const configuration &state) {
  if (state.latency_type < 1 || state.latency_type > LATENCY_TYPE_MAX) {
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

static void ValidateLargeFileMode(const configuration &state){
  printf("%30s : %d\n", "large_file_mode", state.large_file_mode);
}

void SetupNVMLatency(configuration &state){

  switch(state.latency_type){
    case LATENCY_TYPE_1: {
      state.nvm_read_latency = 1;
      state.nvm_write_latency = 1;
      break;
    }

    case LATENCY_TYPE_2: {
      state.nvm_read_latency = 2;
      state.nvm_write_latency = 2;
      break;
    }

    case LATENCY_TYPE_3: {
      state.nvm_read_latency = 2;
      state.nvm_write_latency = 4;
      break;
    }

    case LATENCY_TYPE_4: {
      state.nvm_read_latency = 2;
      state.nvm_write_latency = 8;
      break;
    }

    case LATENCY_TYPE_5: {
      state.nvm_read_latency = 4;
      state.nvm_write_latency = 4;
      break;
    }

    case LATENCY_TYPE_6: {
      state.nvm_read_latency = 4;
      state.nvm_write_latency = 8;
      break;
    }

    case LATENCY_TYPE_7: {
      state.nvm_read_latency = 10;
      state.nvm_write_latency = 20;
      break;
    }

    case LATENCY_TYPE_8: {
      state.nvm_read_latency = 20;
      state.nvm_write_latency = 40;
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
                                                 state,
                                                 last_device_type);
  Device dram_device = DeviceFactory::GetDevice(DEVICE_TYPE_DRAM,
                                                state,
                                                last_device_type);
  Device nvm_device = DeviceFactory::GetDevice(DEVICE_TYPE_NVM,
                                               state,
                                               last_device_type);
  Device disk_device = DeviceFactory::GetDevice(DEVICE_TYPE_DISK,
                                                state,
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
    case HIERARCHY_TYPE_DRAM_DISK: {
      state.devices = {cache_device, dram_device, disk_device};
      state.memory_devices = {cache_device, dram_device};
      state.storage_devices = {disk_device};
    }
    break;
    case HIERARCHY_TYPE_NVM_DISK: {
      state.devices = {cache_device, nvm_device, disk_device};
      state.memory_devices = {cache_device, nvm_device};
      state.storage_devices = {nvm_device, disk_device};
    }
    break;
    case HIERARCHY_TYPE_DRAM_NVM_DISK: {
      state.devices = {cache_device, dram_device, nvm_device, disk_device};
      state.memory_devices = {cache_device, dram_device, nvm_device};
      state.storage_devices = {nvm_device, disk_device};
    }
    break;
    default:
      break;
  }

}

void ParseArguments(int argc, char *argv[], configuration &state) {

  // Default Values
  state.verbose = false;

  state.hierarchy_type = HIERARCHY_TYPE_DRAM_NVM_DISK;
  state.disk_mode_type = DISK_MODE_TYPE_SSD;
  state.size_type = SIZE_TYPE_1;
  state.size_ratio_type = SIZE_RATIO_TYPE_1;
  state.caching_type = CACHING_TYPE_FIFO;
  state.latency_type = LATENCY_TYPE_1;
  state.migration_frequency = 3;
  state.file_name = "";
  state.operation_count = 0;
  state.emulate = false;
  state.large_file_mode = false;

  // Parse args
  while (1) {
    int idx = 0;
    int c = getopt_long(argc, argv,
                        "a:c:d:e:f:m:l:o:r:s:vy:z:h",
                        opts, &idx);

    if (c == -1) break;

    switch (c) {
      case 'a':
        state.hierarchy_type = (HierarchyType)atoi(optarg);
        break;
      case 'c':
        state.caching_type = (CachingType)atoi(optarg);
        break;
      case 'd':
        state.disk_mode_type = (DiskModeType)atoi(optarg);
        break;
      case 'e':
        state.emulate = atoi(optarg);
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
      case 'r':
        state.size_ratio_type = (SizeRatioType)atoi(optarg);
        break;
      case 's':
        state.size_type = (SizeType)atoi(optarg);
        break;
      case 'v':
        state.verbose = atoi(optarg);
        break;
      case 'y':
        state.large_file_mode = atoi(optarg);
        break;
      case 'z':
        state.summary_file = optarg;
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
  if(state.emulate == false){
    printf("//===----------------------------------------------------------------------===//\n");
    printf("//                         MACHINE SIMULATOR                                  //\n");
    printf("//===----------------------------------------------------------------------===//\n");
  }
  else {
    printf("//===----------------------------------------------------------------------===//\n");
    printf("//                         MACHINE EMULATOR                                   //\n");
    printf("//===----------------------------------------------------------------------===//\n");
  }

  ValidateHierarchyType(state);
  ValidateDiskModeType(state);
  ValidateSizeType(state);
  ValidateSizeRatioType(state);
  ValidateLatencyType(state);
  ValidateCachingType(state);
  ValidateFileName(state);
  ValidateSummaryFile(state);
  ValidateMigrationFrequency(state);
  SetupNVMLatency(state);
  ValidateNVMReadLatency(state);
  ValidateNVMWriteLatency(state);
  ValidateOperationCount(state);
  ValidateLargeFileMode(state);

  printf("//===----------------------------------------------------------------------===//\n");

}

}  // namespace machine
