cmake_policy(SET CMP0015 NEW)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DSTATICLIB")

aux_source_directory(. SRC_LIST)

set(EXECUTABLE solidity)

if(ETH_STATIC)
	add_library(${EXECUTABLE} STATIC ${SRC_LIST})
else()
	add_library(${EXECUTABLE} SHARED ${SRC_LIST})
endif()

file(GLOB HEADERS "*.h") 

include_directories(..)

# @todo we only depend on Assembly, not on all of lll
target_link_libraries(${EXECUTABLE} evmface devcore lll)

install( TARGETS ${EXECUTABLE} ARCHIVE DESTINATION lib LIBRARY DESTINATION lib )
install( FILES ${HEADERS} DESTINATION include/${EXECUTABLE} )

