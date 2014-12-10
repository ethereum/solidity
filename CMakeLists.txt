cmake_policy(SET CMP0015 NEW)
set(CMAKE_AUTOMOC OFF)

aux_source_directory(. SRC_LIST)

include_directories(${Boost_INCLUDE_DIRS})
include_directories(..)

set(EXECUTABLE solc)

file(GLOB HEADERS "*.h")
add_executable(${EXECUTABLE} ${SRC_LIST} ${HEADERS})

target_link_libraries(${EXECUTABLE} boost_filesystem)
target_link_libraries(${EXECUTABLE} boost_program_options)
target_link_libraries(${EXECUTABLE} solidity)

install( TARGETS ${EXECUTABLE} DESTINATION bin )

