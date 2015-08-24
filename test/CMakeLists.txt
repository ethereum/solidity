cmake_policy(SET CMP0015 NEW)

aux_source_directory(. SRC_LIST)

macro (add_sources)
	file (RELATIVE_PATH _relPath "${CMAKE_SOURCE_DIR}/test" "${CMAKE_CURRENT_SOURCE_DIR}")
	foreach (_src ${ARGN})
		if (_relPath)
			list (APPEND SRC "${_relPath}/${_src}")
		else()
			list (APPEND SRC "${_src}")
		endif()
	endforeach()
	if (_relPath)
		# propagate SRCS to parent directory
		set (SRC ${SRC} PARENT_SCOPE)
	endif()
endmacro()

add_subdirectory(contracts)
add_subdirectory(libsolidity)

set(SRC_LIST ${SRC_LIST} ${SRC})

# search for test names and create ctest tests
enable_testing()
foreach(file ${SRC_LIST})
	file(STRINGS ${CMAKE_CURRENT_SOURCE_DIR}/${file} test_list_raw REGEX "BOOST_.*TEST_(SUITE|CASE)")
	set(TestSuite "DEFAULT")
	foreach(test_raw ${test_list_raw})
		string(REGEX REPLACE ".*TEST_(SUITE|CASE)\\(([^ ,\\)]*).*" "\\1 \\2" test ${test_raw})
		if(test MATCHES "^SUITE .*")
			string(SUBSTRING ${test} 6 -1 TestSuite)
		elseif(test MATCHES "^CASE .*")
			string(SUBSTRING ${test} 5 -1 TestCase)
			add_test(NAME ${TestSuite}/${TestCase} WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/test COMMAND test -t ${TestSuite}/${TestCase})
		endif(test MATCHES "^SUITE .*")
	endforeach(test_raw)
endforeach(file)

file(GLOB HEADERS "*.h")
set(EXECUTABLE soltest)
add_executable(${EXECUTABLE} ${SRC_LIST} ${HEADERS})

eth_use(${EXECUTABLE} REQUIRED EthCore)

include_directories(BEFORE ..)
include_directories(BEFORE ${JSONCPP_INCLUDE_DIRS})
target_link_libraries(${EXECUTABLE} ${Boost_UNIT_TEST_FRAMEWORK_LIBRARIES})
target_link_libraries(${EXECUTABLE} ${ETH_ETHEREUM_LIBRARY})
target_link_libraries(${EXECUTABLE} ${ETH_ETHCORE_LIBRARY})
target_link_libraries(${EXECUTABLE} solidity)

enable_testing()
set(CTEST_OUTPUT_ON_FAILURE TRUE)
