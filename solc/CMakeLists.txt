set(
	sources
	CommandLineInterface.cpp CommandLineInterface.h
	main.cpp
)

add_executable(solc ${sources})
target_link_libraries(solc PRIVATE solidity ${Boost_PROGRAM_OPTIONS_LIBRARIES})

include(GNUInstallDirs)
install(TARGETS solc DESTINATION "${CMAKE_INSTALL_BINDIR}")

if (EMSCRIPTEN)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s EXPORTED_FUNCTIONS='[\"_compileJSON\",\"_license\",\"_version\",\"_compileJSONMulti\",\"_compileJSONCallback\",\"_compileStandard\"]' -s RESERVED_FUNCTION_POINTERS=20")
	add_executable(soljson jsonCompiler.cpp)
else()
	add_library(soljson jsonCompiler.cpp)
endif()
target_link_libraries(soljson PRIVATE solidity)