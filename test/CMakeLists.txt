set(sources
    boostTest.cpp
    Common.cpp
    Common.h
    EVMHost.cpp
    EVMHost.h
    ExecutionFramework.cpp
    ExecutionFramework.h
    InteractiveTests.h
    Metadata.cpp
    Metadata.h
    Options.cpp
    Options.h
    TestCase.cpp
    TestCase.h
)
detect_stray_source_files("${sources}" ".")

set(contracts_sources
    contracts/AuctionRegistrar.cpp
    contracts/ContractInterface.h
    contracts/FixedFeeRegistrar.cpp
    contracts/Wallet.cpp
)
detect_stray_source_files("${contracts_sources}" "contracts/")

set(libsolutil_sources
    libsolutil/Checksum.cpp
    libsolutil/CommonData.cpp
    libsolutil/IndentedWriter.cpp
    libsolutil/IpfsHash.cpp
    libsolutil/IterateReplacing.cpp
    libsolutil/JSON.cpp
    libsolutil/Keccak256.cpp
    libsolutil/StringUtils.cpp
    libsolutil/SwarmHash.cpp
    libsolutil/UTF8.cpp
    libsolutil/Whiskers.cpp
)
detect_stray_source_files("${libsolutil_sources}" "libsolutil/")

set(libevmasm_sources
    libevmasm/Assembler.cpp
    libevmasm/Optimiser.cpp
)
detect_stray_source_files("${libevmasm_sources}" "libevmasm/")

set(liblangutil_sources
    liblangutil/CharStream.cpp
    liblangutil/SourceLocation.cpp
)
detect_stray_source_files("${liblangutil_sources}" "liblangutil/")

set(libsolidity_sources
    libsolidity/ABIDecoderTests.cpp
    libsolidity/ABIEncoderTests.cpp
    libsolidity/ABIJsonTest.cpp
    libsolidity/ABIJsonTest.h
    libsolidity/ABITestsCommon.h
    libsolidity/AnalysisFramework.cpp
    libsolidity/AnalysisFramework.h
    libsolidity/Assembly.cpp
    libsolidity/ASTJSONTest.cpp
    libsolidity/ASTJSONTest.h
    libsolidity/ErrorCheck.cpp
    libsolidity/ErrorCheck.h
    libsolidity/GasCosts.cpp
    libsolidity/GasMeter.cpp
    libsolidity/GasTest.cpp
    libsolidity/GasTest.h
    libsolidity/Imports.cpp
    libsolidity/InlineAssembly.cpp
    libsolidity/LibSolc.cpp
    libsolidity/Metadata.cpp
    libsolidity/SemanticTest.cpp
    libsolidity/SemanticTest.h
    libsolidity/SemVerMatcher.cpp
    libsolidity/SMTChecker.cpp
    libsolidity/SMTCheckerJSONTest.cpp
    libsolidity/SMTCheckerJSONTest.h
    libsolidity/SMTCheckerTest.cpp
    libsolidity/SMTCheckerTest.h
    libsolidity/SolidityCompiler.cpp
    libsolidity/SolidityEndToEndTest.cpp
    libsolidity/SolidityExecutionFramework.cpp
    libsolidity/SolidityExecutionFramework.h
    libsolidity/SolidityExpressionCompiler.cpp
    libsolidity/SolidityNameAndTypeResolution.cpp
    libsolidity/SolidityNatspecJSON.cpp
    libsolidity/SolidityOptimizer.cpp
    libsolidity/SolidityParser.cpp
    libsolidity/SolidityScanner.cpp
    libsolidity/SolidityTypes.cpp
    libsolidity/StandardCompiler.cpp
    libsolidity/SyntaxTest.cpp
    libsolidity/SyntaxTest.h
    libsolidity/ViewPureChecker.cpp
)
detect_stray_source_files("${libsolidity_sources}" "libsolidity/")

set(libsolidity_util_sources
    libsolidity/util/BytesUtils.cpp
    libsolidity/util/BytesUtils.h
    libsolidity/util/ContractABIUtils.cpp
    libsolidity/util/ContractABIUtils.h
    libsolidity/util/SoltestErrors.h
    libsolidity/util/SoltestTypes.h
    libsolidity/util/TestFileParser.cpp
    libsolidity/util/TestFileParser.h
    libsolidity/util/TestFileParserTests.cpp
    libsolidity/util/TestFunctionCall.cpp
    libsolidity/util/TestFunctionCall.h
    libsolidity/util/TestFunctionCallTests.cpp
)
detect_stray_source_files("${libsolidity_util_sources}" "libsolidity/util/")

set(libyul_sources
    libyul/Common.cpp
    libyul/Common.h
    libyul/CompilabilityChecker.cpp
    libyul/EwasmTranslationTest.cpp
    libyul/EwasmTranslationTest.h
    libyul/FunctionSideEffects.cpp
    libyul/FunctionSideEffects.h
    libyul/Inliner.cpp
    libyul/Metrics.cpp
    libyul/ObjectCompilerTest.cpp
    libyul/ObjectCompilerTest.h
    libyul/ObjectParser.cpp
    libyul/Parser.cpp
    libyul/StackReuseCodegen.cpp
    libyul/YulInterpreterTest.cpp
    libyul/YulInterpreterTest.h
    libyul/YulOptimizerTest.cpp
    libyul/YulOptimizerTest.h
)
detect_stray_source_files("${libyul_sources}" "libyul/")

add_executable(soltest ${sources}
    ${contracts_sources}
    ${libsolutil_sources}
    ${liblangutil_sources}
    ${libevmasm_sources}
    ${libyul_sources}
    ${libsolidity_sources}
    ${libsolidity_util_sources}
)
target_link_libraries(soltest PRIVATE libsolc yul solidity yulInterpreter evmasm solutil Boost::boost Boost::program_options Boost::unit_test_framework evmc)


# Special compilation flag for Visual Studio (version 2019 at least affected)
# in order to compile SolidityEndToEndTest.cpp, which is quite huge.
# We can remove this flag once we've extracted the tests.
# TODO: Remove this option as soon as we have reduced the SLoC's in SolidityEndToEndTest.cpp
if (MSVC)
    target_compile_options(soltest PUBLIC "/bigobj")
endif()

if (NOT Boost_USE_STATIC_LIBS)
    target_compile_definitions(soltest PUBLIC -DBOOST_TEST_DYN_LINK)
endif()

add_subdirectory(tools)
add_subdirectory(evmc)
