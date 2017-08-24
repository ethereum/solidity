file(GLOB_RECURSE sources "*.cpp")
list(REMOVE_ITEM sources "${CMAKE_CURRENT_SOURCE_DIR}/fuzzer.cpp")
file(GLOB_RECURSE headers "*.h")

add_executable(soltest ${sources} ${headers})
target_link_libraries(soltest PRIVATE soljson solidity lll evmasm devcore ${Boost_UNIT_TEST_FRAMEWORK_LIBRARIES})

add_executable(solfuzzer fuzzer.cpp)
target_link_libraries(solfuzzer soljson evmasm ${Boost_PROGRAM_OPTIONS_LIBRARIES})
