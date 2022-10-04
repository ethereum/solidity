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
 * @file SemanticInformation.cpp
 * @author Christian <c@ethdev.com>
 * @date 2015
 * Helper to provide semantic information about assembly items.
 */

#include <libevmasm/SemanticInformation.h>
#include <libevmasm/AssemblyItem.h>

using namespace std;
using namespace solidity;
using namespace solidity::evmasm;

vector<SemanticInformation::Operation> SemanticInformation::readWriteOperations(InternalInstruction _instruction)
{
	switch (_instruction)
	{
	case InternalInstruction::SSTORE:
	case InternalInstruction::SLOAD:
	{
		assertThrow(memory(_instruction) == Effect::None, OptimizerException, "");
		assertThrow(storage(_instruction) != Effect::None, OptimizerException, "");
		Operation op;
		op.effect = storage(_instruction);
		op.location = Location::Storage;
		op.startParameter = 0;
		// We know that exactly one slot is affected.
		op.lengthConstant = 1;
		return {op};
	}
	case InternalInstruction::MSTORE:
	case InternalInstruction::MSTORE8:
	case InternalInstruction::MLOAD:
	{
		assertThrow(memory(_instruction) != Effect::None, OptimizerException, "");
		assertThrow(storage(_instruction) == Effect::None, OptimizerException, "");
		Operation op;
		op.effect = memory(_instruction);
		op.location = Location::Memory;
		op.startParameter = 0;
		if (_instruction == InternalInstruction::MSTORE || _instruction == InternalInstruction::MLOAD)
			op.lengthConstant = 32;
		else if (_instruction == InternalInstruction::MSTORE8)
			op.lengthConstant = 1;

		return {op};
	}
	case InternalInstruction::REVERT:
	case InternalInstruction::RETURN:
	case InternalInstruction::KECCAK256:
	case InternalInstruction::LOG0:
	case InternalInstruction::LOG1:
	case InternalInstruction::LOG2:
	case InternalInstruction::LOG3:
	case InternalInstruction::LOG4:
	{
		assertThrow(storage(_instruction) == Effect::None, OptimizerException, "");
		assertThrow(memory(_instruction) == Effect::Read, OptimizerException, "");
		Operation op;
		op.effect = memory(_instruction);
		op.location = Location::Memory;
		op.startParameter = 0;
		op.lengthParameter = 1;
		return {op};
	}
	case InternalInstruction::EXTCODECOPY:
	{
		assertThrow(memory(_instruction) == Effect::Write, OptimizerException, "");
		assertThrow(storage(_instruction) == Effect::None, OptimizerException, "");
		Operation op;
		op.effect = memory(_instruction);
		op.location = Location::Memory;
		op.startParameter = 1;
		op.lengthParameter = 3;
		return {op};
	}
	case InternalInstruction::CODECOPY:
	case InternalInstruction::CALLDATACOPY:
	case InternalInstruction::RETURNDATACOPY:
	{
		assertThrow(memory(_instruction) == Effect::Write, OptimizerException, "");
		assertThrow(storage(_instruction) == Effect::None, OptimizerException, "");
		Operation op;
		op.effect = memory(_instruction);
		op.location = Location::Memory;
		op.startParameter = 0;
		op.lengthParameter = 2;
		return {op};
	}
	case InternalInstruction::STATICCALL:
	case InternalInstruction::CALL:
	case InternalInstruction::CALLCODE:
	case InternalInstruction::DELEGATECALL:
	{
		size_t paramCount = static_cast<size_t>(instructionInfo(_instruction).args);
		vector<Operation> operations{
			Operation{Location::Memory, Effect::Read, paramCount - 4, paramCount - 3, {}},
			Operation{Location::Storage, Effect::Read, {}, {}, {}}
		};
		if (_instruction != InternalInstruction::STATICCALL)
			operations.emplace_back(Operation{Location::Storage, Effect::Write, {}, {}, {}});
		operations.emplace_back(Operation{
			Location::Memory,
			Effect::Write,
			paramCount - 2,
			// Length is in paramCount - 1, but it is only a max length,
			// there is no guarantee that the full area is written to.
			{},
			{}
		});
		return operations;
	}
	case InternalInstruction::CREATE:
	case InternalInstruction::CREATE2:
		return vector<Operation>{
			Operation{
				Location::Memory,
				Effect::Read,
				1,
				2,
				{}
			},
			Operation{Location::Storage, Effect::Read, {}, {}, {}},
			Operation{Location::Storage, Effect::Write, {}, {}, {}}
		};
	case InternalInstruction::MSIZE:
		// This is just to satisfy the assert below.
		return vector<Operation>{};
	default:
		assertThrow(storage(_instruction) == None && memory(_instruction) == None, AssemblyException, "");
	}
	return {};
}

bool SemanticInformation::breaksCSEAnalysisBlock(AssemblyItem const& _item, bool _msizeImportant)
{
	switch (_item.type())
	{
	default:
	case UndefinedItem:
	case Tag:
	case PushDeployTimeAddress:
	case AssignImmutable:
	case VerbatimBytecode:
		return true;
	case Push:
	case PushTag:
	case PushSub:
	case PushSubSize:
	case PushProgramSize:
	case PushData:
	case PushLibraryAddress:
	case PushImmutable:
		return false;
	case evmasm::Operation:
	{
		if (isSwapInstruction(_item) || isDupInstruction(_item))
			return false;
		if (_item.instruction() == InternalInstruction::GAS || _item.instruction() == InternalInstruction::PC)
			return true; // GAS and PC assume a specific order of opcodes
		if (_item.instruction() == InternalInstruction::MSIZE)
			return true; // msize is modified already by memory access, avoid that for now
		InstructionInfo info = instructionInfo(_item.instruction());
		if (_item.instruction() == InternalInstruction::SSTORE)
			return false;
		if (_item.instruction() == InternalInstruction::MSTORE)
			return false;
		if (!_msizeImportant && (
			_item.instruction() == InternalInstruction::MLOAD ||
			_item.instruction() == InternalInstruction::KECCAK256
		))
			return false;
		//@todo: We do not handle the following memory instructions for now:
		// calldatacopy, codecopy, extcodecopy, mstore8,
		// msize (note that msize also depends on memory read access)

		// the second requirement will be lifted once it is implemented
		return info.sideEffects || info.args > 2;
	}
	}
}

bool SemanticInformation::isCommutativeOperation(AssemblyItem const& _item)
{
	if (_item.type() != evmasm::Operation)
		return false;
	switch (_item.instruction())
	{
	case InternalInstruction::ADD:
	case InternalInstruction::MUL:
	case InternalInstruction::EQ:
	case InternalInstruction::AND:
	case InternalInstruction::OR:
	case InternalInstruction::XOR:
		return true;
	default:
		return false;
	}
}

bool SemanticInformation::isDupInstruction(AssemblyItem const& _item)
{
	if (_item.type() != evmasm::Operation)
		return false;
	return evmasm::isDupInstruction(_item.instruction());
}

bool SemanticInformation::isSwapInstruction(AssemblyItem const& _item)
{
	if (_item.type() != evmasm::Operation)
		return false;
	return evmasm::isSwapInstruction(_item.instruction());
}

bool SemanticInformation::isJumpInstruction(AssemblyItem const& _item)
{
	return _item == InternalInstruction::JUMP || _item == InternalInstruction::JUMPI;
}

bool SemanticInformation::altersControlFlow(AssemblyItem const& _item)
{
	if (_item.type() != evmasm::Operation)
		return false;
	switch (_item.instruction())
	{
	// note that CALL, CALLCODE and CREATE do not really alter the control flow, because we
	// continue on the next instruction
	case InternalInstruction::JUMP:
	case InternalInstruction::JUMPI:
	case InternalInstruction::RETURN:
	case InternalInstruction::SELFDESTRUCT:
	case InternalInstruction::STOP:
	case InternalInstruction::INVALID:
	case InternalInstruction::REVERT:
		return true;
	default:
		return false;
	}
}

bool SemanticInformation::terminatesControlFlow(InternalInstruction _instruction)
{
	switch (_instruction)
	{
	case InternalInstruction::RETURN:
	case InternalInstruction::SELFDESTRUCT:
	case InternalInstruction::STOP:
	case InternalInstruction::INVALID:
	case InternalInstruction::REVERT:
		return true;
	default:
		return false;
	}
}

bool SemanticInformation::reverts(InternalInstruction _instruction)
{
	switch (_instruction)
	{
		case InternalInstruction::INVALID:
		case InternalInstruction::REVERT:
			return true;
		default:
			return false;
	}
}

bool SemanticInformation::isDeterministic(AssemblyItem const& _item)
{
	assertThrow(_item.type() != VerbatimBytecode, AssemblyException, "");

	if (_item.type() != evmasm::Operation)
		return true;

	switch (_item.instruction())
	{
	case InternalInstruction::CALL:
	case InternalInstruction::CALLCODE:
	case InternalInstruction::DELEGATECALL:
	case InternalInstruction::STATICCALL:
	case InternalInstruction::CREATE:
	case InternalInstruction::CREATE2:
	case InternalInstruction::GAS:
	case InternalInstruction::PC:
	case InternalInstruction::MSIZE: // depends on previous writes and reads, not only on content
	case InternalInstruction::BALANCE: // depends on previous calls
	case InternalInstruction::SELFBALANCE: // depends on previous calls
	case InternalInstruction::EXTCODESIZE:
	case InternalInstruction::EXTCODEHASH:
	case InternalInstruction::RETURNDATACOPY: // depends on previous calls
	case InternalInstruction::RETURNDATASIZE:
		return false;
	default:
		return true;
	}
}

bool SemanticInformation::movable(InternalInstruction _instruction)
{
	// These are not really functional.
	if (isDupInstruction(_instruction) || isSwapInstruction(_instruction))
		return false;
	InstructionInfo info = instructionInfo(_instruction);
	if (info.sideEffects)
		return false;
	switch (_instruction)
	{
	case InternalInstruction::KECCAK256:
	case InternalInstruction::BALANCE:
	case InternalInstruction::SELFBALANCE:
	case InternalInstruction::EXTCODESIZE:
	case InternalInstruction::EXTCODEHASH:
	case InternalInstruction::RETURNDATASIZE:
	case InternalInstruction::SLOAD:
	case InternalInstruction::PC:
	case InternalInstruction::MSIZE:
	case InternalInstruction::GAS:
		return false;
	default:
		return true;
	}
	return true;
}

bool SemanticInformation::canBeRemoved(InternalInstruction _instruction)
{
	// These are not really functional.
	assertThrow(!isDupInstruction(_instruction) && !isSwapInstruction(_instruction), AssemblyException, "");

	return !instructionInfo(_instruction).sideEffects;
}

bool SemanticInformation::canBeRemovedIfNoMSize(InternalInstruction _instruction)
{
	if (_instruction == InternalInstruction::KECCAK256 || _instruction == InternalInstruction::MLOAD)
		return true;
	else
		return canBeRemoved(_instruction);
}

SemanticInformation::Effect SemanticInformation::memory(InternalInstruction _instruction)
{
	switch (_instruction)
	{
	case InternalInstruction::CALLDATACOPY:
	case InternalInstruction::CODECOPY:
	case InternalInstruction::EXTCODECOPY:
	case InternalInstruction::RETURNDATACOPY:
	case InternalInstruction::MSTORE:
	case InternalInstruction::MSTORE8:
	case InternalInstruction::CALL:
	case InternalInstruction::CALLCODE:
	case InternalInstruction::DELEGATECALL:
	case InternalInstruction::STATICCALL:
		return SemanticInformation::Write;

	case InternalInstruction::CREATE:
	case InternalInstruction::CREATE2:
	case InternalInstruction::KECCAK256:
	case InternalInstruction::MLOAD:
	case InternalInstruction::MSIZE:
	case InternalInstruction::RETURN:
	case InternalInstruction::REVERT:
	case InternalInstruction::LOG0:
	case InternalInstruction::LOG1:
	case InternalInstruction::LOG2:
	case InternalInstruction::LOG3:
	case InternalInstruction::LOG4:
		return SemanticInformation::Read;

	default:
		return SemanticInformation::None;
	}
}

bool SemanticInformation::movableApartFromEffects(InternalInstruction _instruction)
{
	switch (_instruction)
	{
	case InternalInstruction::EXTCODEHASH:
	case InternalInstruction::EXTCODESIZE:
	case InternalInstruction::RETURNDATASIZE:
	case InternalInstruction::BALANCE:
	case InternalInstruction::SELFBALANCE:
	case InternalInstruction::SLOAD:
	case InternalInstruction::KECCAK256:
	case InternalInstruction::MLOAD:
		return true;

	default:
		return movable(_instruction);
	}
}

SemanticInformation::Effect SemanticInformation::storage(InternalInstruction _instruction)
{
	switch (_instruction)
	{
	case InternalInstruction::CALL:
	case InternalInstruction::CALLCODE:
	case InternalInstruction::DELEGATECALL:
	case InternalInstruction::CREATE:
	case InternalInstruction::CREATE2:
	case InternalInstruction::SSTORE:
		return SemanticInformation::Write;

	case InternalInstruction::SLOAD:
	case InternalInstruction::STATICCALL:
		return SemanticInformation::Read;

	default:
		return SemanticInformation::None;
	}
}

SemanticInformation::Effect SemanticInformation::otherState(InternalInstruction _instruction)
{
	switch (_instruction)
	{
	case InternalInstruction::CALL:
	case InternalInstruction::CALLCODE:
	case InternalInstruction::DELEGATECALL:
	case InternalInstruction::CREATE:
	case InternalInstruction::CREATE2:
	case InternalInstruction::SELFDESTRUCT:
	case InternalInstruction::STATICCALL: // because it can affect returndatasize
		// Strictly speaking, log0, .., log4 writes to the state, but the EVM cannot read it, so they
		// are just marked as having 'other side effects.'
		return SemanticInformation::Write;

	case InternalInstruction::EXTCODESIZE:
	case InternalInstruction::EXTCODEHASH:
	case InternalInstruction::RETURNDATASIZE:
	case InternalInstruction::BALANCE:
	case InternalInstruction::SELFBALANCE:
	case InternalInstruction::RETURNDATACOPY:
	case InternalInstruction::EXTCODECOPY:
		// PC and GAS are specifically excluded here. Instructions such as CALLER, CALLVALUE,
		// ADDRESS are excluded because they cannot change during execution.
		return SemanticInformation::Read;

	default:
		return SemanticInformation::None;
	}
}

bool SemanticInformation::invalidInPureFunctions(InternalInstruction _instruction)
{
	switch (_instruction)
	{
	case InternalInstruction::ADDRESS:
	case InternalInstruction::SELFBALANCE:
	case InternalInstruction::BALANCE:
	case InternalInstruction::ORIGIN:
	case InternalInstruction::CALLER:
	case InternalInstruction::CALLVALUE:
	case InternalInstruction::CHAINID:
	case InternalInstruction::BASEFEE:
	case InternalInstruction::GAS:
	case InternalInstruction::GASPRICE:
	case InternalInstruction::EXTCODESIZE:
	case InternalInstruction::EXTCODECOPY:
	case InternalInstruction::EXTCODEHASH:
	case InternalInstruction::BLOCKHASH:
	case InternalInstruction::COINBASE:
	case InternalInstruction::TIMESTAMP:
	case InternalInstruction::NUMBER:
	case InternalInstruction::DIFFICULTY:
	case InternalInstruction::GASLIMIT:
	case InternalInstruction::STATICCALL:
	case InternalInstruction::SLOAD:
		return true;
	default:
		break;
	}
	return invalidInViewFunctions(_instruction);
}

bool SemanticInformation::invalidInViewFunctions(InternalInstruction _instruction)
{
	switch (_instruction)
	{
	case InternalInstruction::SSTORE:
	case InternalInstruction::JUMP:
	case InternalInstruction::JUMPI:
	case InternalInstruction::LOG0:
	case InternalInstruction::LOG1:
	case InternalInstruction::LOG2:
	case InternalInstruction::LOG3:
	case InternalInstruction::LOG4:
	case InternalInstruction::CREATE:
	case InternalInstruction::CALL:
	case InternalInstruction::CALLCODE:
	case InternalInstruction::DELEGATECALL:
	case InternalInstruction::CREATE2:
	case InternalInstruction::SELFDESTRUCT:
		return true;
	default:
		break;
	}
	return false;
}
