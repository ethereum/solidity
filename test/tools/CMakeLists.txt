add_subdirectory(ossfuzz)

add_subdirectory(yulInterpreter)
add_executable(yulrun yulrun.cpp)
target_link_libraries(yulrun PRIVATE yulInterpreter libsolc evmasm Boost::boost Boost::program_options)

add_executable(solfuzzer afl_fuzzer.cpp fuzzer_common.cpp)
target_link_libraries(solfuzzer PRIVATE libsolc evmasm Boost::boost Boost::program_options Boost::system)

add_executable(yulopti yulopti.cpp)
target_link_libraries(yulopti PRIVATE solidity Boost::boost Boost::program_options Boost::system)

add_executable(isoltest
	isoltest.cpp
	IsolTestOptions.cpp
	../Options.cpp
	../Common.cpp
	../TestCase.cpp
	../libsolidity/util/TestFileParser.cpp
	../libsolidity/util/TestFunctionCall.cpp
	../libsolidity/GasTest.cpp
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
target_link_libraries(isoltest PRIVATE libsolc solidity yulInterpreter evmasm Boost::boost Boost::program_options Boost::unit_test_framework)
