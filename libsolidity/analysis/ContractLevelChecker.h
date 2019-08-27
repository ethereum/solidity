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
#include <list>

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
	/// Helper struct to check overrides
	struct FunctionInfo
	{
		FunctionDefinition const* funcDef;
		ContractDefinition const* contract;
	};

	/// @param _errorReporter provides the error logging functionality.
	explicit ContractLevelChecker(langutil::ErrorReporter& _errorReporter):
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
	void findDuplicateDefinitions(std::map<std::string, std::vector<T>> const& _definitions, std::string _message);
	void checkIllegalOverrides(ContractDefinition const& _contract);
	/// Returns false and reports a type error with an appropriate message if
	/// overridden function signature differs.
	/// Also stores the direct super function in the AST annotations.
	bool checkFunctionOverride(FunctionDefinition const& _function, FunctionDefinition const& _super, bool _isBaseOf);
	void overrideError(FunctionDefinition const& function, FunctionDefinition const& super, std::string message);
	void checkAbstractFunctions(ContractDefinition const& _contract);
	void checkBaseConstructorArguments(ContractDefinition const& _contract);
	void annotateBaseConstructorArguments(
		ContractDefinition const& _currentContract,
		FunctionDefinition const* _baseConstructor,
		ASTNode const* _argumentNode
	);
	void checkConstructor(ContractDefinition const& _contract);
	void checkFallbackFunction(ContractDefinition const& _contract);
	/// Checks that different functions with external visibility end up having different
	/// external argument types (i.e. different signature).
	void checkExternalTypeClashes(ContractDefinition const& _contract);
	/// Checks for hash collisions in external function signatures.
	void checkHashCollisions(ContractDefinition const& _contract);
	/// Checks that all requirements for a library are fulfilled if this is a library.
	void checkLibraryRequirements(ContractDefinition const& _contract);
	/// Checks base contracts for ABI compatibility
	void checkBaseABICompatibility(ContractDefinition const& _contract);

	void checkAmbiguousOverrides(ContractDefinition const& _contract, std::map<std::string, std::list<FunctionInfo>> const& _functions) const;
	void checkOverrideList(std::map<std::string, std::list<FunctionInfo>> const& _functions) const;
	void checkForMissingOverrides(
		std::list<FunctionInfo>::const_iterator _begin,
		std::list<FunctionInfo>::const_iterator const& _end,
		std::vector<ContractDefinition const*> const& _specifiedContracts
	) const;
	void checkForInvalidOverrides(
		std::list<FunctionInfo>::const_iterator _begin,
		std::list<FunctionInfo>::const_iterator const& _end,
		std::vector<ContractDefinition const*> const& _specifiedContracts
	) const;

	std::vector<ContractDefinition const*> resolveOverrideList(OverrideSpecifier const& _overrides) const;

	langutil::ErrorReporter& m_errorReporter;
};

}
}
