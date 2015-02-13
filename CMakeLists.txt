cmake_policy(SET CMP0015 NEW)
set(CMAKE_AUTOMOC OFF)

aux_source_directory(. SRC_LIST)

include_directories(..)
include_directories(BEFORE ${JSONCPP_INCLUDE_DIRS})
include_directories(${Boost_INCLUDE_DIRS})

set(EXECUTABLE solc)

file(GLOB HEADERS "*.h")
add_executable(${EXECUTABLE} ${SRC_LIST} ${HEADERS})

target_link_libraries(${EXECUTABLE} ${Boost_FILESYSTEM_LIBRARIES})
target_link_libraries(${EXECUTABLE} ${Boost_PROGRAM_OPTIONS_LIBRARIES})
target_link_libraries(${EXECUTABLE} solidity)

install( TARGETS ${EXECUTABLE} DESTINATION bin )

