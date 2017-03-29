// WORKLOAD SOURCE

#include <chrono>
#include <ctime>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <map>

#include "macros.h"
#include "workload.h"
#include "distribution.h"
#include "configuration.h"
#include "device.h"
#include "cache.h"

namespace machine {

const static std::string OUTPUT_FILE = "outputfile.summary";
std::ofstream out(OUTPUT_FILE);

size_t query_itr;

double total_duration = 0;

const size_t CLEAN_BLOCK = 100;
const size_t DIRTY_BLOCK = 101;

UNUSED_ATTRIBUTE static void WriteOutput(double duration) {
  // Convert to ms
  duration *= 1000;

  // Write out output in verbose mode
  if (state.verbose == true) {
    printf("----------------------------------------------------------");
    printf("%lu :: %.1lf ms",
           query_itr,
           duration);
  }

  out << query_itr << " ";
  out << std::fixed << std::setprecision(2) << duration << "\n";

  out.flush();
}

void PrintMachine(){

  std::cout << "\n+++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
  std::cout << "MACHINE\n";
  for(auto device: state.devices){
    std::cout << device.cache;
  }
  std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++\n";

}

void BootstrapMachine(const size_t& total_slots) {

  auto last_device = state.devices.back();
  for(size_t slot_itr = 0; slot_itr < total_slots; slot_itr++){
    last_device.cache.Put(slot_itr, CLEAN_BLOCK);
  }

}

bool LocateInDevice(Device device,
                    const size_t& block_id){

  // Check device cache
  try{
    device.cache.Get(block_id);
    return true;
  }
  catch(const std::range_error& not_found){
    // Nothing to do here!
  }

  return false;
}

DeviceType LocateInDevices(std::vector<Device> devices,
                           const size_t& block_id){

  for(auto device : devices){
    auto found = LocateInDevice(device, block_id);
    if(found == true){
      return device.device_type;
    }
  }

  return DeviceType::DEVICE_TYPE_INVALID;
}

bool DeviceExists(const DeviceType& device_type){
  for(auto device : state.memory_devices){
    if(device.device_type == device_type){
      return true;
    }
  }
  return false;
}

DeviceType LocateInMemoryDevices(const size_t& block_id){
  return LocateInDevices(state.memory_devices, block_id);
}

DeviceType LocateInStorageDevices(const size_t& block_id){
  return LocateInDevices(state.storage_devices, block_id);
}

size_t GetDeviceOffset(const DeviceType& device_type){

  size_t device_itr = 0;
  for(auto device : state.devices){
    if(device.device_type == device_type){
      return device_itr;
    }
    device_itr++;
  }

  std::cout << "Did not find device of type : " <<
      DeviceTypeToString(device_type);
  exit(EXIT_FAILURE);
}

void MoveVictim(DeviceType source,
                const size_t& block_id,
                const bool& is_clean);

bool IsClean(const size_t& block_type){
  if(block_type == CLEAN_BLOCK){
    return true;
  }
  return false;
}

std::string PrintCleanStatus(bool is_clean){
  if(is_clean == true){
    return "CLEAN";
  }
  return "DIRTY";
}

void Copy(DeviceType destination,
          DeviceType source,
          const size_t& block_id){

  if(destination == DeviceType::DEVICE_TYPE_INVALID){
    exit(EXIT_FAILURE);
  }

  std::cout << DeviceTypeToString(source) << " ";
  std::cout << "---> " << DeviceTypeToString(destination) << "\n";

  // Write to destination device
  auto device_offset = GetDeviceOffset(destination);
  auto device_cache = state.devices[device_offset].cache;
  auto victim = device_cache.Put(block_id, CLEAN_BLOCK);

  total_duration += GetWriteLatency(destination);
  total_duration += GetReadLatency(source);

  // Move victim
  auto victim_key = victim.block_id;
  auto victim_block_type = victim.block_type;
  MoveVictim(destination,
             victim_key,
             IsClean(victim_block_type));

}

DeviceType GetLowerDevice(DeviceType source){
  DeviceType destination = DeviceType::DEVICE_TYPE_INVALID;
  switch(source){
    case DEVICE_TYPE_DRAM:
      destination = DEVICE_TYPE_NVM;
      break;

    case DEVICE_TYPE_NVM:
      destination = DEVICE_TYPE_SSD;
      break;

    default:
    case DEVICE_TYPE_INVALID:
      exit(EXIT_FAILURE);
  }

  return destination;
}

void MoveVictim(DeviceType source,
                const size_t& block_id,
                const bool& is_clean){

  bool victim_exists = (block_id != INVALID_KEY);
  bool volatile_device = (source == DeviceType::DEVICE_TYPE_DRAM);
  bool is_dirty = (is_clean == false);

  // Check if we have a dirty victim in DRAM
  if(victim_exists && volatile_device && is_dirty){
    auto destination = GetLowerDevice(source);

    std::cout << "Move victim : " << block_id << " ";
    std::cout << PrintCleanStatus(is_clean) << " ";

    // Copy to device
    Copy(destination, source, block_id);
  }

}

void BringBlockToMemory(const size_t& block_id){

  auto memory_device_type = LocateInMemoryDevices(block_id);
  auto storage_device_type = LocateInStorageDevices(block_id);
  auto nvm_exists = DeviceExists(DeviceType::DEVICE_TYPE_NVM);

  // Not found on DRAM & NVM
  if(memory_device_type == DeviceType::DEVICE_TYPE_INVALID){
    // Copy to NVM first if it exists in hierarchy
    if(nvm_exists == true) {
      Copy(DeviceType::DEVICE_TYPE_NVM,
           storage_device_type,
           block_id);
    }
    else {
      Copy(DeviceType::DEVICE_TYPE_DRAM,
           storage_device_type,
           block_id);
    }
  }

  // NVM to DRAM migration
  memory_device_type = LocateInMemoryDevices(block_id);
  if(memory_device_type == DeviceType::DEVICE_TYPE_NVM){
    auto dram_exists = DeviceExists(DeviceType::DEVICE_TYPE_DRAM);
    bool migrate_to_dram = (rand() % state.migration_frequency == 0);
    if(dram_exists == true){
      if(migrate_to_dram == true){
        Copy(DeviceType::DEVICE_TYPE_DRAM,
             DeviceType::DEVICE_TYPE_NVM,
             block_id);
      }
    }
  }

}

void BringBlockToStorage(const size_t& block_id){

  auto memory_device_type = LocateInMemoryDevices(block_id);
  auto nvm_exists = DeviceExists(DeviceType::DEVICE_TYPE_NVM);
  auto ssd_exists = DeviceExists(DeviceType::DEVICE_TYPE_SSD);

  // Check if it is on DRAM
  if(memory_device_type == DeviceType::DEVICE_TYPE_DRAM){
    // Copy to NVM first if it exists in hierarchy
    if(ssd_exists == false || nvm_exists == true) {
      Copy(DeviceType::DEVICE_TYPE_NVM,
           memory_device_type,
           block_id);
    }
    else {
      Copy(DeviceType::DEVICE_TYPE_SSD,
           memory_device_type,
           block_id);
    }

    // Remove from DRAM
    auto device_offset = GetDeviceOffset(memory_device_type);
    auto device_cache = state.devices[device_offset].cache;
    device_cache.Erase(block_id);

    // Update duration
    total_duration += GetWriteLatency(memory_device_type);
  }


}


void ReadBlock(const size_t& block_id){
  std::cout << "READ  " << block_id << "\n";

  // Bring block to memory if needed
  BringBlockToMemory(block_id);

  auto memory_device_type = LocateInMemoryDevices(block_id);
  total_duration += GetReadLatency(memory_device_type);

}

void UpdateBlock(const size_t& block_id) {
  std::cout << "UPDATE " << block_id << "\n";

  // Bring block to memory if needed
  BringBlockToMemory(block_id);

  // Write block on memory device
  auto memory_device_type = LocateInMemoryDevices(block_id);
  if(memory_device_type == DeviceType::DEVICE_TYPE_DRAM){
    auto device_offset = GetDeviceOffset(memory_device_type);
    auto device_cache = state.devices[device_offset].cache;
    auto victim = device_cache.Put(block_id, DIRTY_BLOCK);
    // Check victim
    if(victim.block_id != INVALID_KEY){
      exit(EXIT_FAILURE);
    }
  }

  // Update duration
  total_duration += GetWriteLatency(memory_device_type);

}

void FlushBlock(const size_t& block_id) {
  std::cout << "FLUSH " << block_id << "\n";

  // Check if dirty in DRAM
  auto memory_device_type = LocateInMemoryDevices(block_id);
  if(memory_device_type == DeviceType::DEVICE_TYPE_DRAM){
    auto device_offset = GetDeviceOffset(memory_device_type);
    auto device_cache = state.devices[device_offset].cache;
    auto block_status = device_cache.Get(block_id);
    if(IsClean(block_status) == false){
      // Bring block to storage
      BringBlockToStorage(block_id);
    }
  }

}

void MachineHelper() {

  // Run workload

  // Determine size of last device
  auto last_device = state.devices.back();
  size_t total_slots = last_device.device_size;
  std::cout << "Total slots : " << total_slots << "\n";

  // Reinit duration
  total_duration = 0;

  // Bootstrap
  BootstrapMachine(total_slots);

  // Print machine caches
  PrintMachine();

  size_t upper_bound = total_slots - 1;
  double theta = 0.5;
  size_t operation_count = state.operation_count;
  size_t operation_itr;
  double seed = 23;
  srand(seed);
  int update_ratio = 20;
  int flush_ratio = 5;

  ZipfDistribution zipf_generator(upper_bound, theta);
  UniformDistribution uniform_generator(seed);

  for(operation_itr = 0; operation_itr < operation_count; operation_itr++){
    auto block_id = zipf_generator.GetNextNumber();
    auto operation_sample = rand() % 100;

    std::cout << "\nOperation : " << operation_itr << " :: ";
    if(operation_sample < flush_ratio) {
      UpdateBlock(block_id);
      FlushBlock(block_id);
    }
    else if(operation_sample < update_ratio) {
      UpdateBlock(block_id);
    }
    else {
      ReadBlock(block_id);
    }

    std::cout << "Duration : " << total_duration << "\n";
    std::cout << "-------------------------";

  }

  // Print machine caches
  PrintMachine();

}

void RunMachineTest() {

  // Run the benchmark once
  MachineHelper();

}

}  // namespace machine

