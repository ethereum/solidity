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
/**
 * Base class to perform data flow analysis during AST walks.
 * Tracks assignments and is used as base class for both Rematerialiser and
 * Common Subexpression Eliminator.
 */

#pragma once

#include <libyul/optimiser/ASTWalker.h>
#include <libyul/optimiser/KnowledgeBase.h>
#include <libyul/YulName.h>
#include <libyul/AST.h> // Needed for m_zero below.
#include <libyul/SideEffects.h>

#include <libsolutil/Numeric.h>
#include <libsolutil/Common.h>

#include <map>
#include <set>

namespace solidity::yul
{
struct Dialect;
struct SideEffects;
class KnowledgeBase;

/// Value assigned to a variable.
struct AssignedValue
{
	Expression const* value{nullptr};
	/// Loop nesting depth of the definition of the variable.
	size_t loopDepth{0};
};

/**
 * Base class to perform data flow analysis during AST walks.
 * Tracks assignments and is used as base class for both Rematerialiser and
 * Common Subexpression Eliminator.
 *
 * A special zero constant expression is used for the default value of variables.
 *
 * The class also tracks contents in storage and memory. Both keys and values
 * are names of variables. Whenever such a variable is re-assigned, the knowledge
 * is cleared.
 *
 * For elementary statements, we check if it is an SSTORE(x, y) / MSTORE(x, y)
 * If yes, visit the statement. Then record that fact and clear all storage slots t
 *   where we cannot prove x != t or y == m_storage[t] using the current values of the variables x and t.
 * Otherwise, determine if the statement invalidates storage/memory. If yes, clear all knowledge
 * about storage/memory before visiting the statement. Then visit the statement.
 *
 * For forward-joining control flow, storage/memory information from the branches is combined.
 * If the keys or values are different or non-existent in one branch, the key is deleted.
 * This works also for memory (where addresses overlap) because one branch is always an
 * older version of the other and thus overlapping contents would have been deleted already
 * at the point of assignment.
 *
 * The DataFlowAnalyzer currently does not deal with the ``leave`` statement. This is because
 * it only matters at the end of a function body, which is a point in the code a derived class
 * can not easily deal with.
 *
 * Prerequisite: Disambiguator, ForLoopInitRewriter.
 */
class DataFlowAnalyzer: public ASTModifier
{
public:
	enum class MemoryAndStorage { Analyze, Ignore };
	/// @param _functionSideEffects
	///            Side-effects of user-defined functions. Worst-case side-effects are assumed
	///            if this is not provided or the function is not found.
	///            The parameter is mostly used to determine movability of expressions.
	explicit DataFlowAnalyzer(
		Dialect const& _dialect,
		MemoryAndStorage _analyzeStores,
		std::map<YulName, SideEffects> _functionSideEffects = {}
	);

	using ASTModifier::operator();
	void operator()(ExpressionStatement& _statement) override;
	void operator()(Assignment& _assignment) override;
	void operator()(VariableDeclaration& _varDecl) override;
	void operator()(If& _if) override;
	void operator()(Switch& _switch) override;
	void operator()(FunctionDefinition&) override;
	void operator()(ForLoop&) override;
	void operator()(Block& _block) override;

	/// @returns the current value of the given variable, if known - always movable.
	AssignedValue const* variableValue(YulName _variable) const { return util::valueOrNullptr(m_state.value, _variable); }
	std::set<YulName> const* references(YulName _variable) const { return util::valueOrNullptr(m_state.references, _variable); }
	std::map<YulName, AssignedValue> const& allValues() const { return m_state.value; }
	std::optional<YulName> storageValue(YulName _key) const;
	std::optional<YulName> memoryValue(YulName _key) const;
	std::optional<YulName> keccakValue(YulName _start, YulName _length) const;

protected:
	/// Registers the assignment.
	void handleAssignment(std::set<YulName> const& _names, Expression* _value, bool _isDeclaration);

	/// Creates a new inner scope.
	void pushScope(bool _functionScope);

	/// Removes the innermost scope and clears all variables in it.
	void popScope();

	/// Clears information about the values assigned to the given variables,
	/// for example at points where control flow is merged.
	void clearValues(std::set<YulName> _names);

	virtual void assignValue(YulName _variable, Expression const* _value);

	/// Clears knowledge about storage or memory if they may be modified inside the block.
	void clearKnowledgeIfInvalidated(Block const& _block);

	/// Clears knowledge about storage or memory if they may be modified inside the expression.
	void clearKnowledgeIfInvalidated(Expression const& _expression);

	/// Returns true iff the variable is in scope.
	bool inScope(YulName _variableName) const;

	/// Returns the literal value of the identifier, if it exists.
	std::optional<u256> valueOfIdentifier(YulName const& _name) const;

	enum class StoreLoadLocation {
		Memory = 0,
		Storage = 1,
		Last = Storage
	};

	/// Checks if the statement is sstore(a, b) / mstore(a, b)
	/// where a and b are variables and returns these variables in that case.
	std::optional<std::pair<YulName, YulName>> isSimpleStore(
		StoreLoadLocation _location,
		ExpressionStatement const& _statement
	) const;

	/// Checks if the expression is sload(a) / mload(a)
	/// where a is a variable and returns the variable in that case.
	std::optional<YulName> isSimpleLoad(
		StoreLoadLocation _location,
		Expression const& _expression
	) const;

	/// Checks if the expression is keccak256(s, l)
	/// where s and l are variables and returns these variables in that case.
	std::optional<std::pair<YulName, YulName>> isKeccak(Expression const& _expression) const;

	Dialect const& m_dialect;
	/// Side-effects of user-defined functions. Worst-case side-effects are assumed
	/// if this is not provided or the function is not found.
	std::map<YulName, SideEffects> m_functionSideEffects;

private:
	struct Environment
	{
		std::unordered_map<YulName, YulName> storage;
		std::unordered_map<YulName, YulName> memory;
		/// If keccak[s, l] = y then y := keccak256(s, l) occurs in the code.
		std::map<std::pair<YulName, YulName>, YulName> keccak;
	};
	struct State
	{
		/// Current values of variables, always movable.
		std::map<YulName, AssignedValue> value;
		/// m_references[a].contains(b) <=> the current expression assigned to a references b
		std::unordered_map<YulName, std::set<YulName>> references;

		Environment environment;
	};

	/// Joins knowledge about storage and memory with an older point in the control-flow.
	/// This only works if the current state is a direct successor of the older point,
	/// i.e. `_olderState.storage` and `_olderState.memory` cannot have additional changes.
	/// Does nothing if memory and storage analysis is disabled / ignored.
	void joinKnowledge(Environment const& _olderEnvironment);

	static void joinKnowledgeHelper(
		std::unordered_map<YulName, YulName>& _thisData,
		std::unordered_map<YulName, YulName> const& _olderData
	);

	State m_state;

protected:
	KnowledgeBase m_knowledgeBase;

	/// If true, analyzes memory and storage content via mload/mstore and sload/sstore.
	bool m_analyzeStores = true;
	YulName m_storeFunctionName[static_cast<unsigned>(StoreLoadLocation::Last) + 1];
	YulName m_loadFunctionName[static_cast<unsigned>(StoreLoadLocation::Last) + 1];

	/// Current nesting depth of loops.
	size_t m_loopDepth{0};

	struct Scope
	{
		explicit Scope(bool _isFunction): isFunction(_isFunction) {}
		std::set<YulName> variables;
		bool isFunction;
	};
	/// Special expression whose address will be used in m_value.
	/// YulName does not need to be reset because DataFlowAnalyzer is short-lived.
	Expression const m_zero{Literal{{}, LiteralKind::Number, LiteralValue{0, std::nullopt}}};
	/// List of scopes.
	std::vector<Scope> m_variableScopes;
};

}
