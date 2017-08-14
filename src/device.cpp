// DEVICE SOURCE

#include "macros.h"
#include "device.h"
#include "configuration.h"
#include "stats.h"

namespace machine {

std::map<DeviceType, size_t> device_size;
std::map<DeviceType, double> seq_read_latency;
std::map<DeviceType, double> seq_write_latency;
std::map<DeviceType, double> rnd_read_latency;
std::map<DeviceType, double> rnd_write_latency;

// Machine stats
Stats machine_stats;

void BootstrapDeviceMetrics(const configuration &state){

  // LATENCIES (ns)

  // CACHE
  seq_read_latency[DEVICE_TYPE_CACHE] = 10;
  seq_write_latency[DEVICE_TYPE_CACHE] = 10;
  rnd_read_latency[DEVICE_TYPE_CACHE] = 10;
  rnd_write_latency[DEVICE_TYPE_CACHE] = 10;

  // DRAM
  seq_read_latency[DEVICE_TYPE_DRAM] = 100;
  seq_write_latency[DEVICE_TYPE_DRAM] = 100;
  rnd_read_latency[DEVICE_TYPE_DRAM] = 100;
  rnd_write_latency[DEVICE_TYPE_DRAM] = 100;

  // NVM
  seq_read_latency[DEVICE_TYPE_NVM] = seq_read_latency[DEVICE_TYPE_DRAM];
  seq_write_latency[DEVICE_TYPE_NVM] = seq_write_latency[DEVICE_TYPE_DRAM];
  rnd_read_latency[DEVICE_TYPE_NVM] = rnd_read_latency[DEVICE_TYPE_DRAM];
  rnd_write_latency[DEVICE_TYPE_NVM] = rnd_write_latency[DEVICE_TYPE_DRAM];

  seq_read_latency[DEVICE_TYPE_NVM] *=  state.nvm_read_latency;
  seq_write_latency[DEVICE_TYPE_NVM] *= state.nvm_write_latency;
  rnd_read_latency[DEVICE_TYPE_NVM] *= state.nvm_read_latency;
  rnd_write_latency[DEVICE_TYPE_NVM] *= state.nvm_write_latency;

  // Check disk mode

  // SSD
  if(state.disk_mode_type == DiskModeType::DISK_MODE_TYPE_SSD){
    seq_read_latency[DEVICE_TYPE_DISK] = 100 * 100;
    seq_write_latency[DEVICE_TYPE_DISK] = 250 * 100;
    rnd_read_latency[DEVICE_TYPE_DISK] = 100 * 100;
    rnd_write_latency[DEVICE_TYPE_DISK] = 400 * 100;
  }
  // HDD
  else if(state.disk_mode_type == DiskModeType::DISK_MODE_TYPE_HDD) {
    seq_read_latency[DEVICE_TYPE_DISK] = 1 * 1000 * 1000;
    seq_write_latency[DEVICE_TYPE_DISK] = 1 * 1000 * 1000;
    rnd_read_latency[DEVICE_TYPE_DISK] = 4 * 1000 * 1000;
    rnd_write_latency[DEVICE_TYPE_DISK] = 10 * 1000 * 1000;
  }
  else {
    std::cout << "Invalid disk mode type: " << state.disk_mode_type;
    exit(EXIT_FAILURE);
  }

}

bool IsSequential(std::vector<Device>& devices,
                  const DeviceType& device_type,
                  const size_t& next);

std::string GetPattern(bool is_sequential){
  if(is_sequential == true){
    return "SEQ";
  }
  return "RND";
}

// GET READ & WRITE LATENCY

size_t GetWriteLatency(std::vector<Device>& devices,
                       DeviceType device_type,
                       const size_t& block_id){

  DLOG(INFO) << "WRITE :: " << DeviceTypeToString(device_type) << "\n";

  // Increment stats
  machine_stats.IncrementWriteCount(device_type);

  // Check if sequential or random?
  bool is_sequential = IsSequential(devices, device_type, block_id);

  switch(device_type){
    case DEVICE_TYPE_CACHE:
    case DEVICE_TYPE_DRAM:
    case DEVICE_TYPE_NVM:
    case DEVICE_TYPE_DISK: {
      if(is_sequential == true){
        return seq_write_latency[device_type];
      }
      else {
        return rnd_write_latency[device_type];
      }
    }

    case DEVICE_TYPE_INVALID:
      return 0;

    default: {
      std::cout << "GetWriteLatency: Get invalid device";
      exit(EXIT_FAILURE);
    }
  }
}

size_t GetReadLatency(std::vector<Device>& devices,
                      DeviceType device_type,
                      const size_t& block_id){

  DLOG(INFO) << "READ :: " << DeviceTypeToString(device_type) << "\n";

  // Increment stats
  machine_stats.IncrementReadCount(device_type);

  // Check if sequential or random?
  bool is_sequential = IsSequential(devices, device_type, block_id);

  switch(device_type){
    case DEVICE_TYPE_CACHE:
    case DEVICE_TYPE_DRAM:
    case DEVICE_TYPE_NVM:
    case DEVICE_TYPE_DISK: {
      if(is_sequential == true){
        return seq_read_latency[device_type];
      }
      else {
        return rnd_read_latency[device_type];
      }
    }

    case DEVICE_TYPE_INVALID:
      return 0;

    default: {
      std::cout << "GetReadLatency: Get invalid device";
      exit(EXIT_FAILURE);
    }
  }
}

// LOCATE IN DEVICE

bool LocateInDevice(Device device,
                    const size_t& block_id){

  // Check device cache
  try{
    device.cache.Get(block_id, true);
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

// GET DEVICE OFFSET

size_t GetDeviceOffset(std::vector<Device>& devices,
                       const DeviceType& device_type){

  size_t device_itr = 0;
  for(auto device : devices){
    if(device.device_type == device_type){
      return device_itr;
    }
    device_itr++;
  }

  std::cout << "GetDeviceOffset: Get invalid device: " << DeviceTypeToString(device_type);
  exit(EXIT_FAILURE);
}

// DEVICE EXISTS?

bool DeviceExists(std::vector<Device>& devices,
                  const DeviceType& device_type){
  for(auto device : devices){
    if(device.device_type == device_type){
      return true;
    }
  }
  return false;
}

// IS SEQUENTIAL?

bool IsSequential(std::vector<Device>& devices,
                  const DeviceType& device_type,
                  const size_t& next){
  for(auto device : devices){
    if(device.device_type == device_type){
      return device.cache.IsSequential(next);
    }
  }
  return false;
}

// GET DEVICE LOWER IN THE HIERARCHY

DeviceType GetLowerDevice(std::vector<Device>& devices,
                          DeviceType source){
  DeviceType destination = DeviceType::DEVICE_TYPE_INVALID;
  auto dram_exists = DeviceExists(devices, DeviceType::DEVICE_TYPE_DRAM);
  auto nvm_exists = DeviceExists(devices, DeviceType::DEVICE_TYPE_NVM);

  switch(source){
    case DEVICE_TYPE_CACHE: {
      if(dram_exists == true) {
        destination = DEVICE_TYPE_DRAM;
      }
      else {
        destination = DEVICE_TYPE_NVM;
      }
      break;
    }

    case DEVICE_TYPE_DRAM: {
      if(nvm_exists == true){
        destination = DEVICE_TYPE_NVM;
      }
      else {
        destination = DEVICE_TYPE_DISK;
      }
      break;
    }

    case DEVICE_TYPE_NVM: {
      destination = DEVICE_TYPE_DISK;
      break;
    }

    default:
    case DEVICE_TYPE_INVALID: {
      std::cout << "GetLowerDevice: Get invalid device";
      exit(EXIT_FAILURE);
    }
  }

  return destination;
}

std::string CleanStatus(const size_t& block_status){
  if(block_status == CLEAN_BLOCK){
    return "";
  }
  else if(block_status == DIRTY_BLOCK){
    return "●";
  }
  else {
    std::cout << "Invalid block type: " << block_status;
    exit(EXIT_FAILURE);
  }
}

// COPY + MOVE VICTIM

void MoveVictim(std::vector<Device>& devices,
                DeviceType source,
                const size_t& block_id,
                const size_t& block_status,
                double& total_duration);

void Copy(std::vector<Device>& devices,
          DeviceType destination,
          DeviceType source,
          const size_t& block_id,
          const size_t& block_status,
          double& total_duration){

  DLOG(INFO) << "COPY : " << block_id << " " << " " \
      << DeviceTypeToString(source) << " " \
      << "---> " << DeviceTypeToString(destination) << " " \
      << CleanStatus(block_status) << "\n";

  // Write to destination device
  auto device_offset = GetDeviceOffset(devices, destination);
  auto last_device_type = devices.back().device_type;
  auto device_cache = devices[device_offset].cache;
  auto final_block_status = block_status;
  if(last_device_type == destination){
    final_block_status = CLEAN_BLOCK;
  }
  auto victim = device_cache.Put(block_id, final_block_status);

  total_duration += GetReadLatency(devices, source, block_id);
  total_duration += GetWriteLatency(devices, destination, block_id);

  // Move victim
  auto victim_key = victim.block_id;
  auto victim_status = victim.block_type;
  MoveVictim(devices,
             destination,
             victim_key,
             victim_status,
             total_duration);

}

size_t GetSizeRatio(const SizeRatioType& size_ratio){

  switch (size_ratio) {
    case SIZE_RATIO_TYPE_1:
      return 32;
    case SIZE_RATIO_TYPE_2:
      return 64;
    case SIZE_RATIO_TYPE_3:
      return 128;
    case SIZE_RATIO_TYPE_4:
      return 256;
    default:
      return -1;
  }

}

void MoveVictim(std::vector<Device>& devices,
                DeviceType source,
                const size_t& block_id,
                const size_t& block_status,
                double& total_duration){

  bool victim_exists = (block_id != INVALID_KEY);
  bool memory_device = (source == DeviceType::DEVICE_TYPE_CACHE ||
      source == DeviceType::DEVICE_TYPE_DRAM);
  bool on_nvm = (source == DeviceType::DEVICE_TYPE_NVM);
  bool is_dirty = (block_status == DIRTY_BLOCK);

  if(victim_exists == true) {
    DLOG(INFO) << "Move victim   : " << block_id << " :: ";
    DLOG(INFO) << "Memory device : " << DeviceTypeToString(source) << " :: ";
    DLOG(INFO) << CleanStatus(block_status) << "\n";
  }

  if(victim_exists){
    // Dirty victim
    if((memory_device && is_dirty) || on_nvm){
      auto destination = GetLowerDevice(devices, source);

      // Copy to lower device
      Copy(devices,
           destination,
           source,
           block_id,
           block_status,
           total_duration);
    }
  }

}

// DEVICE FACTORY

Device DeviceFactory::GetDevice(const DeviceType& device_type,
                                const configuration& state,
                                const DeviceType& last_device_type){

  // SIZES (4K blocks)

  size_t scale_factor = 1000 / 4;

  // CACHE size
  device_size[DEVICE_TYPE_CACHE] = 32;

  // SSD size
  device_size[DEVICE_TYPE_DISK] = 64 * 1024;

  // DRAM size
  switch(state.size_type){

    case SIZE_TYPE_1: {
      device_size[DEVICE_TYPE_DRAM] = 1024 * 4;
      break;
    }

    case SIZE_TYPE_2: {
      device_size[DEVICE_TYPE_DRAM] = 1024 * 8;
      break;
    }

    case SIZE_TYPE_3: {
      device_size[DEVICE_TYPE_DRAM] = 1024 * 16;
      break;
    }

    case SIZE_TYPE_4: {
      device_size[DEVICE_TYPE_DRAM] = 1024 * 32;
      break;
    }

    default:{
      std::cout << "Invalid size type: " << state.size_type << "\n";
      exit(EXIT_FAILURE);
    }
  }

  // NVM size
  device_size[DEVICE_TYPE_NVM] = device_size[DEVICE_TYPE_DRAM];
  device_size[DEVICE_TYPE_NVM] *= GetSizeRatio(state.size_ratio_type);

  switch (device_type){
    case DEVICE_TYPE_CACHE:
    case DEVICE_TYPE_DRAM:
    case DEVICE_TYPE_NVM:
    case DEVICE_TYPE_DISK:  {
      // Check for last device
      auto size = device_size[device_type];
      if(last_device_type == device_type){
        size = 1024 * 1024;
      }
      return Device(device_type,
                    state.caching_type,
                    size * scale_factor
      );
    }

    case DEVICE_TYPE_INVALID:
    default: {
      std::cout << "GetDevice: Get invalid device";
      exit(EXIT_FAILURE);
    }
  }

}


}  // End machine namespace

