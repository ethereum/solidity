cmake_policy(SET CMP0015 NEW)
set(CMAKE_AUTOMOC OFF)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DSTATICLIB")

aux_source_directory(. SRC_LIST)

include_directories(${Boost_INCLUDE_DIRS})
include_directories(..)

set(EXECUTABLE solidity)

file(GLOB HEADERS "*.h") 
if(ETH_STATIC)
	add_library(${EXECUTABLE} STATIC ${SRC_LIST} ${HEADERS})
else()
	add_library(${EXECUTABLE} SHARED ${SRC_LIST} ${HEADERS})
endif()

target_link_libraries(${EXECUTABLE} evmcore devcore)
# TODO: Temporary until PR 532 https://github.com/ethereum/cpp-ethereum/pull/532
# gets accepted. Then we can simply add jsoncpp as a dependency and not the
# whole of JSONRPC as we are doing right here
target_link_libraries(${EXECUTABLE} ${JSONRPC_LS})

install( TARGETS ${EXECUTABLE} ARCHIVE DESTINATION lib LIBRARY DESTINATION lib )
install( FILES ${HEADERS} DESTINATION include/${EXECUTABLE} )

