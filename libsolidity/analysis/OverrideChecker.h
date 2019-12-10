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
 * Component that verifies override properties.
 */

#pragma once

#include <libsolidity/ast/ASTForward.h>
#include <liblangutil/SourceLocation.h>
#include <map>
#include <functional>
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
 * Component that verifies override properties.
 */
class OverrideChecker
{
public:

	/// @param _errorReporter provides the error logging functionality.
	explicit OverrideChecker(langutil::ErrorReporter& _errorReporter):
		m_errorReporter(_errorReporter)
	{}

	void check(ContractDefinition const& _contract);

private:
	/**
	 * Comparator that compares
	 *  - functions such that equality means that the functions override each other
	 *  - modifiers by name
	 *  - contracts by AST id.
	 */
	struct LessFunction
	{
		bool operator()(ModifierDefinition const* _a, ModifierDefinition const* _b) const;
		bool operator()(FunctionDefinition const* _a, FunctionDefinition const* _b) const;
		bool operator()(ContractDefinition const* _a, ContractDefinition const* _b) const;
	};

	using FunctionMultiSet = std::multiset<FunctionDefinition const*, LessFunction>;
	using ModifierMultiSet = std::multiset<ModifierDefinition const*, LessFunction>;

	void checkIllegalOverrides(ContractDefinition const& _contract);
	/// Performs various checks related to @a _overriding overriding @a _super like
	/// different return type, invalid visibility change, etc.
	/// Works on functions, modifiers and public state variables.
	/// Also stores @a _super as a base function of @a _function in its AST annotations.
	template<class T, class U>
	void checkOverride(T const& _overriding, U const& _super);
	void overrideListError(
		CallableDeclaration const& _callable,
		std::set<ContractDefinition const*, LessFunction> _secondary,
		std::string const& _message1,
		std::string const& _message2
	);
	void overrideError(
		Declaration const& _overriding,
		Declaration const& _super,
		std::string _message,
		std::string _secondaryMsg = "Overridden function is here:"
	);
	/// Checks for functions in different base contracts which conflict with each
	/// other and thus need to be overridden explicitly.
	void checkAmbiguousOverrides(ContractDefinition const& _contract) const;
	void checkAmbiguousOverridesInternal(std::set<
		CallableDeclaration const*,
		std::function<bool(CallableDeclaration const*, CallableDeclaration const*)>
	> _baseCallables, langutil::SourceLocation const& _location) const;
	/// Resolves an override list of UserDefinedTypeNames to a list of contracts.
	std::set<ContractDefinition const*, LessFunction> resolveOverrideList(OverrideSpecifier const& _overrides) const;

	template <class T>
	void checkOverrideList(
		std::multiset<T const*, LessFunction> const& _funcSet,
		T const& _function
	);

	/// Returns all functions of bases that have not yet been overwritten.
	/// May contain the same function multiple times when used with shared bases.
	FunctionMultiSet const& inheritedFunctions(ContractDefinition const& _contract) const;
	ModifierMultiSet const& inheritedModifiers(ContractDefinition const& _contract) const;

	langutil::ErrorReporter& m_errorReporter;

	/// Cache for inheritedFunctions().
	std::map<ContractDefinition const*, FunctionMultiSet> mutable m_inheritedFunctions;
	std::map<ContractDefinition const*, ModifierMultiSet> mutable m_contractBaseModifiers;
};

}
}
