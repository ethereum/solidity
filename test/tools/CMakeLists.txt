add_executable(solfuzzer fuzzer.cpp)
target_link_libraries(solfuzzer PRIVATE libsolc evmasm ${Boost_PROGRAM_OPTIONS_LIBRARIES})
