cmake_policy(SET CMP0015 NEW)

aux_source_directory(. SRC_LIST)

include_directories(..)

set(EXECUTABLE solc)

add_executable(${EXECUTABLE} ${SRC_LIST})

target_link_libraries(${EXECUTABLE} boost_filesystem)
target_link_libraries(${EXECUTABLE} boost_program_options)
target_link_libraries(${EXECUTABLE} solidity)

install( TARGETS ${EXECUTABLE} DESTINATION bin )

