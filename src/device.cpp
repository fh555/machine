// DEVICE SOURCE

#include "macros.h"
#include "device.h"
#include "configuration.h"
#include "stats.h"

#define _FILE_OFFSET_BITS  64

namespace machine {

std::map<DeviceType, size_t> device_size;
std::map<DeviceType, double> seq_read_latency;
std::map<DeviceType, double> seq_write_latency;
std::map<DeviceType, double> rnd_read_latency;
std::map<DeviceType, double> rnd_write_latency;

// Machine stats
Stats machine_stats;

Timer<std::ratio<1, 1000 * 1000 * 1000>> physical_timer;

void BootstrapDeviceMetrics(const configuration &state){

  // LATENCIES (ns)

  // CACHE
  seq_read_latency[DEVICE_TYPE_CACHE] = 10;
  seq_write_latency[DEVICE_TYPE_CACHE] = 10;
  rnd_read_latency[DEVICE_TYPE_CACHE] = 10;
  rnd_write_latency[DEVICE_TYPE_CACHE] = 10;

  // DRAM
  seq_read_latency[DEVICE_TYPE_DRAM] = 1000;
  seq_write_latency[DEVICE_TYPE_DRAM] = 2000;
  rnd_read_latency[DEVICE_TYPE_DRAM] = 2000;
  rnd_write_latency[DEVICE_TYPE_DRAM] = 2500;

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
    seq_read_latency[DEVICE_TYPE_DISK] = 30 * 1000;
    seq_write_latency[DEVICE_TYPE_DISK] = 100 * 1000;
    rnd_read_latency[DEVICE_TYPE_DISK] = 50 * 1000;
    rnd_write_latency[DEVICE_TYPE_DISK] = 150 * 1000;
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

std::vector<DeviceType> emulated_device_types =
{
    DeviceType::DEVICE_TYPE_DRAM,
    DeviceType::DEVICE_TYPE_NVM,
    DeviceType::DEVICE_TYPE_DISK
};

std::map<DeviceType, bool> is_device_emulated;
std::map<DeviceType, off64_t> file_sizes;
std::map<DeviceType, std::string> file_paths;
std::map<DeviceType, FILE*> file_pointers;

std::string write_buffer;
char read_buffer[BLOCK_SIZE*2];

std::string random_string(size_t length){
  auto randchar = []() -> char
      {
    const char charset[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    const size_t max_index = (sizeof(charset) - 1);
    return charset[ rand() % max_index ];
      };
  std::string str(length,0);
  std::generate_n( str.begin(), length, randchar );
  return str;
}

void BootstrapFileSystemForEmulation(const configuration& state){

  // Setup buffer
  write_buffer = random_string(BLOCK_SIZE);

  is_device_emulated[DeviceType::DEVICE_TYPE_INVALID] = false;
  is_device_emulated[DeviceType::DEVICE_TYPE_CACHE] = false;
  is_device_emulated[DeviceType::DEVICE_TYPE_DRAM] = true;
  is_device_emulated[DeviceType::DEVICE_TYPE_NVM] = true;
  is_device_emulated[DeviceType::DEVICE_TYPE_DISK] = true;

  // Large file mode
  if(state.large_file_mode == false){
    file_sizes[DeviceType::DEVICE_TYPE_DRAM] = 1 * (1024 * 1024 * 1024);
    file_sizes[DeviceType::DEVICE_TYPE_NVM] = 1 * (1024 * 1024 * 1024);
    file_sizes[DeviceType::DEVICE_TYPE_DISK] = 1 * (1024 * 1024 * 1024);
  }
  else {
    file_sizes[DeviceType::DEVICE_TYPE_DRAM] = 1 * (1024 * 1024 * 1024);
    file_sizes[DeviceType::DEVICE_TYPE_NVM] = 8 * (1024 * 1024 * 1024L);
    file_sizes[DeviceType::DEVICE_TYPE_DISK] = 32 * (1024 * 1024 * 1024L);
  }

  file_paths[DeviceType::DEVICE_TYPE_DRAM] = "/data1/database";
  file_paths[DeviceType::DEVICE_TYPE_NVM] = "/mnt/pmfs/database";
  file_paths[DeviceType::DEVICE_TYPE_DISK] = "/data2/database";

  // Create and truncate files
  for(auto device_type : emulated_device_types){
    FILE *file_pointer = NULL;
    int status;

    file_pointer = fopen(file_paths[device_type].c_str(), "w");
    if(file_pointer == NULL) {
      std::cout << "Could not open file: " << file_paths[device_type] << "\n";
      exit(EXIT_FAILURE);
    }

    // Size in GB
    auto file_size = file_sizes[device_type];
    status = ftruncate(fileno(file_pointer), file_size);
    if(status != 0){
      std::cout << "Could not truncate file: " << file_paths[device_type] << "\n";
      exit(EXIT_FAILURE);
    }
    else {
      std::cout << "Truncated file: " << file_paths[device_type] << " file_size: " << file_size << "\n";
    }

    // Sync
    status = fsync(fileno(file_pointer));
    if(status != 0){
      std::cout << "Could not sync file: " << file_paths[device_type] << "\n";
      exit(EXIT_FAILURE);
    }

    // Close file
    status = fclose(file_pointer);
    if(status != 0){
      std::cout << "Could not close file: " << file_paths[device_type] << "\n";
      exit(EXIT_FAILURE);
    }

  }

  std::cout << "Bootstrapped File System \n";

  // Open all files
  for(auto device_type : emulated_device_types){
    FILE *file_pointer = NULL;

    file_pointer = fopen(file_paths[device_type].c_str(), "rw+");
    if(file_pointer == NULL) {
      std::cout << "Could not open file: " << file_paths[device_type] << "\n";
      exit(EXIT_FAILURE);
    }

    // Cache file pointer
    file_pointers[device_type] = file_pointer;

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

size_t sync_frequency = 1;

size_t GetWriteLatency(std::vector<Device>& devices,
                       DeviceType device_type,
                       const size_t& block_id,
                       const bool& flush_block){

  DLOG(INFO) << "WRITE :: " << DeviceTypeToString(device_type) << "\n";

  // Increment stats
  machine_stats.IncrementWriteCount(device_type);
  if(flush_block == true){
    machine_stats.IncrementFlushCount(device_type);
  }

  // Emulate if needed
  if(emulate == true && is_device_emulated[device_type] == true){
    // Seek
    auto max_size = file_sizes[device_type];
    auto location = (block_id * BLOCK_SIZE) % max_size;

    physical_timer.Start();
    auto status = fseek(file_pointers[device_type], location, SEEK_SET);
    physical_timer.Stop();

    if(status != 0){
      perror("seek");
      exit(EXIT_FAILURE);
    }

    // Write
    physical_timer.Start();
    auto write_size = fwrite(write_buffer.c_str(), write_buffer.size(), 1,
                             file_pointers[device_type]);
    physical_timer.Stop();

    if(write_size == 0){
      std::cout << "Writing error: " << file_paths[device_type];
      std::cout << " "  << file_paths[device_type];
      std::cout << " "  << "write_size: " << write_size << "\n";
      perror("write");
      exit(EXIT_FAILURE);
    }

    // Flush if needed
    if(flush_block == true){

      physical_timer.Start();
      status = fflush(file_pointers[device_type]);
      physical_timer.Stop();

      if(status != 0){
        perror("fflush");
        exit(EXIT_FAILURE);
      }

      // Sync if needed
      auto sync = rand() % sync_frequency;
      if(sync == 0){

        physical_timer.Start();
        status = fsync(fileno(file_pointers[device_type]));
        physical_timer.Stop();

        if(status != 0){
          perror("fsync");
          exit(EXIT_FAILURE);
        }
        machine_stats.IncrementSyncCount(device_type);
      }

    }

  }

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

  // Emulate if needed
  if(emulate == true && is_device_emulated[device_type] == true){
    // Seek
    auto max_size = file_sizes[device_type];
    auto location = (block_id * BLOCK_SIZE) % max_size;

    physical_timer.Start();
    auto status = fseek(file_pointers[device_type], location, SEEK_SET);
    physical_timer.Stop();

    if(status != 0){
      perror("seek");
      exit(EXIT_FAILURE);
    }

    // Read
    physical_timer.Start();
    auto read_size = fread(read_buffer, BLOCK_SIZE, 1, file_pointers[device_type]);
    physical_timer.Stop();

    if(read_size != 1){
      std::cout << "Reading error: " << file_pointers[device_type];
      std::cout << " "  << file_paths[device_type];
      std::cout << " "  << "read_size: " << read_size << "\n";
      perror("read");
      exit(EXIT_FAILURE);
    }

  }

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
      else if(nvm_exists == true) {
        destination = DEVICE_TYPE_NVM;
      }
      else{
        destination = DEVICE_TYPE_DISK;
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
          const bool& flush_block,
          double& total_duration){

  DLOG(INFO) << "COPY : " << block_id << " " << " " \
      << DeviceTypeToString(source) << " " \
      << "---> " << DeviceTypeToString(destination) << " " \
      << CleanStatus(block_status) << "\n";

  // Increment stats
  machine_stats.IncrementOpCount(source, destination);

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
  total_duration += GetWriteLatency(devices, destination, block_id, flush_block);

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
  auto flush_block = false;

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
           flush_block,
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

