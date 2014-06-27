cmake_policy(SET CMP0015 NEW)

aux_source_directory(. SRC_LIST)

include_directories(..)

set(EXECUTABLE lllc)

add_executable(${EXECUTABLE} ${SRC_LIST})

if (JSONRPC_LS)
	add_definitions(-DETH_JSONRPC)
	include_directories(${JSONRPC_ID})
	target_link_libraries(${EXECUTABLE} ${JSONRPC_LS})
endif ()

if (READLINE_LS)
	add_definitions(-DETH_READLINE)
	include_directories(${READLINE_ID})
	target_link_libraries(${EXECUTABLE} ${READLINE_LS})
endif ()

if (${TARGET_PLATFORM} STREQUAL "w64")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++")
	target_link_libraries(${EXECUTABLE} gcc)
	target_link_libraries(${EXECUTABLE} gdi32)
	target_link_libraries(${EXECUTABLE} ws2_32)
	target_link_libraries(${EXECUTABLE} mswsock)
	target_link_libraries(${EXECUTABLE} shlwapi)
	target_link_libraries(${EXECUTABLE} iphlpapi)
	target_link_libraries(${EXECUTABLE} cryptopp)
	target_link_libraries(${EXECUTABLE} boost_system-mt-s)
	target_link_libraries(${EXECUTABLE} boost_filesystem-mt-s)
	target_link_libraries(${EXECUTABLE} boost_thread_win32-mt-s)
	set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS)
elseif (UNIX)
else ()
	target_link_libraries(${EXECUTABLE} ${CRYPTOPP_LIBRARIES})
	target_link_libraries(${EXECUTABLE} boost_system)
	target_link_libraries(${EXECUTABLE} boost_filesystem)
	find_package(Threads REQUIRED)
	target_link_libraries(${EXECUTABLE} ${CMAKE_THREAD_LIBS_INIT})
endif ()

target_link_libraries(${EXECUTABLE} lll)
target_link_libraries(${EXECUTABLE} ethcore)
target_link_libraries(${EXECUTABLE} ${MINIUPNPC_LS})
target_link_libraries(${EXECUTABLE} ${LEVELDB_LS})
target_link_libraries(${EXECUTABLE} gmp)

install( TARGETS ${EXECUTABLE} DESTINATION bin )

