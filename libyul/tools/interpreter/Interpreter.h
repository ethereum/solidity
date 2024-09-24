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

#include <libyul/ASTForward.h>
#include <libyul/YulName.h>
#include <libyul/Exceptions.h>

#include <libevmasm/Instruction.h>

#include <libsolutil/FixedHash.h>
#include <libsolutil/CommonData.h>

#include <libsolutil/Exceptions.h>

#include <map>

namespace solidity::yul
{
struct Dialect;
}

namespace solidity::yul::tools::interpreter
{

template<typename T>
class ExecutionTerminatedCommon
{
public:
	bool operator==(T) const { return true; }
	bool operator!=(T) const { return false; }
};

class ExplicitlyTerminated : public ExecutionTerminatedCommon<ExplicitlyTerminated>
{
};

class ExplicitlyTerminatedWithReturn : public ExecutionTerminatedCommon<ExplicitlyTerminatedWithReturn>
{
};

class StepLimitReached : public ExecutionTerminatedCommon<StepLimitReached>
{
};

class TraceLimitReached : public ExecutionTerminatedCommon<TraceLimitReached>
{
};

class ExpressionNestingLimitReached : public ExecutionTerminatedCommon<ExpressionNestingLimitReached>
{
};

using ExecutionTerminated = std::variant<
	ExplicitlyTerminated,
	ExplicitlyTerminatedWithReturn,
	StepLimitReached,
	TraceLimitReached,
	ExpressionNestingLimitReached
>;

enum class ControlFlowState
{
	Default,
	Continue,
	Break,
	Leave
};

struct ExecutionOk
{
	ControlFlowState state;

	bool operator==(ExecutionOk other) const
	{
		return state == other.state;
	}

	bool operator!=(ExecutionOk other) const
	{
		return state != other.state;
	}
};

struct EvaluationOk
{
	std::vector<u256> values;

	EvaluationOk(u256 _x):
		values{_x}
	{
	}

	EvaluationOk(std::vector<u256> const& _values):
		values(_values)
	{
	}
};

using ExecutionResult = std::variant<ExecutionOk, ExecutionTerminated>;
using EvaluationResult = std::variant<EvaluationOk, ExecutionTerminated>;

struct InterpreterConfig
{
	size_t maxTraceSize = 0;
	size_t maxSteps = 0;
	size_t maxExprNesting = 0;
};

struct InterpreterState
{
	InterpreterConfig const config;

	size_t numSteps = 0;

	/// Number of the current state instance, used for recursion protection
	size_t numInstance = 0;
};

/**
 * Scope structure built and maintained during execution.
 */
struct Scope
{
	/// Used for variables and functions. Value is nullptr for variables.
	std::map<YulName, FunctionDefinition const*> names;
	std::map<Block const*, std::unique_ptr<Scope>> subScopes;
	Scope* parent = nullptr;
};

/**
 * Yul interpreter.
 */
class Interpreter
{
public:
	/// Executes the Yul interpreter.
	static void run(
		InterpreterState& _state,
		Dialect const& _dialect,
		Block const& _ast
	);

	Interpreter(
		InterpreterState& _state,
		Dialect const& _dialect,
		Scope& _scope,
		std::map<YulName, u256> _variables = {}
	):
		m_dialect(_dialect),
		m_state(_state),
		m_variables(std::move(_variables)),
		m_scope(&_scope)
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

	ExecutionResult visit(Statement const& _st);

	// Expression visit methods

	EvaluationResult operator()(Literal const&);
	EvaluationResult operator()(Identifier const&);
	EvaluationResult operator()(FunctionCall const& _funCall);

	EvaluationResult visit(Expression const& _st);

	u256 valueOfVariable(YulName _name) const { return m_variables.at(_name); }

protected:
	virtual std::unique_ptr<Interpreter> makeInterpreterCopy(std::map<YulName, u256> _variables = {}) const
	{
		return std::make_unique<Interpreter>(
			m_state,
			m_dialect,
			*m_scope,
			std::move(_variables)
		);
	}

	// evaluate the expression and assert that the number of return variable is _numReturnVars
	virtual EvaluationResult evaluate(Expression const& _expression, size_t _numReturnVars);

	/// Evaluates the given expression from right to left and
	/// stores it in m_value.
	EvaluationResult evaluateArgs(
		std::vector<Expression> const& _expr,
		std::vector<std::optional<LiteralKind>> const* _literalArguments
	);

	void enterScope(Block const& _block);
	void leaveScope();

	/// Increment interpreter step count, returning StepLimitReached if step
	/// limit is reached.
	std::optional<ExecutionTerminated> incrementStatementStep();

	/// Increment evaluation count, returning ExpressionNestingLimitReached if
	/// the nesting level is beyond the upper bound configured in the
	/// interpreter state.
	std::optional<ExecutionTerminated> incrementExpressionStep();

	Dialect const& m_dialect;
	InterpreterState& m_state;
	/// Values of variables.
	std::map<YulName, u256> m_variables;
	Scope* m_scope;

	unsigned m_expressionNestingLevel = 0;
};

}
