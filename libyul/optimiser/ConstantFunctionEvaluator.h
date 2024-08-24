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
 * Optimiser component that evaluates constant functions and replace theirs body
 * with evaluated result.
 */
#pragma once

#include <libyul/ASTForward.h>

#include <libyul/optimiser/ASTWalker.h>
#include <libyul/optimiser/OptimiserStep.h>
#include <libyul/tools/interpreter/Interpreter.h>
#include <libyul/Exceptions.h>

namespace solidity::yul
{


/**
 * Optimiser component that evaluates constant functions and replace theirs body
 * with evaluated result.
 *
 * A function is _constant_ if it satisfies all of the following criteria:
 * - It take no arguments
 * - If executed, this function only perform arithmetic operations and calls
 *   other function (that only does arithmetic expression). This means
 *   if any of reading from/writing to any memory, logging, creating contract, ...
 *   operations encountered, the function is not constant.
 *
 * Non-constant functions are left unchanged after the transformation.
 *
 * Under the hood, this component will use yul interpreter to evaluate the function.
 *
 * For example, this component may change the following code:
 *
 * function foo() -> x
 * {
 *  let u, v := bar()
 *	x := add(u, v)
 * }
 *
 * function bar() -> u, v
 * {
 * 	switch iszero(0) { u := 6 v := 9 }
 * 	default { u := 4 v := 20 }
 * }
 *
 * into
 *
 * function foo() -> x
 * { x := 15 }
 *
 * function bar() -> u, v
 * { u, v := 6, 9 }
 */
class ConstantFunctionEvaluator: public ASTModifier
{
public:
	static constexpr char const* name{"ConstantFunctionEvaluator"};
	static void run(OptimiserStepContext& _context, Block& _ast);

	void operator()(FunctionDefinition& _function) override;
	void operator()(Block& _block) override;

private:
	ConstantFunctionEvaluator(Dialect const& _dialect);

	void enterScope(Block const& _block);
	void leaveScope();

	Dialect const& m_dialect;
	tools::interpreter::Scope m_rootScope;
	tools::interpreter::Scope* m_currentScope;
};


}

namespace solidity::yul::tools::interpreter
{

class BuiltinNonArithmeticFunctionInvoked: public InterpreterTerminatedGeneric
{
};

class UnlimitedLiteralEncountered: public InterpreterTerminatedGeneric
{
};

class ArithmeticOnlyInterpreter : public Interpreter
{
public:
	ArithmeticOnlyInterpreter(
		InterpreterState& _state,
		Dialect const& _dialect,
		Scope& _scope,
		size_t _callerRecursionDepth,
		std::map<YulName, u256> _variables = {}
	): Interpreter(
		_state,
		_dialect,
		_scope,
		/* _disableExternalCalls=*/ false,  // we disable by explicit check
		/* _disableMemoryTracing=*/ true,
		_callerRecursionDepth,
		_variables
	)
	{
	}

protected:
	virtual u256 evaluate(Expression const& _expression) override;
	virtual std::vector<u256> evaluateMulti(Expression const& _expression) override;
};

class ArithmeticOnlyExpressionEvaluator: public ExpressionEvaluator
{
public:
	using ExpressionEvaluator::ExpressionEvaluator;

	void operator()(FunctionCall const& _funCall) override;

protected:
	enum class FunctionCallType
	{
		BuiltinArithmetic,
		BuiltinNonArithmetic,
		InvokeOther,
	};

	virtual std::unique_ptr<Interpreter> makeInterpreterCopy(std::map<YulName, u256> _variables = {}) const override
	{
		return std::make_unique<ArithmeticOnlyInterpreter>(
			m_state,
			m_dialect,
			m_scope,
			m_recursionDepth,
			std::move(_variables)
		);
	}
	virtual std::unique_ptr<Interpreter> makeInterpreterNew(InterpreterState& _state, Scope& _scope) const override
	{
		return std::make_unique<ArithmeticOnlyInterpreter>(
			_state,
			m_dialect,
			_scope,
			m_recursionDepth
		);
	}

	u256 getValueForUnlimitedLiteral(Literal const&) override
	{
		BOOST_THROW_EXCEPTION(UnlimitedLiteralEncountered());
	}
	FunctionCallType determineFunctionCallType(FunctionCall const& _funCall);
};

}
