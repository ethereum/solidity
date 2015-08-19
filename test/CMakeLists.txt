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

add_subdirectory(fuzzTesting)
add_subdirectory(libdevcore)
add_subdirectory(libdevcrypto)
add_subdirectory(libethcore)
add_subdirectory(libethereum)
add_subdirectory(libevm)
add_subdirectory(libnatspec)
add_subdirectory(libp2p)
add_subdirectory(external-dependencies)

if (JSCONSOLE)
	add_subdirectory(libjsengine)
endif()

if (SOLIDITY)
	add_subdirectory(libsolidity)
	add_subdirectory(contracts)
endif ()
if (JSONRPC)
add_subdirectory(libweb3jsonrpc)
endif ()
add_subdirectory(libwhisper)

set(SRC_LIST ${SRC_LIST} ${SRC})

include_directories(BEFORE ${JSONCPP_INCLUDE_DIRS})
include_directories(BEFORE ..)
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${CRYPTOPP_INCLUDE_DIRS})
include_directories(${JSON_RPC_CPP_INCLUDE_DIRS})

if (JSCONSOLE)
	include_directories(${V8_INCLUDE_DIRS})
endif()

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

target_link_libraries(testeth ${Boost_UNIT_TEST_FRAMEWORK_LIBRARIES})
target_link_libraries(testeth ${CURL_LIBRARIES})
target_link_libraries(testeth ${CRYPTOPP_LIBRARIES})
target_link_libraries(testeth ethereum)
target_link_libraries(testeth ethcore)
if (NOT WIN32)
	target_link_libraries(testeth secp256k1)
endif ()

if (JSCONSOLE)
	target_link_libraries(testeth jsengine)
endif()

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

