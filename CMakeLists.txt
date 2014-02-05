cmake_policy(SET CMP0015 NEW)

aux_source_directory(. SRC_LIST)

include_directories(../libethereum)
link_directories(../libethereum)

add_executable(testeth ${SRC_LIST})

find_package(Threads REQUIRED)

target_link_libraries(testeth ethereum)
target_link_libraries(testeth miniupnpc)
target_link_libraries(testeth ${CRYPTOPP_LIBRARIES})
target_link_libraries(testeth secp256k1)
target_link_libraries(testeth gmp)
target_link_libraries(testeth boost_system)
target_link_libraries(testeth boost_filesystem)
target_link_libraries(testeth ${CMAKE_THREAD_LIBS_INIT})
