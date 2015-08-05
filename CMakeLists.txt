cmake_policy(SET CMP0015 NEW)
set(CMAKE_AUTOMOC OFF)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DSTATICLIB")

aux_source_directory(. SRC_LIST)

include_directories(BEFORE ${JSONCPP_INCLUDE_DIRS})
include_directories(BEFORE ..)
include_directories(${Boost_INCLUDE_DIRS})

set(EXECUTABLE evmasm)

file(GLOB HEADERS "*.h")

if (ETH_STATIC)
	add_library(${EXECUTABLE} STATIC ${SRC_LIST} ${HEADERS})
else()
	add_library(${EXECUTABLE} SHARED ${SRC_LIST} ${HEADERS})
endif()

target_link_libraries(${EXECUTABLE} evmcore)
target_link_libraries(${EXECUTABLE} devcrypto)

install( TARGETS ${EXECUTABLE} RUNTIME DESTINATION bin ARCHIVE DESTINATION lib LIBRARY DESTINATION lib )
install( FILES ${HEADERS} DESTINATION include/${EXECUTABLE} )

