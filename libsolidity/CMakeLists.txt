# Until we have a clear separation, libyul has to be included here
set(sources
	analysis/ConstantEvaluator.cpp
	analysis/ControlFlowAnalyzer.cpp
	analysis/ControlFlowBuilder.cpp
	analysis/ControlFlowGraph.cpp
	analysis/DeclarationContainer.cpp
	analysis/DocStringAnalyser.cpp
	analysis/GlobalContext.cpp
	analysis/NameAndTypeResolver.cpp
	analysis/PostTypeChecker.cpp
	analysis/ReferencesResolver.cpp
	analysis/SemVerHandler.cpp
	analysis/StaticAnalyzer.cpp
	analysis/SyntaxChecker.cpp
	analysis/TypeChecker.cpp
	analysis/ViewPureChecker.cpp
	ast/AST.cpp
	ast/ASTAnnotations.cpp
	ast/ASTJsonConverter.cpp
	ast/ASTPrinter.cpp
	ast/Types.cpp
	codegen/ABIFunctions.cpp
	codegen/ArrayUtils.cpp
	codegen/Compiler.cpp
	codegen/CompilerContext.cpp
	codegen/CompilerUtils.cpp
	codegen/ContractCompiler.cpp
	codegen/ExpressionCompiler.cpp
	codegen/LValue.cpp
	formal/SMTChecker.cpp
	formal/SMTLib2Interface.cpp
	formal/SMTPortfolio.cpp
	formal/SSAVariable.cpp
	formal/SymbolicTypes.cpp
	formal/SymbolicVariables.cpp
	formal/VariableUsage.cpp
	interface/ABI.cpp
	interface/AssemblyStack.cpp
	interface/CompilerStack.cpp
	interface/GasEstimator.cpp
	interface/Natspec.cpp
	interface/StandardCompiler.cpp
	interface/Version.cpp
	parsing/DocStringParser.cpp
	parsing/Parser.cpp
)

find_package(Z3 QUIET)
if (${Z3_FOUND})
  include_directories(${Z3_INCLUDE_DIR})
  add_definitions(-DHAVE_Z3)
  message("Z3 SMT solver found. This enables optional SMT checking with Z3.")
  set(z3_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/formal/Z3Interface.cpp")
else()
  set(z3_SRCS)
endif()

find_package(CVC4 QUIET)
if (${CVC4_FOUND})
  include_directories(${CVC4_INCLUDE_DIR})
  add_definitions(-DHAVE_CVC4)
  message("CVC4 SMT solver found. This enables optional SMT checking with CVC4.")
  set(cvc4_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/formal/CVC4Interface.cpp")
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
  target_link_libraries(solidity PUBLIC ${Z3_LIBRARY})
endif()

if (${CVC4_FOUND})
  target_link_libraries(solidity PUBLIC ${CVC4_LIBRARIES})
endif()
