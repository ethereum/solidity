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
 * Specific AST walkers that collect semantical facts.
 */

#pragma once

#include <libyul/optimiser/ASTWalker.h>

#include <set>

namespace yul
{
struct Dialect;

/**
 * Specific AST walker that determines whether an expression is movable.
 */
class MovableChecker: public ASTWalker
{
public:
	explicit MovableChecker(Dialect const& _dialect);
	MovableChecker(Dialect const& _dialect, Expression const& _expression);

	void operator()(Identifier const& _identifier) override;
	void operator()(FunctionalInstruction const& _functionalInstruction) override;
	void operator()(FunctionCall const& _functionCall) override;

	/// Disallow visiting anything apart from Expressions (this throws).
	void visit(Statement const&) override;
	using ASTWalker::visit;

	bool movable() const { return m_movable; }
	bool sideEffectFree() const { return m_sideEffectFree; }

	std::set<YulString> const& referencedVariables() const { return m_variableReferences; }

private:
	Dialect const& m_dialect;
	/// Which variables the current expression references.
	std::set<YulString> m_variableReferences;
	/// Is the current expression movable or not.
	bool m_movable = true;
	/// Is the current expression side-effect free, i.e. can be removed
	/// without changing the semantics.
	bool m_sideEffectFree = true;
};

/**
 * Helper class to find "irregular" control flow.
 * This includes termination, break and continue.
 */
class TerminationFinder
{
public:
	enum class ControlFlow { FlowOut, Break, Continue, Terminate };

	/// @returns the index of the first statement in the provided sequence
	/// that is an unconditional ``break``, ``continue`` or a
	/// call to a terminating builtin function.
	/// If control flow can continue at the end of the list,
	/// returns `FlowOut` and ``size_t(-1)``.
	/// The function might return ``FlowOut`` even though control
	/// flow cannot actually continue.
	static std::pair<ControlFlow, size_t> firstUnconditionalControlFlowChange(
		std::vector<Statement> const& _statements
	);

	/// @returns the control flow type of the given statement.
	/// This function could return FlowOut even if control flow never continues.
	static ControlFlow controlFlowKind(Statement const& _statement);

	/// @returns true if the expression statement is a direct
	/// call to a builtin terminating function like
	/// ``stop``, ``revert`` or ``return``.
	static bool isTerminatingBuiltin(ExpressionStatement const& _exprStmnt);
};

}
