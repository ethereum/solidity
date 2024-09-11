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

#include <test/tools/yulInterpreter/Interpreter.h>

#include <test/tools/yulInterpreter/EVMInstructionInterpreter.h>

#include <libyul/AST.h>
#include <libyul/Dialect.h>
#include <libyul/Utilities.h>
#include <libyul/backends/evm/EVMDialect.h>

#include <liblangutil/Exceptions.h>

#include <libsolutil/FixedHash.h>

#include <range/v3/view/reverse.hpp>

#include <ostream>
#include <variant>

using namespace solidity;
using namespace solidity::yul;
using namespace solidity::yul::test;

using solidity::util::h256;

void InterpreterState::dumpStorage(std::ostream& _out) const
{
	for (auto const& [slot, value]: storage)
		if (value != h256{})
			_out << "  " << slot.hex() << ": " << value.hex() << std::endl;
}

void InterpreterState::dumpTransientStorage(std::ostream& _out) const
{
	for (auto const& [slot, value]: transientStorage)
		if (value != h256{})
			_out << "  " << slot.hex() << ": " << value.hex() << std::endl;
}

void InterpreterState::dumpTraceAndState(std::ostream& _out, bool _disableMemoryTrace) const
{
	_out << "Trace:" << std::endl;
	for (auto const& line: trace)
		_out << "  " << line << std::endl;
	if (!_disableMemoryTrace)
	{
		_out << "Memory dump:\n";
		std::map<u256, u256> words;
		for (auto const& [offset, value]: memory)
			words[(offset / 0x20) * 0x20] |= u256(uint32_t(value)) << (256 - 8 - 8 * static_cast<size_t>(offset % 0x20));
		for (auto const& [offset, value]: words)
			if (value != 0)
				_out << "  " << std::uppercase << std::hex << std::setw(4) << offset << ": " << h256(value).hex() << std::endl;
	}
	_out << "Storage dump:" << std::endl;
	dumpStorage(_out);

	_out << "Transient storage dump:" << std::endl;
	dumpTransientStorage(_out);

	if (!calldata.empty())
	{
		_out << "Calldata dump:";

		for (size_t offset = 0; offset < calldata.size(); ++offset)
			if (calldata[offset] != 0)
			{
				if (offset % 32 == 0)
					_out <<
						std::endl <<
						"  " <<
						std::uppercase <<
						std::hex <<
						std::setfill(' ') <<
						std::setw(4) <<
						offset <<
						": ";

				_out <<
					std::hex <<
					std::setw(2) <<
					std::setfill('0') <<
					static_cast<int>(calldata[offset]);
			}

		_out << std::endl;
	}
}

void Interpreter::run(
	InterpreterState& _state,
	Dialect const& _dialect,
	Block const& _ast,
	bool _disableExternalCalls,
	bool _disableMemoryTrace
)
{
	Scope scope;
	Interpreter{_state, _dialect, scope, _disableExternalCalls, _disableMemoryTrace}(_ast);
}

void Interpreter::operator()(ExpressionStatement const& _expressionStatement)
{
	evaluateMulti(_expressionStatement.expression);
}

void Interpreter::operator()(Assignment const& _assignment)
{
	solAssert(_assignment.value, "");
	std::vector<u256> values = evaluateMulti(*_assignment.value);
	solAssert(values.size() == _assignment.variableNames.size(), "");
	for (size_t i = 0; i < values.size(); ++i)
	{
		YulName varName = _assignment.variableNames.at(i).name;
		solAssert(m_variables.count(varName), "");
		m_variables[varName] = values.at(i);
	}
}

void Interpreter::operator()(VariableDeclaration const& _declaration)
{
	std::vector<u256> values(_declaration.variables.size(), 0);
	if (_declaration.value)
		values = evaluateMulti(*_declaration.value);

	solAssert(values.size() == _declaration.variables.size(), "");
	for (size_t i = 0; i < values.size(); ++i)
	{
		YulName varName = _declaration.variables.at(i).name;
		solAssert(!m_variables.count(varName), "");
		m_variables[varName] = values.at(i);
		m_scope->names.emplace(varName, nullptr);
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

	enterScope(_forLoop.pre);
	ScopeGuard g([this]{ leaveScope(); });

	for (auto const& statement: _forLoop.pre.statements)
	{
		visit(statement);
		if (m_state.controlFlowState == ControlFlowState::Leave)
			return;
	}
	while (evaluate(*_forLoop.condition) != 0)
	{
		// Increment step for each loop iteration for loops with
		// an empty body and post blocks to prevent a deadlock.
		if (_forLoop.body.statements.size() == 0 && _forLoop.post.statements.size() == 0)
			incrementStep();

		m_state.controlFlowState = ControlFlowState::Default;
		(*this)(_forLoop.body);
		if (m_state.controlFlowState == ControlFlowState::Break || m_state.controlFlowState == ControlFlowState::Leave)
			break;

		m_state.controlFlowState = ControlFlowState::Default;
		(*this)(_forLoop.post);
		if (m_state.controlFlowState == ControlFlowState::Leave)
			break;
	}
	if (m_state.controlFlowState != ControlFlowState::Leave)
		m_state.controlFlowState = ControlFlowState::Default;
}

void Interpreter::operator()(Break const&)
{
	m_state.controlFlowState = ControlFlowState::Break;
}

void Interpreter::operator()(Continue const&)
{
	m_state.controlFlowState = ControlFlowState::Continue;
}

void Interpreter::operator()(Leave const&)
{
	m_state.controlFlowState = ControlFlowState::Leave;
}

void Interpreter::operator()(Block const& _block)
{
	enterScope(_block);
	// Register functions.
	for (auto const& statement: _block.statements)
		if (std::holds_alternative<FunctionDefinition>(statement))
		{
			FunctionDefinition const& funDef = std::get<FunctionDefinition>(statement);
			m_scope->names.emplace(funDef.name, &funDef);
		}

	for (auto const& statement: _block.statements)
	{
		incrementStep();
		visit(statement);
		if (m_state.controlFlowState != ControlFlowState::Default)
			break;
	}

	leaveScope();
}

u256 Interpreter::evaluate(Expression const& _expression)
{
	ExpressionEvaluator ev(m_state, m_dialect, *m_scope, m_variables, m_disableExternalCalls, m_disableMemoryTrace);
	ev.visit(_expression);
	return ev.value();
}

std::vector<u256> Interpreter::evaluateMulti(Expression const& _expression)
{
	ExpressionEvaluator ev(m_state, m_dialect, *m_scope, m_variables, m_disableExternalCalls, m_disableMemoryTrace);
	ev.visit(_expression);
	return ev.values();
}

void Interpreter::enterScope(Block const& _block)
{
	if (!m_scope->subScopes.count(&_block))
		m_scope->subScopes[&_block] = std::make_unique<Scope>(Scope{
			{},
			{},
			m_scope
		});
	m_scope = m_scope->subScopes[&_block].get();
}

void Interpreter::leaveScope()
{
	for (auto const& [var, funDeclaration]: m_scope->names)
		if (!funDeclaration)
			m_variables.erase(var);
	m_scope = m_scope->parent;
	yulAssert(m_scope, "");
}

void Interpreter::incrementStep()
{
	m_state.numSteps++;
	if (m_state.maxSteps > 0 && m_state.numSteps >= m_state.maxSteps)
	{
		m_state.trace.emplace_back("Interpreter execution step limit reached.");
		BOOST_THROW_EXCEPTION(StepLimitReached());
	}
}

void ExpressionEvaluator::operator()(Literal const& _literal)
{
	incrementStep();
	setValue(_literal.value.value());
}

void ExpressionEvaluator::operator()(Identifier const& _identifier)
{
	solAssert(m_variables.count(_identifier.name), "");
	incrementStep();
	setValue(m_variables.at(_identifier.name));
}

void ExpressionEvaluator::operator()(FunctionCall const& _funCall)
{
	std::vector<std::optional<LiteralKind>> const* literalArguments = nullptr;
	if (BuiltinFunction const* builtin = m_dialect.builtin(_funCall.functionName.name))
		if (!builtin->literalArguments.empty())
			literalArguments = &builtin->literalArguments;
	evaluateArgs(_funCall.arguments, literalArguments);

	if (EVMDialect const* dialect = dynamic_cast<EVMDialect const*>(&m_dialect))
	{
		if (BuiltinFunctionForEVM const* fun = dialect->builtin(_funCall.functionName.name))
		{
			EVMInstructionInterpreter interpreter(dialect->evmVersion(), m_state, m_disableMemoryTrace);

			u256 const value = interpreter.evalBuiltin(*fun, _funCall.arguments, values());

			if (
				!m_disableExternalCalls &&
				fun->instruction &&
				evmasm::isCallInstruction(*fun->instruction)
			)
				runExternalCall(*fun->instruction);

			setValue(value);
			return;
		}
	}

	Scope* scope = &m_scope;
	for (; scope; scope = scope->parent)
		if (scope->names.count(_funCall.functionName.name))
			break;
	yulAssert(scope, "");

	FunctionDefinition const* fun = scope->names.at(_funCall.functionName.name);
	yulAssert(fun, "Function not found.");
	yulAssert(m_values.size() == fun->parameters.size(), "");
	std::map<YulName, u256> variables;
	for (size_t i = 0; i < fun->parameters.size(); ++i)
		variables[fun->parameters.at(i).name] = m_values.at(i);
	for (size_t i = 0; i < fun->returnVariables.size(); ++i)
		variables[fun->returnVariables.at(i).name] = 0;

	m_state.controlFlowState = ControlFlowState::Default;
	std::unique_ptr<Interpreter> interpreter = makeInterpreterCopy(std::move(variables));
	(*interpreter)(fun->body);
	m_state.controlFlowState = ControlFlowState::Default;

	m_values.clear();
	for (auto const& retVar: fun->returnVariables)
		m_values.emplace_back(interpreter->valueOfVariable(retVar.name));
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

void ExpressionEvaluator::evaluateArgs(
	std::vector<Expression> const& _expr,
	std::vector<std::optional<LiteralKind>> const* _literalArguments
)
{
	incrementStep();
	std::vector<u256> values;
	size_t i = 0;
	/// Function arguments are evaluated in reverse.
	for (auto const& expr: _expr | ranges::views::reverse)
	{
		if (!_literalArguments || !_literalArguments->at(_expr.size() - i - 1))
			visit(expr);
		else
		{
			if (std::get<Literal>(expr).value.unlimited())
			{
				yulAssert(std::get<Literal>(expr).kind == LiteralKind::String);
				m_values = {0xdeadbeef};
			}
			else
				m_values = {std::get<Literal>(expr).value.value()};
		}

		values.push_back(value());
		++i;
	}
	m_values = std::move(values);
	std::reverse(m_values.begin(), m_values.end());
}

void ExpressionEvaluator::incrementStep()
{
	m_nestingLevel++;
	if (m_state.maxExprNesting > 0 && m_nestingLevel > m_state.maxExprNesting)
	{
		m_state.trace.emplace_back("Maximum expression nesting level reached.");
		BOOST_THROW_EXCEPTION(ExpressionNestingLimitReached());
	}
}

void ExpressionEvaluator::runExternalCall(evmasm::Instruction _instruction)
{
	u256 memOutOffset = 0;
	u256 memOutSize = 0;
	u256 callvalue = 0;
	u256 memInOffset = 0;
	u256 memInSize = 0;

	// Setup memOut* values
	if (
		_instruction == evmasm::Instruction::CALL ||
		_instruction == evmasm::Instruction::CALLCODE
	)
	{
		memOutOffset = values()[5];
		memOutSize = values()[6];
		callvalue = values()[2];
		memInOffset = values()[3];
		memInSize = values()[4];
	}
	else if (
		_instruction == evmasm::Instruction::DELEGATECALL ||
		_instruction == evmasm::Instruction::STATICCALL
	)
	{
		memOutOffset = values()[4];
		memOutSize = values()[5];
		memInOffset = values()[2];
		memInSize = values()[3];
	}
	else
		yulAssert(false);

	// Don't execute external call if it isn't our own address
	if (values()[1] != util::h160::Arith(m_state.address))
		return;

	Scope tmpScope;
	InterpreterState tmpState;
	tmpState.calldata = m_state.readMemory(memInOffset, memInSize);
	tmpState.callvalue = callvalue;
	tmpState.numInstance = m_state.numInstance + 1;

	yulAssert(tmpState.numInstance < 1024, "Detected more than 1024 recursive calls, aborting...");

	// Create new interpreter for the called contract
	std::unique_ptr<Interpreter> newInterpreter = makeInterpreterNew(tmpState, tmpScope);

	Scope* abstractRootScope = &m_scope;
	Scope* fileScope = nullptr;
	Block const* ast = nullptr;

	// Find file scope
	while (abstractRootScope->parent)
	{
		fileScope = abstractRootScope;
		abstractRootScope = abstractRootScope->parent;
	}

	// Get AST for file scope
	for (auto&& [block, scope]: abstractRootScope->subScopes)
		if (scope.get() == fileScope)
		{
			ast = block;
			break;
		}

	yulAssert(ast);

	try
	{
		(*newInterpreter)(*ast);
	}
	catch (ExplicitlyTerminatedWithReturn const&)
	{
		// Copy return data to our memory
		copyZeroExtended(
			m_state.memory,
			newInterpreter->returnData(),
			memOutOffset.convert_to<size_t>(),
			0,
			memOutSize.convert_to<size_t>()
		);
		m_state.returndata = newInterpreter->returnData();
	}
}
