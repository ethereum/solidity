add_subdirectory(ossfuzz)

add_subdirectory(yulInterpreter)
add_executable(yulrun yulrun.cpp)
target_link_libraries(yulrun PRIVATE yulInterpreter libsolc evmasm Boost::boost Boost::program_options)

add_executable(solfuzzer afl_fuzzer.cpp fuzzer_common.cpp)
target_link_libraries(solfuzzer PRIVATE libsolc evmasm Boost::boost Boost::program_options Boost::system)

add_executable(yulopti yulopti.cpp)
target_link_libraries(yulopti PRIVATE solidity Boost::boost Boost::program_options Boost::system)

add_executable(end-to-end-extractor
	end-to-end-extractor/main.cpp
	end-to-end-extractor/EndToEndTests.h
	end-to-end-extractor/EndToEndTests.cpp
	end-to-end-extractor/ExtractorExecutionFramework.h
	end-to-end-extractor/ExtractorExecutionFramework.cpp
	end-to-end-extractor/ExtractionTask.h
	end-to-end-extractor/ExtractionTask.cpp
	end-to-end-extractor/FakeEvmHost.hpp 
	end-to-end-extractor/FakeCompilerStack.hpp 
	end-to-end-extractor/EndToEndCppTestFile.cpp 
	end-to-end-extractor/EndToEndCppTestFile.h
	../Common.cpp
	../libsolidity/util/BytesUtils.cpp
	../libsolidity/util/TestFileParser.cpp
	../libsolidity/util/ContractABIUtils.cpp
)
target_compile_definitions(end-to-end-extractor PRIVATE SOLIDITY_ROOT="${CMAKE_SOURCE_DIR}")
target_link_libraries(end-to-end-extractor PRIVATE evmc libsolc solidity yulInterpreter evmasm Boost::boost Boost::program_options Boost::system)

add_executable(isoltest
	isoltest.cpp
	IsolTestOptions.cpp
	../Common.cpp
	../CommonSyntaxTest.cpp
	../EVMHost.cpp
	../TestCase.cpp
	../libsolidity/util/BytesUtils.cpp
	../libsolidity/util/ContractABIUtils.cpp
	../libsolidity/util/TestFileParser.cpp
	../libsolidity/util/TestFunctionCall.cpp
	../libsolidity/GasTest.cpp
	../libsolidity/SyntaxTest.cpp
	../libsolidity/SemanticTest.cpp
	../libsolidity/AnalysisFramework.cpp
	../libsolidity/SolidityExecutionFramework.cpp
	../ExecutionFramework.cpp
	../libsolidity/ABIJsonTest.cpp
	../libsolidity/ASTJSONTest.cpp
	../libsolidity/SMTCheckerTest.cpp
	../libsolidity/SMTCheckerJSONTest.cpp
	../libyul/Common.cpp
	../libyul/EwasmTranslationTest.cpp
	../libyul/FunctionSideEffects.cpp
	../libyul/ObjectCompilerTest.cpp
	../libyul/SyntaxTest.cpp
	../libyul/YulOptimizerTest.cpp
	../libyul/YulInterpreterTest.cpp
)
target_link_libraries(isoltest PRIVATE evmc libsolc solidity yulInterpreter evmasm Boost::boost Boost::program_options Boost::unit_test_framework)
