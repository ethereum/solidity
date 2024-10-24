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
 * Yul interpreter.
 */

#pragma once

#include <libyul/interpreter/PureInterpreterState.h>
#include <libyul/interpreter/Results.h>
#include <libyul/interpreter/Scope.h>
#include <libyul/interpreter/types.h>

#include <libyul/ASTForward.h>

namespace solidity::yul
{
struct Dialect;
}

namespace solidity::yul::interpreter
{

/**
 * Yul interpreter.
 */
class PureInterpreter
{
public:
	PureInterpreter(
		PureInterpreterState& _state,
		Dialect const& _dialect,
		Scope& _scope,
		size_t _callerRecursionDepth,
		VariableValuesMap _variables = {}
	):
		m_dialect(_dialect),
		m_state(_state),
		m_variables(std::move(_variables)),
		m_scope(&_scope),

		// The only place that increases recursion depth
		m_recursionDepth(_callerRecursionDepth + 1)
	{
	}

	// Statement visit methods

	ExecutionResult operator()(ExpressionStatement const& _statement);
	ExecutionResult operator()(Assignment const& _assignment);
	ExecutionResult operator()(VariableDeclaration const& _varDecl);
	ExecutionResult operator()(If const& _if);
	ExecutionResult operator()(Switch const& _switch);
	ExecutionResult operator()(FunctionDefinition const&);
	ExecutionResult operator()(ForLoop const&);
	ExecutionResult operator()(Break const&);
	ExecutionResult operator()(Continue const&);
	ExecutionResult operator()(Leave const&);
	ExecutionResult operator()(Block const& _block);

	/// Only execute the statements in order.
	/// Will not alter the scope.
	ExecutionResult execute(std::vector<Statement> const& _statements);

	ExecutionResult visit(Statement const& _statement);

	// Expression visit methods

	EvaluationResult operator()(Literal const&);
	EvaluationResult operator()(Identifier const&);
	EvaluationResult operator()(FunctionCall const& _functionCall);

	EvaluationResult visit(Expression const& _expression);

	u256 valueOfVariable(YulName _name) const { return m_variables.at(_name); }
	VariableValuesMap const& allVariables() const { return m_variables; }

protected:
	PureInterpreter makeInterpreterCopy(VariableValuesMap _variables = {}) const
	{
		return PureInterpreter(
			m_state,
			m_dialect,
			*m_scope,
			m_recursionDepth,
			std::move(_variables)
		);
	}

	// evaluate the expression and assert that the number of return variable is _numReturnVars
	EvaluationResult evaluate(Expression const& _expression, size_t _numReturnVars);

	/// Evaluates the given expression from right to left and
	/// stores it in m_value.
	EvaluationResult evaluateArgs(std::vector<Expression> const& _arguments);

	void enterScope(Block const& _block);
	void leaveScope();

	/// Increment interpreter step count, returning StepLimitReached if step
	/// limit is reached.
	BOOST_OUTCOME_V2_NAMESPACE::result<void, ExecutionTerminated> incrementStatementStep();

	/// Increment evaluation count, returning ExpressionNestingLimitReached if
	/// the nesting level is beyond the upper bound configured in the
	/// interpreter state.
	BOOST_OUTCOME_V2_NAMESPACE::result<void, ExecutionTerminated> incrementExpressionStep();

	Dialect const& m_dialect;
	PureInterpreterState& m_state;

	VariableValuesMap m_variables;
	Scope* m_scope;

	size_t const m_recursionDepth{};
	unsigned m_expressionNestingLevel{};
};

}
