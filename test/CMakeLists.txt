cmake_policy(SET CMP0015 NEW)

aux_source_directory(. SRC_LIST)
aux_source_directory(contracts SRC_LIST)
aux_source_directory(libsolidity SRC_LIST)

get_filename_component(TESTS_DIR "${CMAKE_CURRENT_SOURCE_DIR}" ABSOLUTE)

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

file(GLOB HEADERS "*.h" "*/*.h")
set(EXECUTABLE soltest)
eth_simple_add_executable(${EXECUTABLE} ${SRC_LIST} ${HEADERS})

eth_use(${EXECUTABLE} REQUIRED Solidity::solidity Eth::ethereum Eth::ethcore)

include_directories(BEFORE ..)
target_link_libraries(${EXECUTABLE} ${Boost_UNIT_TEST_FRAMEWORK_LIBRARIES})

enable_testing()
set(CTEST_OUTPUT_ON_FAILURE TRUE)
