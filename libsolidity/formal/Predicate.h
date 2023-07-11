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
// SPDX-License-Identifier: GPL-3.0

#pragma once

#include <libsolidity/formal/SymbolicVariables.h>
#include <libsolidity/formal/SymbolicVariables.h>

#include <libsolidity/ast/AST.h>

#include <libsmtutil/Sorts.h>

#include <map>
#include <optional>
#include <vector>

namespace solidity::langutil
{
class CharStreamProvider;
}

namespace solidity::frontend
{

enum class PredicateType
{
	Interface,
	NondetInterface,
	ConstructorSummary,
	FunctionSummary,
	FunctionBlock,
	FunctionErrorBlock,
	InternalCall,
	ExternalCallTrusted,
	ExternalCallUntrusted,
	Error,
	Custom
};

/**
 * Represents a predicate used by the CHC engine.
 */
class Predicate
{
public:
	static Predicate const* create(
		smtutil::SortPointer _sort,
		std::string _name,
		PredicateType _type,
		smt::EncodingContext& _context,
		ASTNode const* _node = nullptr,
		ContractDefinition const* _contractContext = nullptr,
		std::vector<ScopeOpener const*> _scopeStack = {}
	);

	Predicate(
		smt::SymbolicFunctionVariable&& _predicate,
		PredicateType _type,
		ASTNode const* _node = nullptr,
		ContractDefinition const* _contractContext = nullptr,
		std::vector<ScopeOpener const*> _scopeStack = {}
	);

	/// Predicate should not be copiable.
	Predicate(Predicate const&) = delete;
	Predicate& operator=(Predicate const&) = delete;

	/// @returns the Predicate associated with _name.
	static Predicate const* predicate(std::string const& _name);

	/// Resets all the allocated predicates.
	static void reset();

	/// @returns a function application of the predicate over _args.
	smtutil::Expression operator()(std::vector<smtutil::Expression> const& _args) const;

	/// @returns the function declaration of the predicate.
	smtutil::Expression functor() const;
	/// @returns the function declaration of the predicate with index _idx.
	smtutil::Expression functor(unsigned _idx) const;
	/// Increases the index of the function declaration of the predicate.
	void newFunctor();

	/// @returns the program node this predicate represents.
	ASTNode const* programNode() const;

	/// @returns the ContractDefinition of the most derived contract
	/// being analyzed.
	ContractDefinition const* contextContract() const;

	/// @returns the ContractDefinition that this predicate represents
	/// or nullptr otherwise.
	ContractDefinition const* programContract() const;

	/// @returns the FunctionDefinition that this predicate represents
	/// or nullptr otherwise.
	FunctionDefinition const* programFunction() const;

	/// @returns the FunctionCall that this predicate represents
	/// or nullptr otherwise.
	FunctionCall const* programFunctionCall() const;

	/// @returns the VariableDeclaration that this predicate represents
	/// or nullptr otherwise.
	VariableDeclaration const* programVariable() const;

	/// @returns the program state variables in the scope of this predicate.
	std::optional<std::vector<VariableDeclaration const*>> stateVariables() const;

	/// @returns true if this predicate represents a summary.
	bool isSummary() const;

	/// @returns true if this predicate represents a function summary.
	bool isFunctionSummary() const;

	/// @returns true if this predicate represents a function block.
	bool isFunctionBlock() const;

	/// @returns true if this predicate represents a function error block.
	bool isFunctionErrorBlock() const;

	/// @returns true if this predicate represents an internal function call.
	bool isInternalCall() const;

	/// @returns true if this predicate represents a trusted external function call.
	bool isExternalCallTrusted() const;

	/// @returns true if this predicate represents an untrusted external function call.
	bool isExternalCallUntrusted() const;

	/// @returns true if this predicate represents a constructor summary.
	bool isConstructorSummary() const;

	/// @returns true if this predicate represents an interface.
	bool isInterface() const;

	/// @returns true if this predicate represents a nondeterministic interface.
	bool isNondetInterface() const;

	PredicateType type() const { return m_type; }

	/// @returns a formatted string representing a call to this predicate
	/// with _args.
	std::string formatSummaryCall(
		std::vector<smtutil::Expression> const& _args,
		langutil::CharStreamProvider const& _charStreamProvider,
		bool _appendTxVars = false
	) const;

	/// @returns the values of the state variables from _args at the point
	/// where this summary was reached.
	std::vector<std::optional<std::string>> summaryStateValues(std::vector<smtutil::Expression> const& _args) const;

	/// @returns the values of the function input variables from _args at the point
	/// where this summary was reached.
	std::vector<std::optional<std::string>> summaryPostInputValues(std::vector<smtutil::Expression> const& _args) const;

	/// @returns the values of the function output variables from _args at the point
	/// where this summary was reached.
	std::vector<std::optional<std::string>> summaryPostOutputValues(std::vector<smtutil::Expression> const& _args) const;

	/// @returns the values of the local variables used by this predicate.
	std::pair<std::vector<std::optional<std::string>>, std::vector<VariableDeclaration const*>> localVariableValues(std::vector<smtutil::Expression> const& _args) const;

	/// @returns a substitution map from the arguments of _predExpr
	/// to a Solidity-like expression.
	std::map<std::string, std::string> expressionSubstitution(smtutil::Expression const& _predExpr) const;

private:
	/// @returns the formatted version of the given SMT expressions. Those expressions must be SMT constants.
	std::vector<std::optional<std::string>> formatExpressions(std::vector<smtutil::Expression> const& _exprs, std::vector<Type const*> const& _types) const;

	/// @returns a string representation of the SMT expression based on a Solidity type.
	std::optional<std::string> expressionToString(smtutil::Expression const& _expr, Type const* _type) const;

	/// Recursively fills _array from _expr.
	/// _expr should have the form `store(store(...(const_array(x_0), i_0, e_0), i_m, e_m), i_k, e_k)`.
	/// @returns true if the construction worked,
	/// and false if at least one element could not be built.
	bool fillArray(smtutil::Expression const& _expr, std::vector<std::string>& _array, ArrayType const& _type) const;

	std::map<std::string, std::optional<std::string>> readTxVars(smtutil::Expression const& _tx) const;

	/// The actual SMT expression.
	smt::SymbolicFunctionVariable m_predicate;

	/// The type of this predicate.
	PredicateType m_type;

	/// The ASTNode that this predicate represents.
	/// nullptr if this predicate is not associated with a specific program AST node.
	ASTNode const* m_node = nullptr;

	/// The ContractDefinition that contains this predicate.
	/// nullptr if this predicate is not associated with a specific contract.
	/// This is unfortunately necessary because of virtual resolution for
	/// function nodes.
	ContractDefinition const* m_contractContext = nullptr;

	/// Maps the name of the predicate to the actual Predicate.
	/// Used in counterexample generation.
	static std::map<std::string, Predicate> m_predicates;

	/// The scope stack when the predicate was created.
	/// Used to identify the subset of variables in scope.
	std::vector<ScopeOpener const*> const m_scopeStack;
};

struct PredicateCompare
{
	bool operator()(Predicate const* lhs, Predicate const* rhs) const
	{
		// We cannot use m_node->id() because different predicates may
		// represent the same program node.
		// We use the symbolic name since it is unique per predicate and
		// the order does not really matter.
		return lhs->functor().name < rhs->functor().name;
	}
};

}
