set(sources
    boostTest.cpp
    Common.cpp
    Common.h
    CommonSyntaxTest.cpp
    CommonSyntaxTest.h
    EVMHost.cpp
    EVMHost.h
    ExecutionFramework.cpp
    ExecutionFramework.h
    InteractiveTests.h
    Metadata.cpp
    Metadata.h
    TestCase.cpp
    TestCase.h
    TestCaseReader.cpp
    TestCaseReader.h
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
    libsolutil/FixedHash.cpp
    libsolutil/IndentedWriter.cpp
    libsolutil/IpfsHash.cpp
    libsolutil/IterateReplacing.cpp
    libsolutil/JSON.cpp
    libsolutil/Keccak256.cpp
    libsolutil/LazyInit.cpp
    libsolutil/LEB128.cpp
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
    liblangutil/Scanner.cpp
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
    libsolidity/SolidityTypes.cpp
    libsolidity/StandardCompiler.cpp
    libsolidity/SyntaxTest.cpp
    libsolidity/SyntaxTest.h
    libsolidity/ViewPureChecker.cpp
    libsolidity/analysis/FunctionCallGraph.cpp
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
    libyul/SyntaxTest.h
    libyul/SyntaxTest.cpp
    libyul/YulInterpreterTest.cpp
    libyul/YulInterpreterTest.h
    libyul/YulOptimizerTest.cpp
    libyul/YulOptimizerTest.h
    libyul/YulOptimizerTestCommon.cpp
    libyul/YulOptimizerTestCommon.h
)
detect_stray_source_files("${libyul_sources}" "libyul/")

set(yul_phaser_sources
    yulPhaser/TestHelpers.h
    yulPhaser/TestHelpers.cpp
    yulPhaser/TestHelpersTest.cpp
    yulPhaser/Common.cpp
    yulPhaser/Chromosome.cpp
    yulPhaser/FitnessMetrics.cpp
    yulPhaser/AlgorithmRunner.cpp
    yulPhaser/GeneticAlgorithms.cpp
    yulPhaser/Mutations.cpp
    yulPhaser/PairSelections.cpp
    yulPhaser/Phaser.cpp
    yulPhaser/Population.cpp
    yulPhaser/Program.cpp
    yulPhaser/ProgramCache.cpp
    yulPhaser/Selections.cpp
    yulPhaser/SimulationRNG.cpp

    # FIXME: yul-phaser is not a library so I can't just add it to target_link_libraries().
    # My current workaround is just to include its source files here but this introduces
    # unnecessary duplication. Create a library or find a way to reuse the list in both places.
    ../tools/yulPhaser/AlgorithmRunner.cpp
    ../tools/yulPhaser/Common.cpp
    ../tools/yulPhaser/Chromosome.cpp
    ../tools/yulPhaser/FitnessMetrics.cpp
    ../tools/yulPhaser/GeneticAlgorithms.cpp
    ../tools/yulPhaser/Mutations.cpp
    ../tools/yulPhaser/PairSelections.cpp
    ../tools/yulPhaser/Phaser.cpp
    ../tools/yulPhaser/Population.cpp
    ../tools/yulPhaser/Program.cpp
    ../tools/yulPhaser/ProgramCache.cpp
    ../tools/yulPhaser/Selections.cpp
    ../tools/yulPhaser/SimulationRNG.cpp
)
detect_stray_source_files("${yul_phaser_sources}" "yulPhaser/")

add_executable(soltest ${sources}
    ${contracts_sources}
    ${libsolutil_sources}
    ${liblangutil_sources}
    ${libevmasm_sources}
    ${libyul_sources}
    ${libsolidity_sources}
    ${libsolidity_util_sources}
    ${yul_phaser_sources}
)
target_link_libraries(soltest PRIVATE libsolc yul solidity smtutil solutil Boost::boost yulInterpreter evmasm Boost::filesystem Boost::program_options Boost::unit_test_framework evmc)


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
