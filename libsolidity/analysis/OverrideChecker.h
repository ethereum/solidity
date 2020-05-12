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
#include <libsolidity/ast/ASTEnums.h>
#include <liblangutil/SourceLocation.h>
#include <map>
#include <functional>
#include <set>
#include <variant>
#include <optional>

namespace solidity::langutil
{
class ErrorReporter;
struct ErrorId;
struct SourceLocation;
}

namespace solidity::frontend
{
class FunctionType;
class ModifierType;

/**
 * Class that represents a function, public state variable or modifier
 * and helps with overload checking.
 * Regular comparison is performed based on AST node, while CompareBySignature
 * results in two elements being equal when they can override each
 * other.
 */
class OverrideProxy
{
public:
	OverrideProxy() {}
	explicit OverrideProxy(FunctionDefinition const* _fun): m_item{_fun} {}
	explicit OverrideProxy(ModifierDefinition const* _mod): m_item{_mod} {}
	explicit OverrideProxy(VariableDeclaration const* _var): m_item{_var} {}

	bool operator<(OverrideProxy const& _other) const;

	struct CompareBySignature
	{
		bool operator()(OverrideProxy const& _a, OverrideProxy const& _b) const;
	};

	bool isVariable() const;
	bool isFunction() const;
	bool isModifier() const;

	size_t id() const;
	std::shared_ptr<OverrideSpecifier> overrides() const;
	std::set<OverrideProxy> baseFunctions() const;
	/// This stores the item in the list of base items.
	void storeBaseFunction(OverrideProxy const& _base) const;

	std::string const& name() const;
	ContractDefinition const& contract() const;
	std::string const& contractName() const;
	Visibility visibility() const;
	StateMutability stateMutability() const;
	bool virtualSemantics() const;

	/// @returns receive / fallback / function (only the latter for modifiers and variables);
	langutil::Token functionKind() const;

	FunctionType const* functionType() const;
	ModifierType const* modifierType() const;

	Declaration const* declaration() const;

	langutil::SourceLocation const& location() const;

	std::string astNodeName() const;
	std::string astNodeNameCapitalized() const;
	std::string distinguishingProperty() const;

	/// @returns true if this AST elements supports the feature of being unimplemented
	/// and is actually not implemented.
	bool unimplemented() const;

	/**
	 * Struct to help comparing override items about whether they override each other.
	 * Does not produce a total order.
	 */
	struct OverrideComparator
	{
		std::string name;
		std::optional<langutil::Token> functionKind;
		std::optional<std::vector<std::string>> parameterTypes;

		bool operator<(OverrideComparator const& _other) const;
	};

	/// @returns a structure used to compare override items with regards to whether
	/// they override each other.
	OverrideComparator const& overrideComparator() const;

private:
	std::variant<
		FunctionDefinition const*,
		ModifierDefinition const*,
		VariableDeclaration const*
	> m_item;

	std::shared_ptr<OverrideComparator> mutable m_comparator;
};


/**
 * Component that verifies override properties.
 */
class OverrideChecker
{
public:
	using OverrideProxyBySignatureMultiSet = std::multiset<OverrideProxy, OverrideProxy::CompareBySignature>;

	/// @param _errorReporter provides the error logging functionality.
	explicit OverrideChecker(langutil::ErrorReporter& _errorReporter):
		m_errorReporter(_errorReporter)
	{}

	void check(ContractDefinition const& _contract);

	struct CompareByID
	{
		bool operator()(ContractDefinition const* _a, ContractDefinition const* _b) const;
	};

	/// Returns all functions of bases (including public state variables) that have not yet been overwritten.
	/// May contain the same function multiple times when used with shared bases.
	OverrideProxyBySignatureMultiSet const& inheritedFunctions(ContractDefinition const& _contract) const;
	OverrideProxyBySignatureMultiSet const& inheritedModifiers(ContractDefinition const& _contract) const;

private:
	void checkIllegalOverrides(ContractDefinition const& _contract);
	/// Performs various checks related to @a _overriding overriding @a _super like
	/// different return type, invalid visibility change, etc.
	/// Works on functions, modifiers and public state variables.
	/// Also stores @a _super as a base function of @a _function in its AST annotations.
	void checkOverride(OverrideProxy const& _overriding, OverrideProxy const& _super);
	void overrideListError(
		OverrideProxy const& _item,
		std::set<ContractDefinition const*, CompareByID> _secondary,
		langutil::ErrorId _error,
		std::string const& _message1,
		std::string const& _message2
	);
	void overrideError(
		Declaration const& _overriding,
		Declaration const& _super,
		langutil::ErrorId _error,
		std::string const& _message,
		std::string const& _secondaryMsg = "Overridden function is here:"
	);
	void overrideError(
		OverrideProxy const& _overriding,
		OverrideProxy const& _super,
		langutil::ErrorId _error,
		std::string const& _message,
		std::string const& _secondaryMsg = "Overridden function is here:"
	);
	/// Checks for functions in different base contracts which conflict with each
	/// other and thus need to be overridden explicitly.
	void checkAmbiguousOverrides(ContractDefinition const& _contract) const;
	void checkAmbiguousOverridesInternal(std::set<OverrideProxy> _baseCallables, langutil::SourceLocation const& _location) const;
	/// Resolves an override list of UserDefinedTypeNames to a list of contracts.
	std::set<ContractDefinition const*, CompareByID> resolveOverrideList(OverrideSpecifier const& _overrides) const;

	void checkOverrideList(OverrideProxy _item, OverrideProxyBySignatureMultiSet const& _inherited);

	langutil::ErrorReporter& m_errorReporter;

	/// Cache for inheritedFunctions().
	std::map<ContractDefinition const*, OverrideProxyBySignatureMultiSet> mutable m_inheritedFunctions;
	std::map<ContractDefinition const*, OverrideProxyBySignatureMultiSet> mutable m_inheritedModifiers;
};

}
