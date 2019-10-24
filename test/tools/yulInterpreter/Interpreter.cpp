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

#include <test/tools/yulInterpreter/Interpreter.h>

#include <test/tools/yulInterpreter/EVMInstructionInterpreter.h>

#include <libyul/AsmData.h>
#include <libyul/Dialect.h>
#include <libyul/Utilities.h>
#include <libyul/backends/evm/EVMDialect.h>

#include <liblangutil/Exceptions.h>

#include <libdevcore/FixedHash.h>

#include <boost/range/adaptor/reversed.hpp>
#include <boost/algorithm/cxx11/all_of.hpp>

#include <ostream>

using namespace std;
using namespace dev;
using namespace yul;
using namespace yul::test;

void InterpreterState::dumpTraceAndState(ostream& _out) const
{
	_out << "Trace:" << endl;
	for (auto const& line: trace)
		_out << "  " << line << endl;
	_out << "Memory dump:\n";
	map<u256, u256> words;
	for (auto const& [offset, value]: memory)
		words[(offset / 0x20) * 0x20] |= u256(uint32_t(value)) << (256 - 8 - 8 * size_t(offset % 0x20));
	for (auto const& [offset, value]: words)
		if (value != 0)
			_out << "  " << std::uppercase << std::hex << std::setw(4) << offset << ": " << h256(value).hex() << endl;
	_out << "Storage dump:" << endl;
	for (auto const& slot: storage)
		if (slot.second != h256(0))
			_out << "  " << slot.first.hex() << ": " << slot.second.hex() << endl;
}

void Interpreter::operator()(ExpressionStatement const& _expressionStatement)
{
	evaluateMulti(_expressionStatement.expression);
}

void Interpreter::operator()(Assignment const& _assignment)
{
	solAssert(_assignment.value, "");
	vector<u256> values = evaluateMulti(*_assignment.value);
	solAssert(values.size() == _assignment.variableNames.size(), "");
	for (size_t i = 0; i < values.size(); ++i)
	{
		YulString varName = _assignment.variableNames.at(i).name;
		solAssert(m_variables.count(varName), "");
		m_variables[varName] = values.at(i);
	}
}

void Interpreter::operator()(VariableDeclaration const& _declaration)
{
	vector<u256> values(_declaration.variables.size(), 0);
	if (_declaration.value)
		values = evaluateMulti(*_declaration.value);

	solAssert(values.size() == _declaration.variables.size(), "");
	for (size_t i = 0; i < values.size(); ++i)
	{
		YulString varName = _declaration.variables.at(i).name;
		solAssert(!m_variables.count(varName), "");
		m_variables[varName] = values.at(i);
		solAssert(!m_scopes.back().count(varName), "");
		m_scopes.back().emplace(varName, nullptr);
	}
}

void Interpreter::operator()(If const& _if)
{
	solAssert(_if.condition, "");
	if (evaluate(*_if.condition) != 0)
		(*this)(_if.body);
}

void Interpreter::operator()(Switch const& _switch)
{
	solAssert(_switch.expression, "");
	u256 val = evaluate(*_switch.expression);
	solAssert(!_switch.cases.empty(), "");
	for (auto const& c: _switch.cases)
		// Default case has to be last.
		if (!c.value || evaluate(*c.value) == val)
		{
			(*this)(c.body);
			break;
		}
}

void Interpreter::operator()(FunctionDefinition const&)
{
}

void Interpreter::operator()(ForLoop const& _forLoop)
{
	solAssert(_forLoop.condition, "");

	openScope();
	for (auto const& statement: _forLoop.pre.statements)
		visit(statement);
	while (evaluate(*_forLoop.condition) != 0)
	{
		m_state.loopState = LoopState::Default;
		(*this)(_forLoop.body);
		if (m_state.loopState == LoopState::Break)
			break;

		m_state.loopState = LoopState::Default;
		(*this)(_forLoop.post);
	}
	m_state.loopState = LoopState::Default;
	closeScope();
}

void Interpreter::operator()(Break const&)
{
	m_state.loopState = LoopState::Break;
}

void Interpreter::operator()(Continue const&)
{
	m_state.loopState = LoopState::Continue;
}

void Interpreter::operator()(Block const& _block)
{
	m_state.numSteps++;
	if (m_state.maxSteps > 0 && m_state.numSteps >= m_state.maxSteps)
	{
		m_state.trace.emplace_back("Interpreter execution step limit reached.");
		throw StepLimitReached();
	}
	openScope();
	// Register functions.
	for (auto const& statement: _block.statements)
		if (statement.type() == typeid(FunctionDefinition))
		{
			FunctionDefinition const& funDef = boost::get<FunctionDefinition>(statement);
			solAssert(!m_scopes.back().count(funDef.name), "");
			m_scopes.back().emplace(funDef.name, &funDef);
		}

	for (auto const& statement: _block.statements)
	{
		visit(statement);
		if (m_state.loopState != LoopState::Default)
			break;
	}

	closeScope();
}

u256 Interpreter::evaluate(Expression const& _expression)
{
	ExpressionEvaluator ev(m_state, m_dialect, m_variables, m_scopes);
	ev.visit(_expression);
	return ev.value();
}

vector<u256> Interpreter::evaluateMulti(Expression const& _expression)
{
	ExpressionEvaluator ev(m_state, m_dialect, m_variables, m_scopes);
	ev.visit(_expression);
	return ev.values();
}

void Interpreter::closeScope()
{
	for (auto const& [var, funDeclaration]: m_scopes.back())
		if (!funDeclaration)
			solAssert(m_variables.erase(var) == 1, "");
	m_scopes.pop_back();
}

void ExpressionEvaluator::operator()(Literal const& _literal)
{
	static YulString const trueString("true");
	static YulString const falseString("false");

	setValue(valueOfLiteral(_literal));
}

void ExpressionEvaluator::operator()(Identifier const& _identifier)
{
	solAssert(m_variables.count(_identifier.name), "");
	setValue(m_variables.at(_identifier.name));
}

void ExpressionEvaluator::operator()(FunctionalInstruction const& _instr)
{
	evaluateArgs(_instr.arguments);
	EVMInstructionInterpreter interpreter(m_state);
	// The instruction might also return nothing, but it does not
	// hurt to set the value in that case.
	setValue(interpreter.eval(_instr.instruction, values()));
}

void ExpressionEvaluator::operator()(FunctionCall const& _funCall)
{
	evaluateArgs(_funCall.arguments);

	if (EVMDialect const* dialect = dynamic_cast<EVMDialect const*>(&m_dialect))
		if (BuiltinFunctionForEVM const* fun = dialect->builtin(_funCall.functionName.name))
		{
			EVMInstructionInterpreter interpreter(m_state);
			setValue(interpreter.evalBuiltin(*fun, values()));
			return;
		}

	auto [functionScopes, fun] = findFunctionAndScope(_funCall.functionName.name);

	solAssert(fun, "Function not found.");
	solAssert(m_values.size() == fun->parameters.size(), "");
	map<YulString, u256> variables;
	for (size_t i = 0; i < fun->parameters.size(); ++i)
		variables[fun->parameters.at(i).name] = m_values.at(i);
	for (size_t i = 0; i < fun->returnVariables.size(); ++i)
		variables[fun->returnVariables.at(i).name] = 0;

	Interpreter interpreter(m_state, m_dialect, variables, functionScopes);
	interpreter(fun->body);

	m_values.clear();
	for (auto const& retVar: fun->returnVariables)
		m_values.emplace_back(interpreter.valueOfVariable(retVar.name));
}

u256 ExpressionEvaluator::value() const
{
	solAssert(m_values.size() == 1, "");
	return m_values.front();
}

void ExpressionEvaluator::setValue(u256 _value)
{
	m_values.clear();
	m_values.emplace_back(std::move(_value));
}

void ExpressionEvaluator::evaluateArgs(vector<Expression> const& _expr)
{
	vector<u256> values;
	/// Function arguments are evaluated in reverse.
	for (auto const& expr: _expr | boost::adaptors::reversed)
	{
		visit(expr);
		values.push_back(value());
	}
	m_values = std::move(values);
	std::reverse(m_values.begin(), m_values.end());
}

pair<
	vector<map<YulString, FunctionDefinition const*>>,
	FunctionDefinition const*
> ExpressionEvaluator::findFunctionAndScope(YulString _functionName) const
{
	FunctionDefinition const* fun = nullptr;
	std::vector<std::map<YulString, FunctionDefinition const*>> newScopes;
	for (auto const& scope: m_scopes)
	{
		// Copy over all functions.
		newScopes.push_back({});
		for (auto const& [name, funDef]: scope)
			if (funDef)
				newScopes.back().emplace(name, funDef);
		// Stop at the called function.
		if (scope.count(_functionName))
		{
			fun = scope.at(_functionName);
			break;
		}
	}
	return {move(newScopes), fun};
}
