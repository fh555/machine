// WORKLOAD SOURCE

#include <chrono>
#include <ctime>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <map>
#include <set>

#include "macros.h"
#include "workload.h"
#include "distribution.h"
#include "configuration.h"
#include "device.h"
#include "cache.h"
#include "stats.h"

namespace machine {

const static std::string OUTPUT_FILE = "outputfile.summary";
std::ofstream out(OUTPUT_FILE);

size_t query_itr;

double total_duration = 0;

// Stats
extern Stats machine_stats;

static void WriteOutput(double stat) {

  // Write out output in verbose mode
  if (state.verbose == true) {
    printf("----------------------------------------------------------");
    printf("%.2lf s",
           stat);
  }

  out << std::fixed << std::setprecision(2) << stat << "\n";

  out.flush();
}

size_t GetMachineSize(){

  size_t machine_size = 0;
  for(auto device: state.devices){
    auto device_size = device.cache.CurrentCapacity();
    machine_size += device_size;
  }

  return machine_size;
}

void PrintMachine(){

  std::cout << "\n+++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
  std::cout << "MACHINE\n";
  for(auto device: state.devices){
    std::cout << device.cache;
  }
  std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++\n";

  std::cout << machine_stats;

}

void PrintRequiredBlocks(size_t percent,
                         size_t total_frequency,
                         const std::map<size_t, size_t>& frequency_map){

  auto required_frequency = (total_frequency * percent)/100;
  std::cout << "REQUIRED: " << required_frequency << "\n";
  size_t current_total_frequency = 0;
  size_t current_total_blocks = 0;

  for(auto rit = frequency_map.rbegin(); rit!= frequency_map.rend(); ++rit){
    auto frequency = rit->first;
    auto blocks = rit->second;

    current_total_frequency += frequency;
    current_total_blocks += blocks;

    std::cout << "TOTAL: " << current_total_frequency
        << " BLOCKS NEEDED: " << current_total_blocks << "\n";
    if(current_total_frequency > required_frequency){
      break;
    }

  }

  std::cout << "PERCENT: " << percent << "% BLOCKS NEEDED: " << current_total_blocks << "\n";
}

void PrintWorkload(const std::map<size_t, size_t>& block_map){
  std::map<size_t, size_t> frequency_map;
  size_t bucket_size = 1;
  size_t total_frequency = 0;

  for(auto entry: block_map){
    auto frequency = entry.second;
    frequency_map[frequency/bucket_size]++;
    total_frequency += frequency;
  }

  std::cout << "FREQUENCY DISTRIBUTION \n";
  for(auto frequency : frequency_map){
    std::cout << "Frequency : " << std::setw(5)
    << (frequency.first) * bucket_size << " - "
    << (frequency.first + 1) * bucket_size
    << " Block Count: " << frequency.second << "\n";
  }
  std::cout << "\n";

  PrintRequiredBlocks(1,
                      total_frequency/bucket_size,
                      frequency_map);

  PrintRequiredBlocks(10,
                      total_frequency/bucket_size,
                      frequency_map);

}

DeviceType LocateInMemoryDevices(const size_t& block_id){
  return LocateInDevices(state.memory_devices, block_id);
}

DeviceType LocateInStorageDevices(const size_t& block_id){
  return LocateInDevices(state.storage_devices, block_id);
}

bool IsVolatileDevice(DeviceType device_type){
  return (device_type == DeviceType::DEVICE_TYPE_CACHE ||
      device_type == DeviceType::DEVICE_TYPE_DRAM);
}

void BringBlockToMemory(const size_t& block_id){

  auto memory_device_type = LocateInMemoryDevices(block_id);
  auto storage_device_type = LocateInStorageDevices(block_id);
  auto nvm_exists = DeviceExists(state.devices, DeviceType::DEVICE_TYPE_NVM);

  // Not found on DRAM & NVM
  if(memory_device_type == DeviceType::DEVICE_TYPE_INVALID &&
      storage_device_type != DeviceType::DEVICE_TYPE_INVALID){
    // Copy to NVM first if it exists in hierarchy
    if(nvm_exists == true) {
      Copy(state.devices,
           DeviceType::DEVICE_TYPE_NVM,
           storage_device_type,
           block_id,
           CLEAN_BLOCK,
           total_duration);
    }
    else {
      Copy(state.devices,
           DeviceType::DEVICE_TYPE_DRAM,
           storage_device_type,
           block_id,
           CLEAN_BLOCK,
           total_duration);
    }
  }

  // NVM to DRAM migration
  memory_device_type = LocateInMemoryDevices(block_id);

  if(memory_device_type == DeviceType::DEVICE_TYPE_NVM){
    auto dram_exists = DeviceExists(state.devices, DeviceType::DEVICE_TYPE_DRAM);
    bool migrate_to_dram = (rand() % state.migration_frequency == 0);
    if(dram_exists == true){
      if(migrate_to_dram == true){
        Copy(state.devices,
             DeviceType::DEVICE_TYPE_DRAM,
             DeviceType::DEVICE_TYPE_NVM,
             block_id,
             CLEAN_BLOCK,
             total_duration);
      }
    }
  }

  // DRAM to CACHE migration
  memory_device_type = LocateInMemoryDevices(block_id);

  if(memory_device_type == DeviceType::DEVICE_TYPE_DRAM){
    bool migrate_to_cache = (rand() % state.migration_frequency == 0);
    if(migrate_to_cache == true){
      Copy(state.devices,
           DeviceType::DEVICE_TYPE_CACHE,
           DeviceType::DEVICE_TYPE_DRAM,
           block_id,
           CLEAN_BLOCK,
           total_duration);
    }
  }

}

void BringBlockToStorage(const size_t& block_id,
                         const size_t& block_status){

  auto source = LocateInMemoryDevices(block_id);
  auto is_volatile_source = IsVolatileDevice(source);
  auto nvm_exists = DeviceExists(state.devices, DeviceType::DEVICE_TYPE_NVM);
  auto last_device_type = state.devices.back().device_type;
  auto nvm_last = (last_device_type == DeviceType::DEVICE_TYPE_NVM);
  auto nvm_status = block_status;
  if(nvm_last == true){
    nvm_status = CLEAN_BLOCK;
  }

  // Check if it is on DRAM or CACHE
  if(is_volatile_source){
    // Copy to NVM first if it exists in hierarchy
    if(nvm_exists == true) {
      Copy(state.devices,
           DeviceType::DEVICE_TYPE_NVM,
           source,
           block_id,
           nvm_status,
           total_duration);
    }
    else {
      Copy(state.devices,
           DeviceType::DEVICE_TYPE_SSD,
           source,
           block_id,
           CLEAN_BLOCK,
           total_duration);
    }

    // Mark block as clean
    auto device_offset = GetDeviceOffset(state.devices, source);
    auto device_cache = state.devices[device_offset].cache;
    auto victim = device_cache.Put(block_id, CLEAN_BLOCK);
    if(victim.block_id != INVALID_KEY){
      exit(EXIT_FAILURE);
    }

    // Update duration
    total_duration += GetWriteLatency(state.devices, source, block_id);
  }

}

void BootstrapBlock(const size_t& block_id) {

  auto last_device_cache = state.devices.back().cache;
  last_device_cache.Put(block_id, CLEAN_BLOCK);

}

void WriteBlock(const size_t& block_id) {

  // Bring block to memory if needed
  BringBlockToMemory(block_id);

  auto destination = LocateInMemoryDevices(block_id);

  // CASE 1: New block
  if(destination == DeviceType::DEVICE_TYPE_INVALID){
    //std::cout << "WRITE " << block_id << "\n";

    // Mark block as dirty
    Copy(state.devices,
         DeviceType::DEVICE_TYPE_CACHE,
         DeviceType::DEVICE_TYPE_INVALID,
         block_id,
         DIRTY_BLOCK,
         total_duration);

    return;
  }

  // CASE 2: Existing block
  //std::cout << "UPDATE " << block_id << "\n";

  // Mark block as dirty
  auto is_volatile_destination = IsVolatileDevice(destination);
  if(is_volatile_destination){
    auto device_offset = GetDeviceOffset(state.devices, destination);
    auto device_cache = state.devices[device_offset].cache;
    auto victim = device_cache.Put(block_id, DIRTY_BLOCK);
    if(victim.block_id != INVALID_KEY){
      exit(EXIT_FAILURE);
    }
  }

  // Update duration
  total_duration += GetWriteLatency(state.devices, destination, block_id);

}

void ReadBlock(const size_t& block_id){
  //std::cout << "READ  " << block_id << "\n";

  // Bring block to memory if needed
  BringBlockToMemory(block_id);

  // Update duration
  auto source = LocateInMemoryDevices(block_id);
  total_duration += GetReadLatency(state.devices, source, block_id);

  if(source == DeviceType::DEVICE_TYPE_INVALID){
    std::cout << "Could not read block : " << block_id << "\n";
    exit(EXIT_FAILURE);
  }

}

void FlushBlock(const size_t& block_id) {
  //std::cout << "FLUSH " << block_id << "\n";

  // Check if dirty in volatile device
  auto memory_device_type = LocateInMemoryDevices(block_id);
  auto is_volatile_device = IsVolatileDevice(memory_device_type);
  if(is_volatile_device == true){
    auto device_offset = GetDeviceOffset(state.devices, memory_device_type);
    auto device_cache = state.devices[device_offset].cache;
    auto block_status = device_cache.Get(block_id, true);
    if(block_status != CLEAN_BLOCK){
      BringBlockToStorage(block_id, block_status);
    }
  }

}

size_t GetGlobalBlockNumber(const size_t& fork_number,
                            const size_t& block_number){
  return (fork_number * 10 + block_number);
}

void MachineHelper() {

  // Run workload

  // Go through trace file
  std::unique_ptr<std::istream> input;
  char operation_type;
  size_t fork_number;
  size_t block_number;

  if (state.file_name.empty()) {
    return;
  }
  else {
    std::cout << "Running trace " << state.file_name << "...\n";
    input.reset(new std::ifstream(state.file_name.c_str()));
  }

  size_t fragment_size = 4096;
  char buffer[fragment_size];

  size_t operation_itr = 0;
  size_t invalid_operation_itr = 0;

  std::map<size_t, size_t> block_map;

  // PREPROCESS
  while(!input->eof()){
    operation_itr++;

    // Get a line from the input stream
    input->getline(buffer, fragment_size);

    // Check statement
    sscanf(buffer, "%c %lu %lu",
           &operation_type,
           &fork_number,
           &block_number);

    auto global_block_number = GetGlobalBlockNumber(fork_number, block_number);

    // Block does not exist
    if(block_map.count(global_block_number) == 0){
      BootstrapBlock(global_block_number);
    }
    block_map[global_block_number]++;

    if(state.operation_count != 0){
      if(operation_itr > state.operation_count){
        break;
      }
    }

  }

  // Print Workload
  PrintWorkload(block_map);

  // Print machine caches
  PrintMachine();

  // Reset file pointer
  input->clear();
  input->seekg(0, std::ios::beg);

  // Reinit duration
  total_duration = 0;
  operation_itr = 0;

  // Reset stats
  machine_stats.Reset();

  // RUN SIMULATION
  while(!input->eof()){
    operation_itr++;

    // Get a line from the input stream
    input->getline(buffer, fragment_size);

    // Check statement
    sscanf(buffer, "%c %lu %lu",
           &operation_type,
           &fork_number,
           &block_number);

    auto global_block_number = GetGlobalBlockNumber(fork_number, block_number);

    switch(operation_type){
      case 'r':
        ReadBlock(global_block_number);
        break;

      case 'w':
        WriteBlock(global_block_number);
        break;

      case 'f':
        FlushBlock(global_block_number);
        break;

      default:
        invalid_operation_itr++;
        break;
    }

    if(operation_itr % 100000 == 0){
      std::cout << "Operation " << operation_itr << " :: " <<
          operation_type << " " << global_block_number << " "
          << fork_number << " " << block_number << " :: "
          << total_duration / (1000 * 1000) << "s \n";
    }

    if(state.operation_count != 0){
      if(operation_itr > state.operation_count){
        break;
      }
    }

  }

  auto throughput = (operation_itr * 1000 * 1000)/total_duration;

  std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
  std::cout << "Throughput : " << throughput << " (ops/s) \n";
  std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++\n";

  // Get machine size
  auto machine_size = GetMachineSize();
  std::cout << "Machine size  : " << machine_size << "\n";
  std::cout << "Invalid operation count  : " << invalid_operation_itr << "\n";

  // Print machine caches
  PrintMachine();

  // Emit output
  WriteOutput(throughput);

}

void RunMachineTest() {

  // Run the benchmark once
  MachineHelper();

}

}  // namespace machine

