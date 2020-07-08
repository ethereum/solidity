// SPDX-License-Identifier: GPL-3.0
/**
 * Component that verifies overloads, abstract contracts, function clashes and others
 * checks at contract or function level.
 */

#pragma once

#include <libsolidity/ast/ASTForward.h>
#include <libsolidity/analysis/OverrideChecker.h>
#include <liblangutil/SourceLocation.h>
#include <map>
#include <functional>
#include <set>

namespace solidity::langutil
{
class ErrorReporter;
}

namespace solidity::frontend
{

/**
 * Component that verifies overloads, abstract contracts, function clashes and others
 * checks at contract or function level.
 */
class ContractLevelChecker
{
public:

	/// @param _errorReporter provides the error logging functionality.
	explicit ContractLevelChecker(langutil::ErrorReporter& _errorReporter):
		m_overrideChecker{_errorReporter},
		m_errorReporter(_errorReporter)
	{}

	/// Performs checks on the given contract.
	/// @returns true iff all checks passed. Note even if all checks passed, errors() can still contain warnings
	bool check(ContractDefinition const& _contract);

private:
	/// Checks that two functions defined in this contract with the same name have different
	/// arguments and that there is at most one constructor.
	void checkDuplicateFunctions(ContractDefinition const& _contract);
	void checkDuplicateEvents(ContractDefinition const& _contract);
	template <class T>
	void findDuplicateDefinitions(std::map<std::string, std::vector<T>> const& _definitions);
	/// Checks for unimplemented functions and modifiers.
	void checkAbstractDefinitions(ContractDefinition const& _contract);
	/// Checks that the base constructor arguments are properly provided.
	/// Fills the list of unimplemented functions in _contract's annotations.
	void checkBaseConstructorArguments(ContractDefinition const& _contract);
	void annotateBaseConstructorArguments(
		ContractDefinition const& _currentContract,
		FunctionDefinition const* _baseConstructor,
		ASTNode const* _argumentNode
	);
	/// Checks that different functions with external visibility end up having different
	/// external argument types (i.e. different signature).
	void checkExternalTypeClashes(ContractDefinition const& _contract);
	/// Checks for hash collisions in external function signatures.
	void checkHashCollisions(ContractDefinition const& _contract);
	/// Checks that all requirements for a library are fulfilled if this is a library.
	void checkLibraryRequirements(ContractDefinition const& _contract);
	/// Checks base contracts for ABI compatibility
	void checkBaseABICompatibility(ContractDefinition const& _contract);

	/// Warns if the contract has a payable fallback, but no receive ether function.
	void checkPayableFallbackWithoutReceive(ContractDefinition const& _contract);

	OverrideChecker m_overrideChecker;
	langutil::ErrorReporter& m_errorReporter;
};

}
