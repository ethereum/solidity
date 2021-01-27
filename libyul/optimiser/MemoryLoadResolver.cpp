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

#include "libyul/Dialect.h"
#include "libyul/YulString.h"
#include "libyul/optimiser/ASTWalker.h"
#include "libyul/optimiser/CallGraphGenerator.h"
#include "libyul/optimiser/DataFlowAnalyzer.h"
#include "libyul/optimiser/NameDispenser.h"
#include "libyul/optimiser/OptimiserStep.h"
#include "libyul/optimiser/SSAValueTracker.h"
#include <libyul/optimiser/MemoryLoadResolver.h>
#include <libyul/optimiser/Semantics.h>

#include <libyul/AST.h>
#include <libyul/SideEffects.h>
#include <libyul/Utilities.h>

#include <libsolutil/Visitor.h>
#include <libsolutil/CommonData.h>

#include <libsmtutil/SMTPortfolio.h>
#include <libsmtutil/SolverInterface.h>
#include <libsmtutil/Helpers.h>
#include <optional>

using namespace std;
using namespace solidity;
using namespace solidity::yul;
using namespace solidity::util;
using namespace solidity::smtutil;

void MemoryLoadResolver::run(OptimiserStepContext& _context, Block& _ast)
{
	MemoryLoadResolver{
		_context.dialect,
		SideEffectsPropagator::sideEffects(_context.dialect, CallGraphGenerator::callGraph(_ast)),
		SSAValueTracker::ssaVariables(_ast),
		MSizeFinder::containsMSize(_context.dialect, _ast)
	}(_ast);
}

void MemoryLoadResolver::visit(Expression& _e)
{
	Scoper::visit(_e);

	// We skip the replacing if there is msize. TODO do we need this, and can we do it earlier?
	if (m_containsMsize)
		return;

	// TODO Extract this
	// Check and replace the value of mload(x), if possible
	if (FunctionCall const* functionCall = get_if<FunctionCall>(&_e))
		if (functionCall->functionName.name == m_dialect.memoryLoadFunction(YulString{})->name)

			if (Identifier const* identifier = get_if<Identifier>(&functionCall->arguments.front()))
				if (m_memory.count(identifier->name))
				{
					auto& value = m_memory.at(identifier->name);
					if (inScope(value))
						_e = Identifier{locationOf(_e), value};
				}
}

void MemoryLoadResolver::operator()(VariableDeclaration& _v)
{
	// Otherwise, we simply assign a free variable to it.
	Scoper::operator()(_v);

	if (
		_v.variables.size() == 1 &&
		_v.value &&
		m_ssaVariables.count(_v.variables.front().name)
	)
	{
		YulString& variableName = _v.variables.front().name;
		bool const inserted = m_variables.insert({
			variableName,
			m_solver->newVariable("yul_" + variableName.str(), defaultSort())
			}).second;
		yulAssert(inserted, "");
		std::cout << "encoding: " << variableName.str() << std::endl;
		m_solver->addAssertion(m_variables.at(variableName) == encodeExpression(*_v.value));
	}
}

void MemoryLoadResolver::operator()(FunctionCall& _f)
{
	Scoper::operator()(_f);

	SideEffectsCollector sideEffects{m_dialect, _f, &m_functionSideEffects};

	if (sideEffects.invalidatesMemory())
	{
		set<YulString> keysToErase;
		for ([[maybe_unused]] auto const& [key, value]: m_memory)
			if (invalidatesMemoryLocation(key, _f))
				keysToErase.insert(key); // TODO

		for (auto const& key: keysToErase)
			m_memory.erase(key);
	}

	if (auto memoryMapping = isSimpleMStore(_f))
		// TODO can the key be already present in the m_memory?
		m_memory[memoryMapping->first] = memoryMapping->second;
}

void MemoryLoadResolver::operator()(If& _if)
{
	// We completely clear `m_memory` if condition writes to memory. Ideally this would not happen
	// because of expression splitter, splitting the condition and replacing it by an identifier.
	// That way, we only clear memory partially.
	clearMemoryIfInvalidated(*_if.condition);

	auto memory = m_memory;

	ASTModifier::operator()(_if);

	joinMemory(memory);
}

void MemoryLoadResolver::operator()(Switch& _switch)
{
	// See comment inside operator()(If& _if)
	clearMemoryIfInvalidated(*_switch.expression);

	visit(*_switch.expression);

	for (auto& _case: _switch.cases)
	{
		auto memory = m_memory;

		(*this)(_case.body);

		joinMemory(memory);

		clearMemoryIfInvalidated(_case.body);
	}

	for (auto& _case: _switch.cases)
		clearMemoryIfInvalidated(_case.body);
}

void MemoryLoadResolver::operator()(FunctionDefinition& _functionDefinition)
{
	map<YulString, YulString> memory;
	swap(memory, m_memory);

	Scoper::operator()(_functionDefinition);

	// Note that the contents of and memory at this point might be incorrect due to the fact that
	// the DataFlowAnalyzer ignores the ``leave`` statement.
	swap(memory, m_memory);
}

void MemoryLoadResolver::operator()(ForLoop& _for)
{
	// If the pre block was not empty,
	// we would have to deal with more complicated scoping rules.
	assertThrow(_for.pre.statements.empty(), OptimizerException, "");

	clearMemoryIfInvalidated(*_for.condition);
	clearMemoryIfInvalidated(_for.post);
	clearMemoryIfInvalidated(_for.body);

	visit(*_for.condition);
	(*this)(_for.body);
	clearMemoryIfInvalidated(_for.body);
	(*this)(_for.post);
	clearMemoryIfInvalidated(*_for.condition);
	clearMemoryIfInvalidated(_for.post);
	clearMemoryIfInvalidated(_for.body);
}

void MemoryLoadResolver::joinMemory(map<YulString, YulString> const& _older)
{
	// We clear if the key does not exist in the older map or if the value is different.
	// This also works for memory because _older is an "older version"
	// of m_memory and thus any overlapping write would have cleared the keys
	// that are not known to be different inside m_memory already.
	set<YulString> keysToErase;

	for (auto const& [key, value]: m_memory)
	{
		auto it = _older.find(key);
		if (it == _older.end() || it->second != value)
			keysToErase.insert(key);
	}

	for (auto const& key: keysToErase)
		m_memory.erase(key);
}


void MemoryLoadResolver::clearMemoryIfInvalidated(Block const& _block)
{
	SideEffectsCollector sideEffects(m_dialect, _block, &m_functionSideEffects);

	if (sideEffects.invalidatesMemory())
		m_memory.clear();
}

void MemoryLoadResolver::clearMemoryIfInvalidated(Expression const& _expr)
{
	SideEffectsCollector sideEffects(m_dialect, _expr, &m_functionSideEffects);

	if (sideEffects.invalidatesMemory())
		m_memory.clear();
}


std::optional<pair<YulString, YulString>> MemoryLoadResolver::isSimpleMStore(
	FunctionCall const& _functionCall
) const
{
	if (_functionCall.functionName.name == m_dialect.memoryStoreFunction({})->name)
		if (Identifier const* key = std::get_if<Identifier>(&_functionCall.arguments.front()))
			if (Identifier const* value = std::get_if<Identifier>(&_functionCall.arguments.back()))
				if (m_ssaVariables.count(key->name) && m_ssaVariables.count(value->name))
					return make_pair(key->name, value->name);
	return {};
}

smtutil::Expression MemoryLoadResolver::encodeExpression(yul::Expression const& _expression)
{
	return std::visit(GenericVisitor{
		[&](FunctionCall const& _functionCall)
		{
			if (auto const* dialect = dynamic_cast<EVMDialect const*>(&m_dialect))
				if (auto const* builtin = dialect->builtin(_functionCall.functionName.name))
					if (builtin->instruction)
						return encodeEVMBuiltin(*builtin->instruction, _functionCall.arguments);
			return newRestrictedVariable();
		},
		[&](Identifier const& _identifier)
		{
			if (// TODO do we need to check if it's an SSAVariable?
				m_ssaVariables.count(_identifier.name) &&
				m_variables.count(_identifier.name)
			)
				return m_variables.at(_identifier.name);
			else
				return newRestrictedVariable();

		},
		[&](Literal const& _literal)
		{
			return literalValue(_literal);
		}
	}, _expression);
}

smtutil::Expression MemoryLoadResolver::encodeEVMBuiltin(
	evmasm::Instruction _instruction,
	vector<yul::Expression> const& _arguments
)
{
	vector<smtutil::Expression> arguments = applyMap(
		_arguments,
		[this](yul::Expression const& _expr) { return encodeExpression(_expr); }
	);
	switch (_instruction)
	{
	// TODO during the encoding for Real, change the wrapping.
	case evmasm::Instruction::ADD:
		return wrap(arguments.at(0) + arguments.at(1));

	// Restrictions from EIP-1985
	case evmasm::Instruction::CALLDATASIZE:
	case evmasm::Instruction::CODESIZE:
	case evmasm::Instruction::EXTCODESIZE:
	case evmasm::Instruction::MSIZE:
	case evmasm::Instruction::RETURNDATASIZE:
		return newRestrictedVariable(bigint(1) << 32);
		break;
	default:
		break;
	}
	return newRestrictedVariable();
}

smtutil::Expression MemoryLoadResolver::newVariable()
{
	return m_solver->newVariable(uniqueName(), defaultSort());
}

smtutil::Expression MemoryLoadResolver::newRestrictedVariable(bigint _maxValue)
{
	smtutil::Expression var = newVariable();
	m_solver->addAssertion(0 <= var && var < smtutil::Expression(_maxValue));
	return var;
}

string MemoryLoadResolver::uniqueName()
{
	return "expr_" + to_string(m_varCounter++);
}

shared_ptr<Sort> MemoryLoadResolver::defaultSort() const
{
	// TODO Change into realsort
	return SortProvider::intSort();
}


smtutil::Expression MemoryLoadResolver::constantValue(size_t _value) const
{
	return _value;
}

smtutil::Expression MemoryLoadResolver::literalValue(Literal const& _literal) const
{
	return smtutil::Expression(valueOfLiteral(_literal));
}


smtutil::Expression MemoryLoadResolver::wrap(smtutil::Expression _value)
{
	smtutil::Expression rest = newRestrictedVariable();
	smtutil::Expression multiplier = newVariable();
	m_solver->addAssertion(_value == multiplier * smtutil::Expression(bigint(1) << 256) + rest);
	return rest;
}

// TODO does it have to be _expr
bool MemoryLoadResolver::invalidatesMemoryLocation(YulString const& _name, Expression const& _expression)
{
	if (!holds_alternative<FunctionCall>(_expression))
		return true;

	FunctionCall const& functionCall = get<FunctionCall>(_expression);

	auto addMemoryConstraints = [&](
		evmasm::Instruction _instruction,
		smtutil::Expression _memoryLocation,
		vector<yul::Expression> const& _arguments
	)
	{
		vector<smtutil::Expression> arguments = applyMap(
			_arguments,
			[this](yul::Expression const& _expr) { return encodeExpression(_expr); }
		);

		switch (_instruction)
		{

		case evmasm::Instruction::CALLDATACOPY:
		case evmasm::Instruction::CODECOPY:
		case evmasm::Instruction::RETURNDATACOPY:
			yulAssert(arguments.size() == 3, "");
			m_solver->addAssertion(arguments.at(0) <= _memoryLocation);
			m_solver->addAssertion(_memoryLocation < arguments.at(0) + arguments.at(2));
			break;

		case evmasm::Instruction::EXTCODECOPY:
			yulAssert(arguments.size() == 4, "");
			m_solver->addAssertion(arguments.at(1) <= _memoryLocation);
			m_solver->addAssertion(_memoryLocation < arguments.at(1) + arguments.at(3));
			break;

		// TODO Should mstore and mstore8 be dealt with separately?
		case evmasm::Instruction::MSTORE:
			yulAssert(arguments.size() == 2, "");
			m_solver->addAssertion(arguments.at(0) <= _memoryLocation);
			m_solver->addAssertion(_memoryLocation < arguments.at(0) + constantValue(32));
			break;

		case evmasm::Instruction::MSTORE8:
			yulAssert(arguments.size() == 2, "");
			m_solver->addAssertion(arguments.at(0) <= _memoryLocation);
			m_solver->addAssertion(_memoryLocation < arguments.at(0) + constantValue(1));
			break;

		case evmasm::Instruction::CALL:
		case evmasm::Instruction::CALLCODE:
			yulAssert(arguments.size() == 7, "");
			m_solver->addAssertion(arguments.at(5) <= _memoryLocation);
			m_solver->addAssertion(_memoryLocation < arguments.at(5) + arguments.at(6));
			break;

		case evmasm::Instruction::STATICCALL:
		case evmasm::Instruction::DELEGATECALL:
			yulAssert(arguments.size() == 6, "");
			m_solver->addAssertion(arguments.at(4) <= _memoryLocation);
			m_solver->addAssertion(_memoryLocation < arguments.at(4) + arguments.at(5));
			break;

		default:
			;
		}
	};

	std::cout << "Problem: ? " << _name.str() << std::endl;

	if (auto dialect = dynamic_cast<EVMDialect const*>(&m_dialect))
		if (auto builtin = dialect->builtin(functionCall.functionName.name))
			if (builtin->instruction)
			{
				// TODO at this point, the constraints should be satisfiable.
				// ADD an assert about it?

				m_solver->push();
				addMemoryConstraints(
					*builtin->instruction,
					m_variables.at(_name),
					functionCall.arguments
				);

				CheckResult result1 = m_solver->check({}).first;
				m_solver->pop();

				m_solver->push();
				addMemoryConstraints(
					*builtin->instruction,
					m_variables.at(_name) + constantValue(31),
					functionCall.arguments
				);

				CheckResult result2 = m_solver->check({}).first;
				m_solver->pop();

				if (
					(result1 == CheckResult::UNSATISFIABLE) &&
					(result2 == CheckResult::UNSATISFIABLE)
				)
						return false;

			}

	return true;
}
