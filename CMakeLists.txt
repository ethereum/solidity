cmake_policy(SET CMP0015 NEW)
set(CMAKE_AUTOMOC OFF)

aux_source_directory(. SRC_LIST)
list(REMOVE_ITEM SRC_LIST "./jsonCompiler.cpp")

include_directories(BEFORE ${JSONCPP_INCLUDE_DIRS})
include_directories(BEFORE ..)
include_directories(${Boost_INCLUDE_DIRS})

set(EXECUTABLE solc)

file(GLOB HEADERS "*.h")
add_executable(${EXECUTABLE} ${SRC_LIST} ${HEADERS})

target_link_libraries(${EXECUTABLE} ${Boost_FILESYSTEM_LIBRARIES})
target_link_libraries(${EXECUTABLE} ${Boost_PROGRAM_OPTIONS_LIBRARIES})
target_link_libraries(${EXECUTABLE} solidity)

install( TARGETS ${EXECUTABLE} DESTINATION bin )

if (ETH_STATIC)
	add_library(soljson STATIC jsonCompiler.cpp ${HEADERS})
else()
	add_library(soljson SHARED jsonCompiler.cpp ${HEADERS})
endif()
target_link_libraries(soljson solidity)
