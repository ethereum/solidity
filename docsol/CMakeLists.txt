aux_source_directory(. SRC_LIST)

include_directories(BEFORE ..)

set(EXECUTABLE docsol)

file(GLOB HEADERS "*.h")
eth_simple_add_executable(${EXECUTABLE} ${SRC_LIST} ${HEADERS})

eth_use(${EXECUTABLE} REQUIRED Solidity::solidity)
target_link_libraries(${EXECUTABLE} ${Boost_PROGRAM_OPTIONS_LIBRARIES})

if (APPLE)
	install(TARGETS ${EXECUTABLE} DESTINATION bin)
else()
	eth_install_executable(${EXECUTABLE})
endif()
