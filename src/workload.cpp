// WORKLOAD SOURCE

#include <chrono>
#include <ctime>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <map>
#include <set>
#include <unistd.h>
#include <cstdio>

#include "macros.h"
#include "workload.h"
#include "distribution.h"
#include "configuration.h"
#include "device.h"
#include "cache.h"
#include "stats.h"

namespace machine {

size_t query_itr;

double logical_ns = 0;

bool emulate = false;

// Stats
extern Stats machine_stats;

configuration state;

static void WriteOutput(double stat) {

  std::string OUTPUT_FILE = state.summary_file;
  std::ofstream out(OUTPUT_FILE);

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
    auto device_size = device.cache.GetSize();
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
  size_t current_total_frequency = 0;
  size_t current_total_blocks = 0;

  for(auto rit = frequency_map.rbegin(); rit!= frequency_map.rend(); ++rit){
    auto frequency = rit->first;
    auto blocks = rit->second;

    current_total_frequency += blocks * frequency;
    current_total_blocks += blocks;

    if(current_total_frequency > required_frequency){
      break;
    }

  }

  std::cout << "PERCENT: " << percent << " ";
  std::cout << "BLOCKS NEEDED: ";
  PrintCapacity(current_total_blocks);
  std::cout << "\n";

}

void PrintFrequency(size_t available_blocks,
                    size_t total_frequency,
                    const std::map<size_t, size_t>& frequency_map){

  size_t current_total_frequency = 0;
  size_t current_total_blocks = 0;

  for(auto rit = frequency_map.rbegin(); rit!= frequency_map.rend(); ++rit){
    auto frequency = rit->first;
    auto blocks = rit->second;

    current_total_frequency += blocks * frequency;
    current_total_blocks += blocks;

    if(current_total_blocks > available_blocks){
      break;
    }
  }

  auto captured_frequency = (current_total_frequency * 100)/total_frequency;

  std::cout << "AVAILABLE BLOCKS: ";
  PrintCapacity(available_blocks);

  std::cout << " PERCENT: " << captured_frequency << "%\n";
}

void PrintWorkload(const std::map<size_t, size_t>& block_map){
  std::map<size_t, size_t> frequency_map;
  size_t total_frequency = 0;
  size_t frequency_threshold = 50000;

  std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
  std::cout << "WORKLOAD ANALYSIS \n";

  std::cout << "BLOCK FREQUENCY \n";
  for(auto entry: block_map){
    auto frequency = entry.second;
    frequency_map[frequency]++;
    total_frequency += frequency;
    if(entry.second > frequency_threshold){
      std::cout << "Block : " << entry.first << " - "
          << " Frequency : " << entry.second << "\n";
    }
  }
  std::cout << "\n";

  std::cout << "FREQUENCY DISTRIBUTION \n";
  for(auto frequency : frequency_map){
    if(frequency.first > frequency_threshold){
      std::cout << "Frequency : " << std::setw(5)
      << (frequency.first) << " - "
      << " Block Count: " << frequency.second << "\n";
    }
  }
  std::cout << "\n";

  // SPACE REQUIRED TO COVER A FRACTION OF WORKING SET
  std::vector<size_t> percents = {10, 25, 50, 75, 90, 100};
  for(auto percent: percents){
    PrintRequiredBlocks(percent,
                        total_frequency,
                        frequency_map);
  }

  std::cout << "\n";

  // UTILITY OF CACHE
  std::vector<size_t> cache_sizes = {
      1, 16, 256, 4096, 16384, 65536, 1048576
  };
  for(auto cache_size: cache_sizes){
    PrintFrequency(cache_size,
                   total_frequency,
                   frequency_map);
  }

  std::cout << "\n";
  std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++\n";

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
  auto flush_block = false;

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
           flush_block,
           logical_ns);
    }
    else {
      Copy(state.devices,
           DeviceType::DEVICE_TYPE_DRAM,
           storage_device_type,
           block_id,
           CLEAN_BLOCK,
           flush_block,
           logical_ns);
    }
  }

  // NVM to DRAM migration
  memory_device_type = LocateInMemoryDevices(block_id);

  if(memory_device_type == DeviceType::DEVICE_TYPE_NVM){
    auto dram_exists = DeviceExists(state.devices, DeviceType::DEVICE_TYPE_DRAM);
    bool migrate_upwards = (rand() % state.migration_frequency == 0);
    if(migrate_upwards == true){
      if(dram_exists == true){
        Copy(state.devices,
             DeviceType::DEVICE_TYPE_DRAM,
             DeviceType::DEVICE_TYPE_NVM,
             block_id,
             CLEAN_BLOCK,
             flush_block,
             logical_ns);
      }
    }
  }

  // DRAM/NVM to CACHE migration
  memory_device_type = LocateInMemoryDevices(block_id);

  if(memory_device_type == DeviceType::DEVICE_TYPE_DRAM ||
      memory_device_type == DeviceType::DEVICE_TYPE_NVM){
    Copy(state.devices,
         DeviceType::DEVICE_TYPE_CACHE,
         memory_device_type,
         block_id,
         CLEAN_BLOCK,
         flush_block,
         logical_ns);
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
  auto flush_block = true;

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
           flush_block,
           logical_ns);
    }
    else {
      Copy(state.devices,
           DeviceType::DEVICE_TYPE_DISK,
           source,
           block_id,
           CLEAN_BLOCK,
           flush_block,
           logical_ns);
    }

    // Mark block as clean
    auto device_offset = GetDeviceOffset(state.devices, source);
    auto device_cache = state.devices[device_offset].cache;
    auto victim = device_cache.Put(block_id, CLEAN_BLOCK);
    if(victim.block_id != INVALID_KEY){
      exit(EXIT_FAILURE);
    }

    // Update duration
    logical_ns += GetWriteLatency(state.devices, source, block_id, flush_block);
  }

}

void WriteBlock(const size_t& block_id) {

  // Bring block to memory if needed
  BringBlockToMemory(block_id);

  auto destination = LocateInMemoryDevices(block_id);
  auto flush_block = false;

  // CASE 1: New block
  if(destination == DeviceType::DEVICE_TYPE_INVALID){
    //std::cout << "WRITE " << block_id << "\n";

    // Mark block as dirty
    Copy(state.devices,
         DeviceType::DEVICE_TYPE_CACHE,
         DeviceType::DEVICE_TYPE_INVALID,
         block_id,
         DIRTY_BLOCK,
         flush_block,
         logical_ns);

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
  logical_ns += GetWriteLatency(state.devices, destination, block_id, flush_block);

}

void ReadBlock(const size_t& block_id){
  //std::cout << "READ  " << block_id << "\n";

  // Bring block to memory if needed
  BringBlockToMemory(block_id);

  // Update duration
  auto source = LocateInMemoryDevices(block_id);
  logical_ns += GetReadLatency(state.devices, source, block_id);

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
    // Check device cache
    auto block_status = device_cache.Get(block_id);
    if(block_status == INVALID_VALUE){
      std::cout << "Did not find the to be flushed block: " << block_id;
      exit(EXIT_FAILURE);
    }
    if(block_status != CLEAN_BLOCK){
      BringBlockToStorage(block_id, block_status);
    }

  }

}

void BootstrapBlock(const size_t& block_id) {

  auto nvm_exists = DeviceExists(state.devices, DeviceType::DEVICE_TYPE_NVM);
  if(nvm_exists == true){
    auto device_offset = GetDeviceOffset(state.devices, DeviceType::DEVICE_TYPE_NVM);
    auto device_cache = state.devices[device_offset].cache;

    double size = device_cache.GetSize();
    double capacity = device_cache.GetCapacity();
    double occupied_fraction = size/capacity;
    if(occupied_fraction > 0.5){
      // Bootstrap on last device
      auto last_device_cache = state.devices.back().cache;
      last_device_cache.Put(block_id, CLEAN_BLOCK);
      return;
    }
  }

  // Bootstrap on durable storage
  WriteBlock(block_id);
  FlushBlock(block_id);

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
  size_t warm_up_ratio = 10; // 10%

  // Figure out warm up operation count
  auto warm_up_operation_count = (warm_up_ratio * state.operation_count)/100;

  std::cout << "WARMING UP SIMULATOR:: OPERATION COUNT: " << warm_up_operation_count << "\n";

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

  bool warmed_up = false;

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

    if(warmed_up == false &&
        operation_itr == warm_up_operation_count){
      operation_itr = 0;
      warmed_up = true;
    }

    if(state.operation_count != 0){
      if(operation_itr > state.operation_count){
        break;
      }
    }

  }

  // Print Workload
  PrintWorkload(block_map);

  // Print machine caches
  //PrintMachine();

  // Reset file pointer
  input->clear();
  input->seekg(0, std::ios::beg);

  // Reinit duration
  logical_ns = 0;
  operation_itr = 0;

  // Reset stats
  machine_stats.Reset();

  warmed_up = false;
  size_t read_operation_itr = 0;
  size_t write_operation_itr = 0;
  size_t flush_operation_itr = 0;

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
      case 'r': {
        ReadBlock(global_block_number);
        read_operation_itr++;
        break;
      }

      case 'w': {
        WriteBlock(global_block_number);
        write_operation_itr++;
        break;
      }

      case 'f': {
        FlushBlock(global_block_number);
        flush_operation_itr++;
        break;
      }

      default:
        invalid_operation_itr++;
        break;
    }

    if(warmed_up == false &&
        operation_itr == warm_up_operation_count){

      if(operation_itr % 100000 == 0){
        std::cout << "Operation " << operation_itr << " :: " <<
            operation_type << " " << global_block_number << " "
            << fork_number << " " << block_number << " :: "
            << logical_ns / (1000 * 1000) << "s \n";
      }

      std::cout << "Warmed Up : " << warm_up_operation_count << " ops \n";

      // Reinit duration
      logical_ns = 0;
      operation_itr = 0;

      read_operation_itr = 0;
      write_operation_itr = 0;
      flush_operation_itr = 0;

      // Print machine caches
      //PrintMachine();

      // Reset stats
      machine_stats.Reset();

      // Set warmed up
      warmed_up = true;

      // Start physical timer
      physical_timer.Reset();
    }

    auto logical_s = logical_ns/(1000 * 1000 * 1000);
    auto physical_ns = physical_timer.GetDuration();
    auto physical_s = physical_ns/(1000 * 1000 * 1000);
    if(operation_itr % 100000 == 0){
      std::cout << "Operation " << operation_itr << " :: " <<
          operation_type << " " << global_block_number << " "
          << fork_number << " " << block_number << " :: "
          << logical_s  << "s "
          << physical_s << "s " << "\n";
    }

    if(state.operation_count != 0){
      if(operation_itr > state.operation_count){
        break;
      }
    }

  }

  // Measure physical time, logical time, and throughput
  auto logical_s = logical_ns/(1000 * 1000 * 1000);
  auto physical_ns = physical_timer.GetDuration();
  auto physical_s = physical_ns/(1000 * 1000 * 1000);
  auto throughput = operation_itr/logical_s;

  std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
  std::cout << "PHYSICAL TIME (s): " << physical_s << "\n";
  std::cout << "LOGICAL TIME  (s): " << logical_s << "\n";
  std::cout << "THROUGHPUT : " << throughput << " (OPS/S) \n";
  std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++\n";

  // Get machine size
  auto machine_size = GetMachineSize();
  std::cout << "Machine size  : " << machine_size << "\n";
  std::cout << "Invalid operation count  : " << invalid_operation_itr << "\n";

  std::cout << "READS   : " << (read_operation_itr * 100)/operation_itr << " %\n";
  std::cout << "WRITES  : " << (write_operation_itr * 100)/operation_itr << " %\n";
  std::cout << "FLUSHES : " << (flush_operation_itr * 100)/operation_itr << " %\n";

  // Print machine caches
  PrintMachine();

  // Emit output
  WriteOutput(throughput);

}

void RunMachineTest() {

  // Bootstrap filesystem if needed
  if(state.emulate == true){
    emulate = true;
    BootstrapFileSystemForEmulation(state);
  }

  // Run the benchmark once
  MachineHelper();

}

}  // namespace machine

