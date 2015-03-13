/*
	This file is part of cpp-ethereum.

	cpp-ethereum is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	cpp-ethereum is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
*/
/**
 * @author Christian <c@ethdev.com>
 * @date 2014
 * Routines used by both the compiler and the expression compiler.
 */

#include <libsolidity/CompilerUtils.h>
#include <libsolidity/AST.h>
#include <libevmcore/Instruction.h>

using namespace std;

namespace dev
{
namespace solidity
{

const unsigned int CompilerUtils::dataStartOffset = 4;

unsigned CompilerUtils::loadFromMemory(
	unsigned _offset,
	Type const& _type,
	bool _fromCalldata,
	bool _padToWordBoundaries
)
{
	solAssert(_type.getCategory() != Type::Category::Array, "Unable to statically load dynamic type.");
	m_context << u256(_offset);
	return loadFromMemoryHelper(_type, _fromCalldata, _padToWordBoundaries);
}

void CompilerUtils::loadFromMemoryDynamic(
	Type const& _type,
	bool _fromCalldata,
	bool _padToWordBoundaries,
	bool _keepUpdatedMemoryOffset
)
{
	solAssert(_type.getCategory() != Type::Category::Array, "Arrays not yet implemented.");
	if (_keepUpdatedMemoryOffset)
		m_context << eth::Instruction::DUP1;
	unsigned numBytes = loadFromMemoryHelper(_type, _fromCalldata, _padToWordBoundaries);
	if (_keepUpdatedMemoryOffset)
	{
		// update memory counter
		for (unsigned i = 0; i < _type.getSizeOnStack(); ++i)
			m_context << eth::swapInstruction(1 + i);
		m_context << u256(numBytes) << eth::Instruction::ADD;
	}
}

unsigned CompilerUtils::storeInMemory(unsigned _offset, Type const& _type, bool _padToWordBoundaries)
{
	solAssert(_type.getCategory() != Type::Category::Array, "Unable to statically store dynamic type.");
	unsigned numBytes = prepareMemoryStore(_type, _padToWordBoundaries);
	if (numBytes > 0)
		m_context << u256(_offset) << eth::Instruction::MSTORE;
	return numBytes;
}

void CompilerUtils::storeInMemoryDynamic(Type const& _type, bool _padToWordBoundaries)
{
	if (_type.getCategory() == Type::Category::Array)
	{
		auto const& type = dynamic_cast<ArrayType const&>(_type);
		solAssert(type.isByteArray(), "Non byte arrays not yet implemented here.");

		if (type.getLocation() == ArrayType::Location::CallData)
		{
			// stack: target source_offset source_len
			m_context << eth::Instruction::DUP1 << eth::Instruction::DUP3 << eth::Instruction::DUP5
				// stack: target source_offset source_len source_len source_offset target
				<< eth::Instruction::CALLDATACOPY
				<< eth::Instruction::DUP3 << eth::Instruction::ADD
				<< eth::Instruction::SWAP2 << eth::Instruction::POP << eth::Instruction::POP;
		}
		else
		{
			solAssert(type.getLocation() == ArrayType::Location::Storage, "Memory arrays not yet implemented.");
			m_context << eth::Instruction::DUP1 << eth::Instruction::SLOAD;
			// stack here: memory_offset storage_offset length_bytes
			// jump to end if length is zero
			m_context << eth::Instruction::DUP1 << eth::Instruction::ISZERO;
			eth::AssemblyItem loopEnd = m_context.newTag();
			m_context.appendConditionalJumpTo(loopEnd);
			// compute memory end offset
			m_context << eth::Instruction::DUP3 << eth::Instruction::ADD << eth::Instruction::SWAP2;
			// actual array data is stored at SHA3(storage_offset)
			m_context << eth::Instruction::SWAP1;
			CompilerUtils(m_context).computeHashStatic();
			m_context << eth::Instruction::SWAP1;

			// stack here: memory_end_offset storage_data_offset memory_offset
			eth::AssemblyItem loopStart = m_context.newTag();
			m_context << loopStart
					  // load and store
					  << eth::Instruction::DUP2 << eth::Instruction::SLOAD
					  << eth::Instruction::DUP2 << eth::Instruction::MSTORE
					  // increment storage_data_offset by 1
					  << eth::Instruction::SWAP1 << u256(1) << eth::Instruction::ADD
					  // increment memory offset by 32
					  << eth::Instruction::SWAP1 << u256(32) << eth::Instruction::ADD
					  // check for loop condition
					  << eth::Instruction::DUP1 << eth::Instruction::DUP4 << eth::Instruction::GT;
			m_context.appendConditionalJumpTo(loopStart);
			m_context << loopEnd << eth::Instruction::POP << eth::Instruction::POP;
		}
	}
	else
	{
		unsigned numBytes = prepareMemoryStore(_type, _padToWordBoundaries);
		if (numBytes > 0)
		{
			solAssert(_type.getSizeOnStack() == 1, "Memory store of types with stack size != 1 not implemented.");
			m_context << eth::Instruction::DUP2 << eth::Instruction::MSTORE;
			m_context << u256(numBytes) << eth::Instruction::ADD;
		}
	}
}

void CompilerUtils::moveToStackVariable(VariableDeclaration const& _variable)
{
	unsigned const stackPosition = m_context.baseToCurrentStackOffset(m_context.getBaseStackOffsetOfVariable(_variable));
	unsigned const size = _variable.getType()->getSizeOnStack();
	solAssert(stackPosition >= size, "Variable size and position mismatch.");
	// move variable starting from its top end in the stack
	if (stackPosition - size + 1 > 16)
		BOOST_THROW_EXCEPTION(CompilerError() << errinfo_sourceLocation(_variable.getLocation())
											  << errinfo_comment("Stack too deep."));
	for (unsigned i = 0; i < size; ++i)
		m_context << eth::swapInstruction(stackPosition - size + 1) << eth::Instruction::POP;
}

void CompilerUtils::copyToStackTop(unsigned _stackDepth, unsigned _itemSize)
{
	solAssert(_stackDepth <= 16, "Stack too deep.");
	for (unsigned i = 0; i < _itemSize; ++i)
		m_context << eth::dupInstruction(_stackDepth);
}

void CompilerUtils::popStackElement(Type const& _type)
{
	unsigned const size = _type.getSizeOnStack();
	for (unsigned i = 0; i < size; ++i)
		m_context << eth::Instruction::POP;
}

unsigned CompilerUtils::getSizeOnStack(vector<shared_ptr<Type const>> const& _variableTypes)
{
	unsigned size = 0;
	for (shared_ptr<Type const> const& type: _variableTypes)
		size += type->getSizeOnStack();
	return size;
}

void CompilerUtils::computeHashStatic(Type const& _type, bool _padToWordBoundaries)
{
	unsigned length = storeInMemory(0, _type, _padToWordBoundaries);
	m_context << u256(length) << u256(0) << eth::Instruction::SHA3;
}

unsigned CompilerUtils::loadFromMemoryHelper(Type const& _type, bool _fromCalldata, bool _padToWordBoundaries)
{
	unsigned numBytes = _type.getCalldataEncodedSize(_padToWordBoundaries);
	bool leftAligned = _type.getCategory() == Type::Category::FixedBytes;
	if (numBytes == 0)
		m_context << eth::Instruction::POP << u256(0);
	else
	{
		solAssert(numBytes <= 32, "Static memory load of more than 32 bytes requested.");
		m_context << (_fromCalldata ? eth::Instruction::CALLDATALOAD : eth::Instruction::MLOAD);
		if (numBytes != 32)
		{
			// add leading or trailing zeros by dividing/multiplying depending on alignment
			u256 shiftFactor = u256(1) << ((32 - numBytes) * 8);
			m_context << shiftFactor << eth::Instruction::SWAP1 << eth::Instruction::DIV;
			if (leftAligned)
				m_context << shiftFactor << eth::Instruction::MUL;
		}
	}

	return numBytes;
}


unsigned CompilerUtils::prepareMemoryStore(Type const& _type, bool _padToWordBoundaries) const
{
	unsigned numBytes = _type.getCalldataEncodedSize(_padToWordBoundaries);
	bool leftAligned = _type.getCategory() == Type::Category::FixedBytes;
	if (numBytes == 0)
		m_context << eth::Instruction::POP;
	else
	{
		solAssert(numBytes <= 32, "Memory store of more than 32 bytes requested.");
		if (numBytes != 32 && !leftAligned && !_padToWordBoundaries)
			// shift the value accordingly before storing
			m_context << (u256(1) << ((32 - numBytes) * 8)) << eth::Instruction::MUL;
	}
	return numBytes;
}

}
}
