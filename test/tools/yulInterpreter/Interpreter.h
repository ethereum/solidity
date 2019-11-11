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
 * Yul interpreter.
 */

#pragma once

#include <libyul/AsmDataForward.h>
#include <libyul/optimiser/ASTWalker.h>

#include <libdevcore/FixedHash.h>
#include <libdevcore/CommonData.h>

#include <libdevcore/Exceptions.h>

#include <map>

namespace yul
{
struct Dialect;

namespace test
{

class InterpreterTerminatedGeneric: public dev::Exception
{
};

class ExplicitlyTerminated: public InterpreterTerminatedGeneric
{
};

class StepLimitReached: public InterpreterTerminatedGeneric
{
};

class TraceLimitReached: public InterpreterTerminatedGeneric
{
};

enum class ControlFlowState
{
	Default,
	Continue,
	Break,
	Leave
};

struct InterpreterState
{
	dev::bytes calldata;
	dev::bytes returndata;
	std::map<dev::u256, uint8_t> memory;
	/// This is different than memory.size() because we ignore gas.
	dev::u256 msize;
	std::map<dev::h256, dev::h256> storage;
	dev::u160 address = 0x11111111;
	dev::u256 balance = 0x22222222;
	dev::u256 selfbalance = 0x22223333;
	dev::u160 origin = 0x33333333;
	dev::u160 caller = 0x44444444;
	dev::u256 callvalue = 0x55555555;
	/// Deployed code
	dev::bytes code = dev::asBytes("codecodecodecodecode");
	dev::u256 gasprice = 0x66666666;
	dev::u160 coinbase = 0x77777777;
	dev::u256 timestamp = 0x88888888;
	dev::u256 blockNumber = 1024;
	dev::u256 difficulty = 0x9999999;
	dev::u256 gaslimit = 4000000;
	dev::u256 chainid = 0x01;
	/// Log of changes / effects. Sholud be structured data in the future.
	std::vector<std::string> trace;
	/// This is actually an input parameter that more or less limits the runtime.
	size_t maxTraceSize = 0;
	size_t maxSteps = 0;
	size_t numSteps = 0;
	ControlFlowState controlFlowState = ControlFlowState::Default;

	void dumpTraceAndState(std::ostream& _out) const;
};

/**
 * Yul interpreter.
 */
class Interpreter: public ASTWalker
{
public:
	Interpreter(
		InterpreterState& _state,
		Dialect const& _dialect,
		std::map<YulString, dev::u256> _variables = {},
		std::vector<std::map<YulString, FunctionDefinition const*>> _scopes = {}
	):
		m_dialect(_dialect),
		m_state(_state),
		m_variables(std::move(_variables)),
		m_scopes(std::move(_scopes))
	{}

	void operator()(ExpressionStatement const& _statement) override;
	void operator()(Assignment const& _assignment) override;
	void operator()(VariableDeclaration const& _varDecl) override;
	void operator()(If const& _if) override;
	void operator()(Switch const& _switch) override;
	void operator()(FunctionDefinition const&) override;
	void operator()(ForLoop const&) override;
	void operator()(Break const&) override;
	void operator()(Continue const&) override;
	void operator()(Leave const&) override;
	void operator()(Block const& _block) override;

	std::vector<std::string> const& trace() const { return m_state.trace; }

	dev::u256 valueOfVariable(YulString _name) const { return m_variables.at(_name); }

private:
	/// Asserts that the expression evaluates to exactly one value and returns it.
	dev::u256 evaluate(Expression const& _expression);
	/// Evaluates the expression and returns its value.
	std::vector<dev::u256> evaluateMulti(Expression const& _expression);

	void openScope() { m_scopes.push_back({}); }
	/// Unregisters variables and functions.
	void closeScope();

	Dialect const& m_dialect;
	InterpreterState& m_state;
	/// Values of variables.
	std::map<YulString, dev::u256> m_variables;
	/// Scopes of variables and functions. Used for lookup, clearing at end of blocks
	/// and passing over the visible functions across function calls.
	/// The pointer is nullptr if and only if the key is a variable.
	std::vector<std::map<YulString, FunctionDefinition const*>> m_scopes;
};

/**
 * Yul expression evaluator.
 */
class ExpressionEvaluator: public ASTWalker
{
public:
	ExpressionEvaluator(
		InterpreterState& _state,
		Dialect const& _dialect,
		std::map<YulString, dev::u256> const& _variables,
		std::vector<std::map<YulString, FunctionDefinition const*>> const& _scopes
	):
		m_state(_state),
		m_dialect(_dialect),
		m_variables(_variables),
		m_scopes(_scopes)
	{}

	void operator()(Literal const&) override;
	void operator()(Identifier const&) override;
	void operator()(FunctionCall const& _funCall) override;

	/// Asserts that the expression has exactly one value and returns it.
	dev::u256 value() const;
	/// Returns the list of values of the expression.
	std::vector<dev::u256> values() const { return m_values; }

private:
	void setValue(dev::u256 _value);

	/// Evaluates the given expression from right to left and
	/// stores it in m_value.
	void evaluateArgs(std::vector<Expression> const& _expr);

	/// Finds the function called @a _functionName in the current scope stack and returns
	/// the function's scope stack (with variables removed) and definition.
	std::pair<
		std::vector<std::map<YulString, FunctionDefinition const*>>,
		FunctionDefinition const*
	> findFunctionAndScope(YulString _functionName) const;

	InterpreterState& m_state;
	Dialect const& m_dialect;
	/// Values of variables.
	std::map<YulString, dev::u256> const& m_variables;
	/// Stack of scopes in the current context.
	std::vector<std::map<YulString, FunctionDefinition const*>> const& m_scopes;
	/// Current value of the expression
	std::vector<dev::u256> m_values;
};

}
}

