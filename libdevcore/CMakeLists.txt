set(sources
	Algorithms.h
	AnsiColorized.h
	Assertions.h
	Common.h
	CommonData.cpp
	CommonData.h
	CommonIO.cpp
	CommonIO.h
	Exceptions.cpp
	Exceptions.h
	FixedHash.h
	IndentedWriter.cpp
	IndentedWriter.h
	InvertibleMap.h
	IpfsHash.cpp
	IpfsHash.h
	JSON.cpp
	JSON.h
	Keccak256.cpp
	Keccak256.h
	picosha2.h
	Result.h
	StringUtils.cpp
	StringUtils.h
	SwarmHash.cpp
	SwarmHash.h
	UTF8.cpp
	UTF8.h
	vector_ref.h
	Visitor.h
	Whiskers.cpp
	Whiskers.h
)

add_library(devcore ${sources})
target_link_libraries(devcore PUBLIC jsoncpp Boost::boost Boost::filesystem Boost::system)
target_include_directories(devcore PUBLIC "${CMAKE_SOURCE_DIR}")
add_dependencies(devcore solidity_BuildInfo.h)

if(SOLC_LINK_STATIC)
	target_link_libraries(devcore PUBLIC Threads::Threads)
endif()
