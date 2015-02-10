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

unsigned CompilerUtils::loadFromMemory(unsigned _offset, unsigned _bytes, bool _leftAligned,
									   bool _fromCalldata, bool _padToWordBoundaries)
{
	if (_bytes == 0)
	{
		m_context << u256(0);
		return 0;
	}
	eth::Instruction load = _fromCalldata ? eth::Instruction::CALLDATALOAD : eth::Instruction::MLOAD;
	solAssert(_bytes <= 32, "Memory load of more than 32 bytes requested.");
	if (_bytes == 32 || _padToWordBoundaries)
	{
		m_context << u256(_offset) << load;
		return 32;
	}
	else
	{
		// load data and add leading or trailing zeros by dividing/multiplying depending on alignment
		u256 shiftFactor = u256(1) << ((32 - _bytes) * 8);
		m_context << shiftFactor;
		if (_leftAligned)
			m_context << eth::Instruction::DUP1;
		m_context << u256(_offset) << load << eth::Instruction::DIV;
		if (_leftAligned)
			m_context << eth::Instruction::MUL;
		return _bytes;
	}
}

unsigned CompilerUtils::storeInMemory(unsigned _offset, Type const& _type, bool _padToWordBoundaries)
{
	unsigned numBytes = prepareMemoryStore(_type, _padToWordBoundaries);
	if (numBytes > 0)
		m_context << u256(_offset) << eth::Instruction::MSTORE;
	return numBytes;
}

void CompilerUtils::storeInMemoryDynamic(Type const& _type, bool _padToWordBoundaries)
{
	unsigned numBytes = prepareMemoryStore(_type, _padToWordBoundaries);
	if (numBytes > 0)
	{
		m_context << eth::Instruction::DUP2 << eth::Instruction::MSTORE;
		m_context << u256(numBytes) << eth::Instruction::ADD;
	}
}

void CompilerUtils::moveToStackVariable(VariableDeclaration const& _variable)
{
	unsigned const stackPosition = m_context.baseToCurrentStackOffset(m_context.getBaseStackOffsetOfVariable(_variable));
	unsigned const size = _variable.getType()->getSizeOnStack();
	// move variable starting from its top end in the stack
	if (stackPosition - size + 1 > 16)
		BOOST_THROW_EXCEPTION(CompilerError() << errinfo_sourceLocation(_variable.getLocation())
											  << errinfo_comment("Stack too deep."));
	for (unsigned i = 0; i < size; ++i)
		m_context << eth::swapInstruction(stackPosition - size + 1) << eth::Instruction::POP;
}

void CompilerUtils::copyToStackTop(unsigned _stackDepth, Type const& _type)
{
	if (_stackDepth > 16)
		BOOST_THROW_EXCEPTION(CompilerError() << errinfo_comment("Stack too deep."));
	unsigned const size = _type.getSizeOnStack();
	for (unsigned i = 0; i < size; ++i)
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

unsigned CompilerUtils::prepareMemoryStore(Type const& _type, bool _padToWordBoundaries)
{
	unsigned _encodedSize = _type.getCalldataEncodedSize();
	unsigned numBytes = _padToWordBoundaries ? getPaddedSize(_encodedSize) : _encodedSize;
	bool leftAligned = _type.getCategory() == Type::Category::String;
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
