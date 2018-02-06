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
 * IULIA interpreter.
 */

#pragma once

#include <libjulia/ASTDataForward.h>
#include <libjulia/optimiser/ASTWalker.h>

#include <libdevcore/FixedHash.h>
#include <libdevcore/CommonData.h>

#include <libsolidity/interface/Exceptions.h>

#include <map>

namespace dev
{
using u120 = boost::multiprecision::number<boost::multiprecision::cpp_int_backend<120, 256, boost::multiprecision::unsigned_magnitude, boost::multiprecision::unchecked, void>>;

namespace julia
{

class InterpreterTerminated: Exception
{
};

struct InterpreterState
{
	bytes calldata;
	bytes returndata;
	/// TODO turn this into "vector with holes" for the randomized testing
	bytes memory;
	/// This is different than memory.size() because we ignore gas.
	u256 msize;
	std::map<h256, h256> storage;
	u120 address = 0x11111111;
	u256 balance = 0x22222222;
	u120 origin = 0x33333333;
	u120 caller = 0x44444444;
	u256 callvalue = 0x55555555;
	/// Deployed code
	bytes code = asBytes("codecodecodecodecode");
	u256 gasprice = 0x66666666;
	u120 coinbase = 0x77777777;
	u256 timestamp = 0x88888888;
	u256 blockNumber = 1024;
	u256 difficulty = 0x9999999;
	u256 gaslimit = 4000000;
	/// Log of changes / effects. Sholud be structured data in the future.
	std::vector<std::string> trace;
};

/**
 * IULIA interpreter.
 */
class Interpreter: public ASTWalker
{
public:
	Interpreter(
		InterpreterState& _state,
		std::map<std::string, u256> _variables = {},
		std::map<std::string, FunctionDefinition const*> _functions = {}
	):
		m_state(_state),
		m_variables(std::move(_variables)),
		m_functions(std::move(_functions))
	{}

	virtual void operator()(ExpressionStatement const& _statement) override;
	virtual void operator()(Assignment const& _assignment) override;
	virtual void operator()(VariableDeclaration const& _varDecl) override;
	virtual void operator()(If const& _if) override;
	virtual void operator()(Switch const& _switch) override;
	virtual void operator()(FunctionDefinition const&) override;
	virtual void operator()(ForLoop const&) override;
	virtual void operator()(Block const& _block) override;

	std::vector<std::string> const& trace() const { return m_state.trace; }

	u256 valueOfVariable(std::string const& _name) const { return m_variables.at(_name); }

private:
	/// Asserts that the expression evaluates to exactly one value and returns it.
	u256 evaluate(Expression const& _expression);
	/// Evaluates the expression and returns its value.
	std::vector<u256> evaluateMulti(Expression const& _expression);

	void openScope() { m_scopes.push_back({}); }
	/// Unregisters variables.
	void closeScope();

	InterpreterState& m_state;
	/// Values of variables.
	std::map<std::string, u256> m_variables;
	/// Meanings of functions.
	std::map<std::string, FunctionDefinition const*> m_functions;
	/// Scopes of variables and functions, used to clear them at end of blocks.
	std::vector<std::set<std::string>> m_scopes;
};

/**
 * IULIA expression evaluator.
 */
class ExpressionEvaluator: public ASTWalker
{
public:
	ExpressionEvaluator(
		InterpreterState& _state,
		std::map<std::string, u256> const& _variables,
		std::map<std::string, FunctionDefinition const*> const& _functions
	):
		m_state(_state),
		m_variables(_variables),
		m_functions(_functions)
	{}

	virtual void operator()(Literal const&) override;
	virtual void operator()(Identifier const&) override;
	virtual void operator()(FunctionalInstruction const& _instr) override;
	virtual void operator()(FunctionCall const& _funCall) override;

	/// Asserts that the expression has exactly one value and returns it.
	u256 value() const;
	/// Returns the list of values of the expression.
	std::vector<u256> values() const { return m_values; }

private:
	void setValue(u256 _value);

	/// Evaluates the given expression from right to left and
	/// stores it in m_value.
	void evaluateArgs(std::vector<Expression> const& _expr);

	InterpreterState& m_state;
	/// Values of variables.
	std::map<std::string, u256> const& m_variables;
	/// Meanings of functions.
	std::map<std::string, FunctionDefinition const*> const& m_functions;
	/// Current value of the expression
	std::vector<u256> m_values;
};


}
}
