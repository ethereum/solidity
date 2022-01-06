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
 * Optimiser component that removes stores to memory and storage slots that are not used
 * or overwritten later on.
 */

#include <libyul/optimiser/UnusedStoreEliminator.h>

#include <libyul/optimiser/Semantics.h>
#include <libyul/optimiser/OptimizerUtilities.h>
#include <libyul/optimiser/Semantics.h>
#include <libyul/optimiser/SSAValueTracker.h>
#include <libyul/optimiser/DataFlowAnalyzer.h>
#include <libyul/optimiser/KnowledgeBase.h>
#include <libyul/ControlFlowSideEffectsCollector.h>
#include <libyul/AST.h>

#include <libsolutil/CommonData.h>

#include <libevmasm/Instruction.h>
#include <libevmasm/SemanticInformation.h>

#include <range/v3/algorithm/all_of.hpp>

using namespace std;
using namespace solidity;
using namespace solidity::yul;

/// Variable names for special constants that can never appear in actual Yul code.
static string const zero{"@ 0"};
static string const one{"@ 1"};
static string const thirtyTwo{"@ 32"};


void UnusedStoreEliminator::run(OptimiserStepContext& _context, Block& _ast)
{
	map<YulString, SideEffects> functionSideEffects = SideEffectsPropagator::sideEffects(
		_context.dialect,
		CallGraphGenerator::callGraph(_ast)
	);

	SSAValueTracker ssaValues;
	ssaValues(_ast);
	map<YulString, AssignedValue> values;
	for (auto const& [name, expression]: ssaValues.values())
		values[name] = AssignedValue{expression, {}};
	Expression const zeroLiteral{Literal{{}, LiteralKind::Number, YulString{"0"}, {}}};
	Expression const oneLiteral{Literal{{}, LiteralKind::Number, YulString{"1"}, {}}};
	Expression const thirtyTwoLiteral{Literal{{}, LiteralKind::Number, YulString{"32"}, {}}};
	values[YulString{zero}] = AssignedValue{&zeroLiteral, {}};
	values[YulString{one}] = AssignedValue{&oneLiteral, {}};
	values[YulString{thirtyTwo}] = AssignedValue{&thirtyTwoLiteral, {}};

	bool const ignoreMemory = MSizeFinder::containsMSize(_context.dialect, _ast);
	UnusedStoreEliminator rse{
		_context.dialect,
		functionSideEffects,
		ControlFlowSideEffectsCollector{_context.dialect, _ast}.functionSideEffectsNamed(),
		values,
		ignoreMemory
	};
	rse(_ast);
	rse.changeUndecidedTo(State::Unused, Location::Memory);
	rse.changeUndecidedTo(State::Used, Location::Storage);
	rse.scheduleUnusedForDeletion();

	StatementRemover remover(rse.m_pendingRemovals);
	remover(_ast);
}

void UnusedStoreEliminator::operator()(FunctionCall const& _functionCall)
{
	UnusedStoreBase::operator()(_functionCall);

	for (Operation const& op: operationsFromFunctionCall(_functionCall))
		applyOperation(op);

	ControlFlowSideEffects sideEffects;
	if (auto builtin = m_dialect.builtin(_functionCall.functionName.name))
		sideEffects = builtin->controlFlowSideEffects;
	else
		sideEffects = m_controlFlowSideEffects.at(_functionCall.functionName.name);

	if (!sideEffects.canContinue)
	{
		changeUndecidedTo(State::Unused, Location::Memory);
		changeUndecidedTo(sideEffects.canTerminate ? State::Used : State::Unused, Location::Storage);
	}
}

void UnusedStoreEliminator::operator()(FunctionDefinition const& _functionDefinition)
{
	ScopedSaveAndRestore storeOperations(m_storeOperations, {});
	UnusedStoreBase::operator()(_functionDefinition);
}


void UnusedStoreEliminator::operator()(Leave const&)
{
	changeUndecidedTo(State::Used);
}

void UnusedStoreEliminator::visit(Statement const& _statement)
{
	using evmasm::Instruction;

	UnusedStoreBase::visit(_statement);

	auto const* exprStatement = get_if<ExpressionStatement>(&_statement);
	if (!exprStatement)
		return;

	FunctionCall const* funCall = get_if<FunctionCall>(&exprStatement->expression);
	if (!funCall)
		return;
	optional<Instruction> instruction = toEVMInstruction(m_dialect, funCall->functionName.name);
	if (!instruction)
		return;

	if (!ranges::all_of(funCall->arguments, [](Expression const& _expr) -> bool {
		return get_if<Identifier>(&_expr) || get_if<Literal>(&_expr);
	}))
		return;

	if (
		*instruction == Instruction::SSTORE ||
		(!m_ignoreMemory && (
			*instruction == Instruction::EXTCODECOPY ||
			*instruction == Instruction::CODECOPY ||
			*instruction == Instruction::CALLDATACOPY ||
			*instruction == Instruction::RETURNDATACOPY ||
			*instruction == Instruction::MSTORE ||
			*instruction == Instruction::MSTORE8
		))
	)
	{
		m_stores[YulString{}].insert({&_statement, State::Undecided});
		vector<Operation> operations = operationsFromFunctionCall(*funCall);
		yulAssert(operations.size() == 1, "");
		m_storeOperations[&_statement] = move(operations.front());
	}
}

void UnusedStoreEliminator::finalizeFunctionDefinition(FunctionDefinition const&)
{
	changeUndecidedTo(State::Used);
	scheduleUnusedForDeletion();
}

vector<UnusedStoreEliminator::Operation> UnusedStoreEliminator::operationsFromFunctionCall(
	FunctionCall const& _functionCall
) const
{
	using evmasm::Instruction;

	YulString functionName = _functionCall.functionName.name;
	SideEffects sideEffects;
	if (BuiltinFunction const* f = m_dialect.builtin(functionName))
		sideEffects = f->sideEffects;
	else
		sideEffects = m_functionSideEffects.at(functionName);

	optional<Instruction> instruction = toEVMInstruction(m_dialect, functionName);
	if (!instruction)
	{
		vector<Operation> result;
		// Unknown read is worse than unknown write.
		if (sideEffects.memory != SideEffects::Effect::None)
			result.emplace_back(Operation{Location::Memory, Effect::Read, {}, {}});
		if (sideEffects.storage != SideEffects::Effect::None)
			result.emplace_back(Operation{Location::Storage, Effect::Read, {}, {}});
		return result;
	}

	switch (*instruction)
	{
	case Instruction::SSTORE:
	case Instruction::SLOAD:
	case Instruction::MSTORE:
	case Instruction::MSTORE8:
	case Instruction::MLOAD:
	case Instruction::REVERT:
	case Instruction::RETURN:
	case Instruction::EXTCODECOPY:
	case Instruction::CODECOPY:
	case Instruction::CALLDATACOPY:
	case Instruction::RETURNDATACOPY:
	case Instruction::KECCAK256:
	case Instruction::LOG0:
	case Instruction::LOG1:
	case Instruction::LOG2:
	case Instruction::LOG3:
	case Instruction::LOG4:
	{
		Operation op;
		if (sideEffects.memory == SideEffects::Write || sideEffects.storage == SideEffects::Write)
			op.effect = Effect::Write;
		else
			op.effect = Effect::Read;

		op.location =
			(instruction == Instruction::SSTORE || instruction == Instruction::SLOAD) ?
			Location::Storage :
			Location::Memory;

		if (*instruction == Instruction::EXTCODECOPY)
			op.start = identifierNameIfSSA(_functionCall.arguments.at(1));
		else
			op.start = identifierNameIfSSA(_functionCall.arguments.at(0));

		if (instruction == Instruction::MSTORE || instruction == Instruction::MLOAD)
			op.length = YulString(thirtyTwo);
		else if (instruction == Instruction::MSTORE8)
			op.length = YulString(one);
		else if (
			instruction == Instruction::REVERT ||
			instruction == Instruction::RETURN ||
			instruction == Instruction::KECCAK256 ||
			instruction == Instruction::LOG0 ||
			instruction == Instruction::LOG1 ||
			instruction == Instruction::LOG2 ||
			instruction == Instruction::LOG3 ||
			instruction == Instruction::LOG4
		)
			op.length = identifierNameIfSSA(_functionCall.arguments.at(1));
		else if (*instruction == Instruction::EXTCODECOPY)
			op.length = identifierNameIfSSA(_functionCall.arguments.at(3));
		else if (
			instruction == Instruction::CALLDATACOPY ||
			instruction == Instruction::CODECOPY ||
			instruction == Instruction::RETURNDATACOPY
		)
			op.length = identifierNameIfSSA(_functionCall.arguments.at(2));
		else if (instruction == Instruction::SSTORE || instruction == Instruction::SLOAD)
			// Storage operations, length is unused / non-sensical
			op.length = {};
		else
			yulAssert(false);

		return {op};
	}
	case Instruction::STATICCALL:
	case Instruction::CALL:
	case Instruction::CALLCODE:
	case Instruction::DELEGATECALL:
	{
		size_t arguments = _functionCall.arguments.size();
		return vector<Operation>{
			Operation{
				Location::Memory,
				Effect::Read,
				identifierNameIfSSA(_functionCall.arguments.at(arguments - 4)),
				identifierNameIfSSA(_functionCall.arguments.at(arguments - 3))
			},
			// Unknown read includes unknown write.
			Operation{Location::Storage, Effect::Read, {}, {}},
			Operation{
				Location::Memory,
				Effect::Write,
				identifierNameIfSSA(_functionCall.arguments.at(arguments - 2)),
				identifierNameIfSSA(_functionCall.arguments.at(arguments - 1))
			}
		};
	}
	case Instruction::CREATE:
	case Instruction::CREATE2:
		return vector<Operation>{
			Operation{
				Location::Memory,
				Effect::Read,
				identifierNameIfSSA(_functionCall.arguments.at(1)),
				identifierNameIfSSA(_functionCall.arguments.at(2))
			},
			// Unknown read includes unknown write.
			Operation{Location::Storage, Effect::Read, {}, {}},
		};
	case Instruction::MSIZE:
		// This is just to satisfy the assert below.
		return vector<Operation>{};
	default:
		break;
	}

	yulAssert(
		evmasm::SemanticInformation::storage(*instruction) ==
		evmasm::SemanticInformation::None,
		""
	);
	yulAssert(
		evmasm::SemanticInformation::memory(*instruction) ==
		evmasm::SemanticInformation::None,
		""
	);
	yulAssert(
		sideEffects.memory == SideEffects::Effect::None &&
		sideEffects.storage == SideEffects::Effect::None,
		""
	);
	return {};
}

void UnusedStoreEliminator::applyOperation(UnusedStoreEliminator::Operation const& _operation)
{
	for (auto& [statement, state]: m_stores[YulString{}])
		if (state == State::Undecided)
		{
			Operation const& storeOperation = m_storeOperations.at(statement);
			if (_operation.effect == Effect::Read && !knownUnrelated(storeOperation, _operation))
				state = State::Used;
			else if (_operation.effect == Effect::Write && knownCovered(storeOperation, _operation))
				state = State::Unused;
		}
}

bool UnusedStoreEliminator::knownUnrelated(
	UnusedStoreEliminator::Operation const& _op1,
	UnusedStoreEliminator::Operation const& _op2
) const
{
	KnowledgeBase knowledge(m_dialect, m_ssaValues);

	if (_op1.location != _op2.location)
		return true;
	if (_op1.location == Location::Storage)
	{
		if (_op1.start && _op2.start)
			return knowledge.knownToBeDifferent(*_op1.start, *_op2.start);
	}
	else
	{
		yulAssert(_op1.location == Location::Memory, "");
		if (_op1.length && knowledge.knownToBeZero(*_op1.length))
			return true;
		if (_op2.length && knowledge.knownToBeZero(*_op2.length))
			return true;

		u256 largestPositive = (u256(1) << 128) - 1;
		// 1.start + 1.length <= 2.start
		if (_op1.length && _op2.start && _op1.start)
		{
			optional<u256> length1 = knowledge.valueIfKnownConstant(*_op1.length);
			optional<u256> diff = knowledge.differenceIfKnownConstant(*_op2.start, *_op1.start);
			// diff = 2.start - 1.start
			// We assume both 2.start and 1.start to be "small", so
			// if diff <= largestPositive, 2.start >= 1.start
			if (length1 && diff)
				if (*length1 <= *diff && *diff <= largestPositive)
					return true;
		}
		// 2.start + 2.length <= 1.start
		if (_op2.length && _op1.start && _op2.start)
		{
			optional<u256> length2 = knowledge.valueIfKnownConstant(*_op2.length);
			optional<u256> diff = knowledge.differenceIfKnownConstant(*_op1.start, *_op2.start);
			// diff = 1.start - 2.start
			if (length2 && diff)
				if (*length2 <= *diff && *diff <= largestPositive)
					return true;
		}
	}

	return false;
}

bool UnusedStoreEliminator::knownCovered(
	UnusedStoreEliminator::Operation const& _covered,
	UnusedStoreEliminator::Operation const& _covering
) const
{
	if (_covered.location != _covering.location)
		return false;
	if (
		(_covered.start && _covered.start == _covering.start) &&
		// length is unused for storage
		(_covered.location == Location::Storage || (_covered.length && _covered.length == _covering.length))
	)
		return true;
	if (_covered.location == Location::Memory)
	{
		KnowledgeBase knowledge(m_dialect, m_ssaValues);
		u256 largestPositive = (u256(1) << 128) - 1;

		if (_covered.length && knowledge.knownToBeZero(*_covered.length))
			return true;
		// Condition (i = cover_i_ng, e = cover_e_d):
		// i.start <= e.start && e.start + e.length <= i.start + i.length
		if (!_covered.start || !_covering.start || !_covered.length || !_covering.length)
			return false;
		optional<u256> startDiff = knowledge.differenceIfKnownConstant(*_covered.start, *_covering.start);
		//optional<u256> coveredLength = knowledge.valueIfKnownConstant(*_covered.length);
		optional<u256> lengthDiff = knowledge.differenceIfKnownConstant(*_covering.length, *_covered.length);
		if (
			(startDiff && /*coveredLength &&*/ lengthDiff) &&
			// i.start <= e.start
			*startDiff <= largestPositive &&
			// e.length <= i.length
			*lengthDiff <= largestPositive &&
			// e.start - i.start <= i.length - e.length  <=> e.start + e.length <= i.start + i.length
			*startDiff <= *lengthDiff //&&
			// Just a safety measure against overflow.
			//*coveredLength <= largestPositive
		)
			return true;
	}
	return false;
}

void UnusedStoreEliminator::changeUndecidedTo(
	State _newState,
	optional<UnusedStoreEliminator::Location> _onlyLocation)
{
	for (auto& [statement, state]: m_stores[YulString{}])
		if (
			state == State::Undecided &&
			(_onlyLocation == nullopt || *_onlyLocation == m_storeOperations.at(statement).location)
		)
			state = _newState;
}

optional<YulString> UnusedStoreEliminator::identifierNameIfSSA(Expression const& _expression) const
{
	if (Identifier const* identifier = get_if<Identifier>(&_expression))
		if (m_ssaValues.count(identifier->name))
			return {identifier->name};
	return nullopt;
}

void UnusedStoreEliminator::scheduleUnusedForDeletion()
{
	for (auto const& [statement, state]: m_stores[YulString{}])
		if (state == State::Unused)
			m_pendingRemovals.insert(statement);
}
