set(sources
	CommonData.cpp
	CommonIO.cpp
	Exceptions.cpp
	IndentedWriter.cpp
	JSON.cpp
	Keccak256.cpp
	StringUtils.cpp
	SwarmHash.cpp
	UTF8.cpp
	Whiskers.cpp
)

add_library(devcore ${sources})
target_link_libraries(devcore PRIVATE jsoncpp ${Boost_FILESYSTEM_LIBRARIES} ${Boost_REGEX_LIBRARIES} ${Boost_SYSTEM_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
target_include_directories(devcore PUBLIC "${CMAKE_SOURCE_DIR}")
target_include_directories(devcore SYSTEM PUBLIC ${Boost_INCLUDE_DIRS})
add_dependencies(devcore solidity_BuildInfo.h)
