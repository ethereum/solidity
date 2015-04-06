cmake_policy(SET CMP0015 NEW)

aux_source_directory(. SRC_LIST)
list(REMOVE_ITEM SRC_LIST "./createRandomVMTest.cpp")
list(REMOVE_ITEM SRC_LIST "./createRandomStateTest.cpp")
list(REMOVE_ITEM SRC_LIST "./checkRandomVMTest.cpp")
list(REMOVE_ITEM SRC_LIST "./checkRandomStateTest.cpp")

if (NOT JSONRPC)
	list(REMOVE_ITEM SRC_LIST "./AccountHolder.cpp")
endif()

include_directories(BEFORE ${JSONCPP_INCLUDE_DIRS})
include_directories(BEFORE ..)
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${CRYPTOPP_INCLUDE_DIRS})
include_directories(${JSON_RPC_CPP_INCLUDE_DIRS})

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
			add_test(NAME ${TestSuite}/${TestCase} WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/test COMMAND testeth -t ${TestSuite}/${TestCase})
		endif(test MATCHES "^SUITE .*")
	endforeach(test_raw)
endforeach(file)

file(GLOB HEADERS "*.h")
add_executable(testeth ${SRC_LIST} ${HEADERS})
add_executable(createRandomVMTest createRandomVMTest.cpp vm.cpp TestHelper.cpp Stats.cpp)
add_executable(createRandomStateTest createRandomStateTest.cpp TestHelper.cpp Stats.cpp)
add_executable(checkRandomVMTest checkRandomVMTest.cpp vm.cpp TestHelper.cpp Stats.cpp)
add_executable(checkRandomStateTest checkRandomStateTest.cpp TestHelper.cpp Stats.cpp)

target_link_libraries(testeth ${Boost_UNIT_TEST_FRAMEWORK_LIBRARIES})
target_link_libraries(testeth ${CURL_LIBRARIES})
target_link_libraries(testeth ethereum)
target_link_libraries(testeth ethcore)
target_link_libraries(testeth secp256k1)
if (SOLIDITY)
	target_link_libraries(testeth solidity)
endif ()
target_link_libraries(testeth testutils)
if (GUI AND NOT JUSTTESTS)
	target_link_libraries(testeth webthree)
	target_link_libraries(testeth natspec)
endif()
if (JSONRPC)
	target_link_libraries(testeth web3jsonrpc)
	target_link_libraries(testeth ${JSON_RPC_CPP_CLIENT_LIBRARIES})
endif()

target_link_libraries(createRandomVMTest ${Boost_UNIT_TEST_FRAMEWORK_LIBRARIES})
target_link_libraries(createRandomVMTest ethereum)
target_link_libraries(createRandomVMTest ethcore)
target_link_libraries(createRandomVMTest  testutils)
target_link_libraries(createRandomStateTest ${Boost_UNIT_TEST_FRAMEWORK_LIBRARIES})
target_link_libraries(createRandomStateTest ethereum)
target_link_libraries(createRandomStateTest ethcore)
target_link_libraries(createRandomStateTest testutils)
target_link_libraries(checkRandomVMTest ${Boost_UNIT_TEST_FRAMEWORK_LIBRARIES})
target_link_libraries(checkRandomVMTest ethereum)
target_link_libraries(checkRandomVMTest ethcore)
target_link_libraries(checkRandomVMTest testutils)
target_link_libraries(checkRandomStateTest ${Boost_UNIT_TEST_FRAMEWORK_LIBRARIES})
target_link_libraries(checkRandomStateTest ethereum)
target_link_libraries(checkRandomStateTest ethcore)
target_link_libraries(checkRandomStateTest testutils)

enable_testing()
set(CTEST_OUTPUT_ON_FAILURE TRUE)

include(EthUtils)

eth_add_test(ClientBase
	ARGS --eth_testfile=BlockTests/bcJS_API_Test --eth_threads=1
	ARGS --eth_testfile=BlockTests/bcJS_API_Test --eth_threads=3
	ARGS --eth_testfile=BlockTests/bcJS_API_Test --eth_threads=10
	ARGS --eth_testfile=BlockTests/bcValidBlockTest --eth_threads=1
	ARGS --eth_testfile=BlockTests/bcValidBlockTest --eth_threads=3
	ARGS --eth_testfile=BlockTests/bcValidBlockTest --eth_threads=10
)

eth_add_test(JsonRpc
	ARGS --eth_testfile=BlockTests/bcJS_API_Test 
	ARGS --eth_testfile=BlockTests/bcValidBlockTest
)

