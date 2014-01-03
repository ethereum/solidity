cmake_policy(SET CMP0015 NEW)

aux_source_directory(. SRC_LIST)
include_directories(../libethereum)
link_directories(../libethereum)

add_executable(testeth ${SRC_LIST})

target_link_libraries(testeth libethereum)
