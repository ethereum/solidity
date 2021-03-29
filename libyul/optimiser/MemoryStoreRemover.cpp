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

#include <libyul/optimiser/MemoryStoreRemover.h>
#include <libyul/optimiser/OptimizerUtilities.h>
#include <libyul/optimiser/Semantics.h>
#include <libyul/optimiser/SSAValueTracker.h>

#include <libyul/backends/evm/EVMDialect.h>
#include <libyul/backends/wasm/WasmDialect.h>

#include <libyul/Exceptions.h>

#include <libevmasm/Instruction.h>
#include <libevmasm/SemanticInformation.h>

#include <libsmtutil/SolverInterface.h>

#include <libsolutil/cxx20.h>

using namespace std;
using namespace solidity;
using namespace solidity::yul;
using namespace solidity::util;
using namespace solidity::evmasm;
using namespace solidity::smtutil;

namespace
{

/// A helper class that would replace `mstore(key, expression)` for all `key` in
/// `m_keys`, by `pop(expression)`.
class EraseMemoryStores: public ASTModifier
{
public:
	EraseMemoryStores(Block& _ast, set<YulString> const& _keys, Dialect const& _dialect):
		m_keys(_keys),
		m_dialect(_dialect)
	{
		(*this)(_ast);
	}

private:
	using ASTModifier::operator();

	void operator()(FunctionCall& _functionCall) override;

	set<YulString> const& m_keys;
	Dialect const& m_dialect;
};

void EraseMemoryStores::operator()(FunctionCall& _functionCall)
{
	ASTModifier::operator()(_functionCall);

	if (_functionCall.functionName.name == m_dialect.memoryStoreFunction({})->name)
		if (Identifier const* identifier = get_if<Identifier>(&_functionCall.arguments.at(0)))
			if (m_keys.count(identifier->name))
			{
				BuiltinFunction const* discard = m_dialect.discardFunction({});
				yulAssert(discard, "");
				FunctionCall popFunctionCall{
					_functionCall.location,
					Identifier{_functionCall.location, discard->name},
					{move(_functionCall.arguments.at(1))}
				};
				swap(_functionCall, popFunctionCall);
			}
}

}

// Currently only defines simple upper bounds for some builtins.
smtutil::Expression MemoryStoreRemover::encodeEVMBuiltin(
	evmasm::Instruction _instruction,
	std::vector<Expression> const& _arguments
)
{
	vector<smtutil::Expression> arguments = applyMap(
		_arguments,
		[this](yul::Expression const& _expr) { return encodeExpression(_expr); }
	);

	switch (_instruction)
	{
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

void MemoryStoreRemover::operator()(FunctionCall const& _functionCall)
{
	ASTWalker::operator()(_functionCall);

	// Tracks two different things:
	//
	// 1. Tracking all keys for `mstore(key, value)`. Provided that `key` is is an SSA Variable.
	//    Notice that assigning to location `[key, key + 32)` multiple times is not an issue.
	//    Also, we have no conditions on `value`.
	//
	// 2. We keep track of every builtin-function call that reads from memory. This way, we don't
	//    have to deal with specialized logic for reads inside function calls or control flow.
	if (_functionCall.functionName.name == m_dialect.memoryStoreFunction({})->name)
	{
		yulAssert(_functionCall.arguments.size() == 2, "");
		if (Identifier const* identifier = get_if<Identifier>(&_functionCall.arguments.at(0)))
			if (m_ssaVariables.count(identifier->name))
				m_memoryKeys.insert(identifier->name);
	}
	else if (auto instruction = toEVMInstruction(m_dialect, _functionCall.functionName.name))
		if (SemanticInformation::readsMemory(*instruction))
			m_memoryReads.emplace_back(&_functionCall);

}

void MemoryStoreRemover::operator()(VariableDeclaration const& _varDecl)
{
	ASTWalker::operator()(_varDecl);
	encodeVariableDeclaration(_varDecl);
}

void MemoryStoreRemover::encodeMemoryRead(
	FunctionCall const& _functionCall,
	smtutil::Expression _memoryLocation
)
{
	vector<smtutil::Expression> arguments = applyMap(
		_functionCall.arguments,
		[this](yul::Expression const& _expr) { return encodeExpression(_expr); }
	);

	optional<Instruction> instruction = toEVMInstruction(m_dialect, _functionCall.functionName.name);

	yulAssert(instruction.has_value(), "A function call that was not an EVM builtin.");
	yulAssert(SemanticInformation::readsMemory(*instruction), "");

	// Encode read to memory location [p, p + n)
	auto encodeRead = [&](auto const& p, auto const& n)
	{
		// Check if p + n can overflow.
		// EIP 1985 restricts the value of `msize` to be 2**32. Therefore, in practice `p + n`
		// cannot overflow. But assuming this can lead to incorrectly removing `mstore`, even though
		// such a contract cannot be executed in the first place because of gas reasons.
		m_solver->push();
		m_solver->addAssertion(p + n >= constantValue(bigint(1) << 256));
		auto result = m_solver->check({});
		m_solver->pop();

		// By skipping the encoding, `m_solver.check({})` inside `run(...)` will be satisfiable.
		if (result.first == CheckResult::UNSATISFIABLE)
		{
			m_solver->addAssertion(p <= _memoryLocation);
			m_solver->addAssertion(_memoryLocation < p + n);
		}
	};

	switch (*instruction)
	{
	case Instruction::CREATE:
		yulAssert(arguments.size() == 3, "");
		encodeRead(arguments.at(1), arguments.at(2));
		break;
	case Instruction::CREATE2:
		yulAssert(arguments.size() == 4, "");
		encodeRead(arguments.at(1), arguments.at(2));
		break;
	case Instruction::KECCAK256:
	case Instruction::RETURN:
	case Instruction::REVERT:
		yulAssert(arguments.size() == 2, "");
		encodeRead(arguments.at(0), arguments.at(1));
		break;
	case Instruction::MLOAD:
		yulAssert(arguments.size() == 1, "");
		encodeRead(arguments.at(0), constantValue(32));
		break;
	case Instruction::MSIZE:
		yulAssert(false, "The ast cannot contain msize.");
		break;
	case Instruction::LOG0:
		yulAssert(arguments.size() == 2, "");
		encodeRead(arguments.at(0), arguments.at(1));
		break;
	case Instruction::LOG1:
		yulAssert(arguments.size() == 3, "");
		encodeRead(arguments.at(0), arguments.at(1));
		break;
	case Instruction::LOG2:
		yulAssert(arguments.size() == 4, "");
		encodeRead(arguments.at(0), arguments.at(1));
		break;
	case Instruction::LOG3:
		yulAssert(arguments.size() == 5, "");
		encodeRead(arguments.at(0), arguments.at(1));
		break;
	case Instruction::LOG4:
		yulAssert(arguments.size() == 6, "");
		encodeRead(arguments.at(0), arguments.at(1));
		break;

	// Instructions that read and write from memory
	case Instruction::CODECOPY:
		yulAssert(arguments.size() == 3, "");
		encodeRead(arguments.at(1), arguments.at(2));
		break;
	case Instruction::CALL:
	case Instruction::CALLCODE:
	case Instruction::DELEGATECALL:
		yulAssert(arguments.size() == 7, "");
		encodeRead(arguments.at(3), arguments.at(4));
		break;
	case Instruction::STATICCALL:
		yulAssert(arguments.size() == 6, "");
		encodeRead(arguments.at(2), arguments.at(3));
		break;
	default:
		yulAssert(false, "");
		break;
	}
}

void MemoryStoreRemover::run(OptimiserStepContext& _context, Block& _ast)
{
	if (dynamic_cast<WasmDialect const*>(&_context.dialect))
		return;
	// We skip because the step is not guaranteed to preserve semantics of `msize`.
	if (MSizeFinder::containsMSize(_context.dialect, _ast))
		return;

	set<YulString> ssaVariables = SSAValueTracker::ssaVariables(_ast);

	MemoryStoreRemover m{ssaVariables, _context.dialect};
	m(_ast);

	set<YulString> memoryKeysToErase = move(m.m_memoryKeys);

	// A dummy variable, called `x`, that would be repeated during computations
	smtutil::Expression memoryLocation = m.newVariable();

	// For each memory location, say `key`, we try to prove that the memory hasn't been read from.
	// This is equivalent to showing the following:
	// For every builtin call that reads from `[a, b)`,
	// the system of equations:
	//
	//    a <= x < b
	//    key <= x < key + 32
	//
	// is infeasible. Here `x` is the dummy variable defined above.
	// The loop tries to check if such a system if not-infeasible.
	// If it's not-infeasible, then, the location cannot be removed.
	cxx20::erase_if(memoryKeysToErase, [&](auto const& _key)
	{
		// We check if `key + 32` can overflow. If possible, we cannot erase `mstore(key, ...)`
		auto keyAsExpr = m.m_variables.at(_key);
		m.m_solver->push();
		m.m_solver->addAssertion(
			keyAsExpr + m.constantValue(32) >= m.constantValue(bigint(1) << 256)
		);
		auto result1 = m.m_solver->check({});
		m.m_solver->pop();
		if (result1.first != CheckResult::UNSATISFIABLE)
			return true;

		for (auto const functionCall: m.m_memoryReads)
		{
			yulAssert(functionCall, "");
			m.m_solver->push();
			m.encodeMemoryRead(*functionCall, memoryLocation);

			m.m_solver->addAssertion(keyAsExpr <= memoryLocation);
			m.m_solver->addAssertion(memoryLocation < keyAsExpr + m.constantValue(32));

			auto result2 = m.m_solver->check({});
			m.m_solver->pop();
			// Notice that we check for not unsatisfiable, instead of == satisfiable.
			// Therefore, other SMT results such as `UNKNOWN`, `ERROR`, etc., would not set up the
			// key for erasing.
			if (result2.first != CheckResult::UNSATISFIABLE)
				return true;
		}

		return false;
	});

	EraseMemoryStores{_ast, memoryKeysToErase, _context.dialect};
}
