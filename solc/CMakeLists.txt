aux_source_directory(. SRC_LIST)
list(REMOVE_ITEM SRC_LIST "./jsonCompiler.cpp")

include_directories(BEFORE ..)

set(EXECUTABLE solc)

file(GLOB HEADERS "*.h")
add_executable(${EXECUTABLE} ${SRC_LIST} ${HEADERS})

eth_use(${EXECUTABLE} REQUIRED Solidity)
target_link_libraries(${EXECUTABLE} ${Boost_PROGRAM_OPTIONS_LIBRARIES})

if (APPLE)
	install(TARGETS ${EXECUTABLE} DESTINATION bin)
else()
	eth_install_executable(${EXECUTABLE})
endif()

add_library(soljson jsonCompiler.cpp ${HEADERS})
target_link_libraries(soljson solidity)
