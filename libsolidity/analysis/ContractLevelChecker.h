/*
	This file is part of solidity.

	solidity is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	solidity is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with solidity.  If not, see <http://www.gnu.org/licenses/>.
*/
/**
 * Component that verifies overloads, abstract contracts, function clashes and others
 * checks at contract or function level.
 */

#pragma once

#include <libsolidity/ast/ASTForward.h>
#include <map>
#include <set>

namespace langutil
{
class ErrorReporter;
}

namespace dev
{
namespace solidity
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
		m_errorReporter(_errorReporter)
	{}

	/// Performs checks on the given contract.
	/// @returns true iff all checks passed. Note even if all checks passed, errors() can still contain warnings
	bool check(ContractDefinition const& _contract);

private:
	struct LessFunction
	{
		bool operator()(ModifierDefinition const* _a, ModifierDefinition const* _b) const;
		bool operator()(FunctionDefinition const* _a, FunctionDefinition const* _b) const;
		bool operator()(ContractDefinition const* _a, ContractDefinition const* _b) const;
	};

	using FunctionMultiSet = std::multiset<FunctionDefinition const*, LessFunction>;
	using ModifierMultiSet = std::multiset<ModifierDefinition const*, LessFunction>;

	/// Checks that two functions defined in this contract with the same name have different
	/// arguments and that there is at most one constructor.
	void checkDuplicateFunctions(ContractDefinition const& _contract);
	void checkDuplicateEvents(ContractDefinition const& _contract);
	template <class T>
	void findDuplicateDefinitions(std::map<std::string, std::vector<T>> const& _definitions, std::string _message);
	void checkIllegalOverrides(ContractDefinition const& _contract);
	/// Returns false and reports a type error with an appropriate
	/// message if overridden function signature differs.
	/// Also stores the direct super function in the AST annotations.
	bool checkFunctionOverride(FunctionDefinition const& _function, FunctionDefinition const& _super);
	void overrideListError(FunctionDefinition const& function, std::set<ContractDefinition const*, LessFunction> _secondary, std::string const& _message1, std::string const& _message2);
	void overrideError(CallableDeclaration const& function, CallableDeclaration const& super, std::string message, std::string secondaryMsg = "Overridden function is here:");
	void checkAbstractFunctions(ContractDefinition const& _contract);
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
	/// Checks for functions in different base contracts which conflict with each
	/// other and thus need to be overridden explicitly.
	void checkAmbiguousOverrides(ContractDefinition const& _contract) const;
	/// Resolves an override list of UserDefinedTypeNames to a list of contracts.
	std::set<ContractDefinition const*, LessFunction> resolveOverrideList(OverrideSpecifier const& _overrides) const;

	void checkModifierOverrides(FunctionMultiSet const& _funcSet, ModifierMultiSet const& _modSet, std::vector<ModifierDefinition const*> _modifiers);
	void checkOverrideList(FunctionMultiSet const& _funcSet, FunctionDefinition const& _function);

	/// Returns all functions of bases that have not yet been overwritten.
	/// May contain the same function multiple times when used with shared bases.
	FunctionMultiSet const& inheritedFunctions(ContractDefinition const* _contract) const;
	ModifierMultiSet const& inheritedModifiers(ContractDefinition const* _contract) const;

	/// Warns if the contract has a payable fallback, but no receive ether function.
	void checkPayableFallbackWithoutReceive(ContractDefinition const& _contract);

	langutil::ErrorReporter& m_errorReporter;

	/// Cache for inheritedFunctions().
	std::map<ContractDefinition const*, FunctionMultiSet> mutable m_inheritedFunctions;
	std::map<ContractDefinition const*, ModifierMultiSet> mutable m_contractBaseModifiers;
};

}
}
