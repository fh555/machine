// MACHINE SOURCE

#include <iostream>
#include <fstream>

#include "configuration.h"
#include "workload.h"
#include "device.h"

namespace machine {

extern configuration state;

// Main Entry Point
void RunBenchmark() {

  // Run a single machine test
  RunMachineTest();

}

}  // namespace machine

int main(int argc, char **argv) {

  // Initialize Google's logging library.
  google::InitGoogleLogging(argv[0]);

  machine::ParseArguments(
      argc, argv, machine::state);

  machine::BootstrapDeviceMetrics(machine::state);

  // Construct device list
  machine::ConstructDeviceList(machine::state);

  machine::RunBenchmark();

  return 0;
}
