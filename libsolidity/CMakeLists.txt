# Until we have a clear separation, libyul has to be included here
set(sources
	analysis/ConstantEvaluator.cpp
	analysis/ConstantEvaluator.h
	analysis/ContractLevelChecker.cpp
	analysis/ContractLevelChecker.h
	analysis/ControlFlowAnalyzer.cpp
	analysis/ControlFlowAnalyzer.h
	analysis/ControlFlowBuilder.cpp
	analysis/ControlFlowBuilder.h
	analysis/ControlFlowGraph.cpp
	analysis/ControlFlowGraph.h
	analysis/DeclarationContainer.cpp
	analysis/DeclarationContainer.h
	analysis/DocStringAnalyser.cpp
	analysis/DocStringAnalyser.h
	analysis/GlobalContext.cpp
	analysis/GlobalContext.h
	analysis/NameAndTypeResolver.cpp
	analysis/NameAndTypeResolver.h
	analysis/PostTypeChecker.cpp
	analysis/PostTypeChecker.h
	analysis/ReferencesResolver.cpp
	analysis/ReferencesResolver.h
	analysis/StaticAnalyzer.cpp
	analysis/StaticAnalyzer.h
	analysis/SyntaxChecker.cpp
	analysis/SyntaxChecker.h
	analysis/TypeChecker.cpp
	analysis/TypeChecker.h
	analysis/ViewPureChecker.cpp
	analysis/ViewPureChecker.h
	ast/AST.cpp
	ast/AST.h
	ast/AST_accept.h
	ast/ASTAnnotations.cpp
	ast/ASTAnnotations.h
	ast/ASTEnums.h
	ast/ASTForward.h
	ast/ASTJsonConverter.cpp
	ast/ASTJsonConverter.h
	ast/ASTPrinter.cpp
	ast/ASTPrinter.h
	ast/ASTVisitor.h
	ast/ExperimentalFeatures.h
	ast/Types.cpp
	ast/Types.h
	ast/TypeProvider.cpp
	ast/TypeProvider.h
	codegen/ABIFunctions.cpp
	codegen/ABIFunctions.h
	codegen/ArrayUtils.cpp
	codegen/ArrayUtils.h
	codegen/Compiler.cpp
	codegen/Compiler.h
	codegen/CompilerContext.cpp
	codegen/CompilerContext.h
	codegen/CompilerUtils.cpp
	codegen/CompilerUtils.h
	codegen/ContractCompiler.cpp
	codegen/ContractCompiler.h
	codegen/ExpressionCompiler.cpp
	codegen/ExpressionCompiler.h
	codegen/LValue.cpp
	codegen/LValue.h
	codegen/MultiUseYulFunctionCollector.h
	codegen/MultiUseYulFunctionCollector.cpp
	codegen/YulUtilFunctions.h
	codegen/YulUtilFunctions.cpp
	codegen/ir/IRGenerator.cpp
	codegen/ir/IRGenerator.h
	codegen/ir/IRGeneratorForStatements.cpp
	codegen/ir/IRGeneratorForStatements.h
	codegen/ir/IRGenerationContext.cpp
	codegen/ir/IRGenerationContext.h
	formal/EncodingContext.cpp
	formal/EncodingContext.h
	formal/SMTChecker.cpp
	formal/SMTChecker.h
	formal/SMTLib2Interface.cpp
	formal/SMTLib2Interface.h
	formal/SMTPortfolio.cpp
	formal/SMTPortfolio.h
	formal/SolverInterface.h
	formal/SSAVariable.cpp
	formal/SSAVariable.h
	formal/SymbolicTypes.cpp
	formal/SymbolicTypes.h
	formal/SymbolicVariables.cpp
	formal/SymbolicVariables.h
	formal/VariableUsage.cpp
	formal/VariableUsage.h
	interface/ABI.cpp
	interface/ABI.h
	interface/CompilerStack.cpp
	interface/CompilerStack.h
	interface/GasEstimator.cpp
	interface/GasEstimator.h
	interface/Natspec.cpp
	interface/Natspec.h
	interface/OptimiserSettings.h
	interface/ReadFile.h
	interface/StandardCompiler.cpp
	interface/StandardCompiler.h
	interface/Version.cpp
	interface/Version.h
	parsing/DocStringParser.cpp
	parsing/DocStringParser.h
	parsing/Parser.cpp
	parsing/Parser.h
	parsing/Token.h
)

find_package(Z3 4.6.0)
if (${Z3_FOUND})
  add_definitions(-DHAVE_Z3)
  message("Z3 SMT solver found. This enables optional SMT checking with Z3.")
  set(z3_SRCS formal/Z3Interface.cpp formal/Z3Interface.h)
else()
  set(z3_SRCS)
endif()

find_package(CVC4 QUIET)
if (${CVC4_FOUND})
  add_definitions(-DHAVE_CVC4)
  message("CVC4 SMT solver found. This enables optional SMT checking with CVC4.")
  set(cvc4_SRCS formal/CVC4Interface.cpp formal/CVC4Interface.h)
else()
  set(cvc4_SRCS)
endif()

if (NOT (${Z3_FOUND} OR ${CVC4_FOUND}))
  message("No SMT solver found (or it has been forcefully disabled). Optional SMT checking will not be available.\
  \nPlease install Z3 or CVC4 or remove the option disabling them (USE_Z3, USE_CVC4).")
endif()

add_library(solidity ${sources} ${z3_SRCS} ${cvc4_SRCS})
target_link_libraries(solidity PUBLIC yul evmasm langutil devcore ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY})

if (${Z3_FOUND})
  target_link_libraries(solidity PUBLIC Z3::Z3)
endif()

if (${CVC4_FOUND})
  target_link_libraries(solidity PUBLIC CVC4::CVC4)
endif()
