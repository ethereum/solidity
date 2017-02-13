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
 * @file SemanticInformation.cpp
 * @author Christian <c@ethdev.com>
 * @date 2015
 * Helper to provide semantic information about assembly items.
 */

#include <libevmasm/SemanticInformation.h>
#include <libevmasm/AssemblyItem.h>

using namespace std;
using namespace dev;
using namespace dev::eth;

bool SemanticInformation::breaksCSEAnalysisBlock(AssemblyItem const& _item)
{
	switch (_item.type())
	{
	default:
	case UndefinedItem:
	case Tag:
		return true;
	case Push:
	case PushString:
	case PushTag:
	case PushSub:
	case PushSubSize:
	case PushProgramSize:
	case PushData:
	case PushLibraryAddress:
		return false;
	case Operation:
	{
		if (isSwapInstruction(_item) || isDupInstruction(_item))
			return false;
		if (_item.instruction() == Instruction::GAS || _item.instruction() == Instruction::PC)
			return true; // GAS and PC assume a specific order of opcodes
		if (_item.instruction() == Instruction::MSIZE)
			return true; // msize is modified already by memory access, avoid that for now
		InstructionInfo info = instructionInfo(_item.instruction());
		if (_item.instruction() == Instruction::SSTORE)
			return false;
		if (_item.instruction() == Instruction::MSTORE)
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
	if (_item.type() != Operation)
		return false;
	switch (_item.instruction())
	{
	case Instruction::ADD:
	case Instruction::MUL:
	case Instruction::EQ:
	case Instruction::AND:
	case Instruction::OR:
	case Instruction::XOR:
		return true;
	default:
		return false;
	}
}

bool SemanticInformation::isDupInstruction(AssemblyItem const& _item)
{
	if (_item.type() != Operation)
		return false;
	return Instruction::DUP1 <= _item.instruction() && _item.instruction() <= Instruction::DUP16;
}

bool SemanticInformation::isSwapInstruction(AssemblyItem const& _item)
{
	if (_item.type() != Operation)
		return false;
	return Instruction::SWAP1 <= _item.instruction() && _item.instruction() <= Instruction::SWAP16;
}

bool SemanticInformation::isJumpInstruction(AssemblyItem const& _item)
{
	return _item == AssemblyItem(Instruction::JUMP) || _item == AssemblyItem(Instruction::JUMPI);
}

bool SemanticInformation::altersControlFlow(AssemblyItem const& _item)
{
	if (_item.type() != Operation)
		return false;
	switch (_item.instruction())
	{
	// note that CALL, CALLCODE and CREATE do not really alter the control flow, because we
	// continue on the next instruction
	case Instruction::JUMP:
	case Instruction::JUMPI:
	case Instruction::RETURN:
	case Instruction::SELFDESTRUCT:
	case Instruction::STOP:
	case Instruction::INVALID:
	case Instruction::REVERT:
		return true;
	default:
		return false;
	}
}


bool SemanticInformation::isDeterministic(AssemblyItem const& _item)
{
	if (_item.type() != Operation)
		return true;

	switch (_item.instruction())
	{
	case Instruction::CALL:
	case Instruction::CALLCODE:
	case Instruction::DELEGATECALL:
	case Instruction::CREATE:
	case Instruction::GAS:
	case Instruction::PC:
	case Instruction::MSIZE: // depends on previous writes and reads, not only on content
	case Instruction::BALANCE: // depends on previous calls
	case Instruction::EXTCODESIZE:
		return false;
	default:
		return true;
	}
}

bool SemanticInformation::invalidatesMemory(Instruction _instruction)
{
	switch (_instruction)
	{
	case Instruction::CALLDATACOPY:
	case Instruction::CODECOPY:
	case Instruction::EXTCODECOPY:
	case Instruction::MSTORE:
	case Instruction::MSTORE8:
	case Instruction::CALL:
	case Instruction::CALLCODE:
	case Instruction::DELEGATECALL:
		return true;
	default:
		return false;
	}
}

bool SemanticInformation::invalidatesStorage(Instruction _instruction)
{
	switch (_instruction)
	{
	case Instruction::CALL:
	case Instruction::CALLCODE:
	case Instruction::DELEGATECALL:
	case Instruction::CREATE:
	case Instruction::SSTORE:
		return true;
	default:
		return false;
	}
}
