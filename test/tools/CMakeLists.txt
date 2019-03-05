if (OSSFUZZ)
    add_subdirectory(ossfuzz)
endif()

add_subdirectory(yulInterpreter)
add_executable(yulrun yulrun.cpp)
target_link_libraries(yulrun PRIVATE yulInterpreter libsolc evmasm ${Boost_PROGRAM_OPTIONS_LIBRARIES})

add_executable(solfuzzer afl_fuzzer.cpp fuzzer_common.cpp)
target_link_libraries(solfuzzer PRIVATE libsolc evmasm ${Boost_PROGRAM_OPTIONS_LIBRARIES} ${Boost_SYSTEM_LIBRARIES})

add_executable(yulopti yulopti.cpp)
target_link_libraries(yulopti PRIVATE solidity ${Boost_PROGRAM_OPTIONS_LIBRARIES} ${Boost_SYSTEM_LIBRARIES})

add_executable(isoltest
	isoltest.cpp
	IsolTestOptions.cpp
	../Options.cpp
	../Common.cpp
	../TestCase.cpp
	../libsolidity/util/TestFileParser.cpp
	../libsolidity/util/TestFunctionCall.cpp
	../libsolidity/SyntaxTest.cpp
	../libsolidity/SemanticTest.cpp
	../libsolidity/AnalysisFramework.cpp
	../libsolidity/SolidityExecutionFramework.cpp
	../ExecutionFramework.cpp
	../RPCSession.cpp
	../libsolidity/ASTJSONTest.cpp
	../libsolidity/SMTCheckerJSONTest.cpp
	../libyul/ObjectCompilerTest.cpp
	../libyul/YulOptimizerTest.cpp
	../libyul/YulInterpreterTest.cpp
)
target_link_libraries(isoltest PRIVATE libsolc solidity yulInterpreter evmasm ${Boost_PROGRAM_OPTIONS_LIBRARIES} ${Boost_UNIT_TEST_FRAMEWORK_LIBRARIES})
