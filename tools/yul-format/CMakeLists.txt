add_executable(yul-format main.cpp)
target_link_libraries(yul-format PRIVATE yul ${Boost_PROGRAM_OPTIONS_LIBRARIES})
