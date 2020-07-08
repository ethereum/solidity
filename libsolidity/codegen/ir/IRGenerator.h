// SPDX-License-Identifier: GPL-3.0
/**
 * @author Alex Beregszaszi
 * @date 2017
 * Component that translates Solidity code into Yul.
 */

#pragma once

#include <libsolidity/interface/OptimiserSettings.h>
#include <libsolidity/ast/ASTForward.h>
#include <libsolidity/codegen/ir/IRGenerationContext.h>
#include <libsolidity/codegen/YulUtilFunctions.h>
#include <liblangutil/EVMVersion.h>
#include <string>

namespace solidity::frontend
{

class SourceUnit;

class IRGenerator
{
public:
	IRGenerator(
		langutil::EVMVersion _evmVersion,
		RevertStrings _revertStrings,
		OptimiserSettings _optimiserSettings
	):
		m_evmVersion(_evmVersion),
		m_optimiserSettings(_optimiserSettings),
		m_context(_evmVersion, _revertStrings, std::move(_optimiserSettings)),
		m_utils(_evmVersion, m_context.revertStrings(), m_context.functionCollector())
	{}

	/// Generates and returns the IR code, in unoptimized and optimized form
	/// (or just pretty-printed, depending on the optimizer settings).
	std::pair<std::string, std::string> run(
		ContractDefinition const& _contract,
		std::map<ContractDefinition const*, std::string const> const& _otherYulSources
	);

private:
	std::string generate(
		ContractDefinition const& _contract,
		std::map<ContractDefinition const*, std::string const> const& _otherYulSources
	);
	std::string generate(Block const& _block);

	/// Generates code for all the functions from the function generation queue.
	/// The resulting code is stored in the function collector in IRGenerationContext.
	void generateQueuedFunctions();
	/// Generates  all the internal dispatch functions necessary to handle any function that could
	/// possibly be called via a pointer.
	/// @return The content of the dispatch for reuse in runtime code. Reuse is necessary because
	/// pointers to functions can be passed from the creation code in storage variables.
	InternalDispatchMap generateInternalDispatchFunctions();
	/// Generates code for and returns the name of the function.
	std::string generateFunction(FunctionDefinition const& _function);
	/// Generates a getter for the given declaration and returns its name
	std::string generateGetter(VariableDeclaration const& _varDecl);

	/// Generates code that assigns the initial value of the respective type.
	std::string generateInitialAssignment(VariableDeclaration const& _varDecl);

	/// Generates implicit constructors for all contracts in the inheritance hierarchy of
	/// @a _contract
	/// If there are user defined constructors, their body will be included in implicit constructors body.
	void generateImplicitConstructors(ContractDefinition const& _contract);

	/// Evaluates constructor's arguments for all base contracts (listed in inheritance specifiers) of
	/// @a _contract
	/// @returns Pair of expressions needed to evaluate params and list of parameters in a map contract -> params
	std::pair<std::string, std::map<ContractDefinition const*, std::vector<std::string>>>
	evaluateConstructorArguments(ContractDefinition const& _contract);

	/// Initializes state variables of
	/// @a _contract
	/// @returns Source code to initialize state variables
	std::string initStateVariables(ContractDefinition const& _contract);

	std::string deployCode(ContractDefinition const& _contract);
	std::string callValueCheck();

	std::string dispatchRoutine(ContractDefinition const& _contract);

	std::string memoryInit();

	void resetContext(ContractDefinition const& _contract);

	langutil::EVMVersion const m_evmVersion;
	OptimiserSettings const m_optimiserSettings;

	IRGenerationContext m_context;
	YulUtilFunctions m_utils;
};

}
