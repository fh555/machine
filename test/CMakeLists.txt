##################################################################################

## TESTS

# ---[ FIFO TEST
add_executable(policy_fifo_test policy_fifo_test.cpp)
target_link_libraries(policy_fifo_test machine_library
${GTEST_BOTH_LIBRARIES} 
${GLOG_LIBRARIES} 
${CMAKE_THREAD_LIBS_INIT}
)
add_test(NAME FIFOTest COMMAND policy_fifo_test)

# ---[ LRU TEST
add_executable(policy_lru_test policy_lru_test.cpp)
target_link_libraries(policy_lru_test machine_library 
${GTEST_BOTH_LIBRARIES} 
${GLOG_LIBRARIES} 
${CMAKE_THREAD_LIBS_INIT}
)
add_test(NAME LRUTest COMMAND policy_lru_test)

# ---[ LFU TEST
add_executable(policy_lfu_test policy_lfu_test.cpp)
target_link_libraries(policy_lfu_test machine_library 
${GTEST_BOTH_LIBRARIES} 
${GLOG_LIBRARIES} 
${CMAKE_THREAD_LIBS_INIT}
)
add_test(NAME LFUTest COMMAND policy_lfu_test)

# ---[ DISTRIBUTION TEST
add_executable(distribution_test distribution_test.cpp)
target_link_libraries(distribution_test machine_library
${GTEST_BOTH_LIBRARIES} 
${GLOG_LIBRARIES} 
${CMAKE_THREAD_LIBS_INIT}
)
add_test(NAME DistributionTest COMMAND distribution_test)

## MACHINE

# ---[ MACHINE
add_executable(machine machine.cpp)
target_link_libraries(machine machine_library 
${GTEST_BOTH_LIBRARIES} 
${GLOG_LIBRARIES} 
${CMAKE_THREAD_LIBS_INIT}
)
add_test(NAME MachineTest COMMAND machine)

# --[ Add "make check" target

set(CTEST_FLAGS "")
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} ${CTEST_FLAGS} --verbose)

