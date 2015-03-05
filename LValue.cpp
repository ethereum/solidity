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
 * @date 2015
 * LValues for use in the expresison compiler.
 */

#include <libsolidity/LValue.h>
#include <libevmcore/Instruction.h>
#include <libsolidity/Types.h>
#include <libsolidity/AST.h>
#include <libsolidity/CompilerUtils.h>

using namespace std;
using namespace dev;
using namespace solidity;


StackVariable::StackVariable(CompilerContext& _compilerContext, Declaration const& _declaration):
	LValue(_compilerContext, *_declaration.getType()),
	m_baseStackOffset(m_context.getBaseStackOffsetOfVariable(_declaration)),
	m_size(m_dataType.getSizeOnStack())
{
}

void StackVariable::retrieveValue(SourceLocation const& _location, bool) const
{
	unsigned stackPos = m_context.baseToCurrentStackOffset(m_baseStackOffset);
	if (stackPos >= 15) //@todo correct this by fetching earlier or moving to memory
		BOOST_THROW_EXCEPTION(CompilerError()
			<< errinfo_sourceLocation(_location) << errinfo_comment("Stack too deep."));
	for (unsigned i = 0; i < m_size; ++i)
		m_context << eth::dupInstruction(stackPos + 1);
}

void StackVariable::storeValue(Type const&, SourceLocation const& _location, bool _move) const
{
	unsigned stackDiff = m_context.baseToCurrentStackOffset(m_baseStackOffset) - m_size + 1;
	if (stackDiff > 16)
		BOOST_THROW_EXCEPTION(CompilerError()
			<< errinfo_sourceLocation(_location) << errinfo_comment("Stack too deep."));
	else if (stackDiff > 0)
		for (unsigned i = 0; i < m_size; ++i)
			m_context << eth::swapInstruction(stackDiff) << eth::Instruction::POP;
	if (!_move)
		retrieveValue(_location);
}

void StackVariable::setToZero(SourceLocation const& _location, bool) const
{
	unsigned stackDiff = m_context.baseToCurrentStackOffset(m_baseStackOffset);
	if (stackDiff > 16)
		BOOST_THROW_EXCEPTION(CompilerError()
			<< errinfo_sourceLocation(_location) << errinfo_comment("Stack too deep."));
	solAssert(stackDiff >= m_size - 1, "");
	for (unsigned i = 0; i < m_size; ++i)
		m_context << u256(0) << eth::swapInstruction(stackDiff + 1 - i)
			<< eth::Instruction::POP;
}


StorageItem::StorageItem(CompilerContext& _compilerContext, Declaration const& _declaration):
	StorageItem(_compilerContext, *_declaration.getType())
{
	m_context << m_context.getStorageLocationOfVariable(_declaration);
}

StorageItem::StorageItem(CompilerContext& _compilerContext, Type const& _type):
	LValue(_compilerContext, _type)
{
	if (m_dataType.isValueType())
	{
		solAssert(m_dataType.getStorageSize() == m_dataType.getSizeOnStack(), "");
		solAssert(m_dataType.getStorageSize() <= numeric_limits<unsigned>::max(),
			"The storage size of " + m_dataType.toString() + " should fit in an unsigned");
		m_size = unsigned(m_dataType.getStorageSize());
	}
	else
		m_size = 0; // unused
}

void StorageItem::retrieveValue(SourceLocation const&, bool _remove) const
{
	if (!m_dataType.isValueType())
		return; // no distinction between value and reference for non-value types
	if (!_remove)
		m_context << eth::Instruction::DUP1;
	if (m_size == 1)
		m_context << eth::Instruction::SLOAD;
	else
		for (unsigned i = 0; i < m_size; ++i)
		{
			m_context << eth::Instruction::DUP1 << eth::Instruction::SLOAD << eth::Instruction::SWAP1;
			if (i + 1 < m_size)
				m_context << u256(1) << eth::Instruction::ADD;
			else
				m_context << eth::Instruction::POP;
		}
}

void StorageItem::storeValue(Type const& _sourceType, SourceLocation const& _location, bool _move) const
{
	// stack layout: value value ... value target_ref
	if (m_dataType.isValueType())
	{
		if (!_move) // copy values
		{
			if (m_size + 1 > 16)
				BOOST_THROW_EXCEPTION(CompilerError()
					<< errinfo_sourceLocation(_location) << errinfo_comment("Stack too deep."));
			for (unsigned i = 0; i < m_size; ++i)
				m_context << eth::dupInstruction(m_size + 1) << eth::Instruction::SWAP1;
		}
		if (m_size > 1) // store high index value first
			m_context << u256(m_size - 1) << eth::Instruction::ADD;
		for (unsigned i = 0; i < m_size; ++i)
		{
			if (i + 1 >= m_size)
				m_context << eth::Instruction::SSTORE;
			else
				// stack here: value value ... value value (target_ref+offset)
				m_context << eth::Instruction::SWAP1 << eth::Instruction::DUP2
					<< eth::Instruction::SSTORE
					<< u256(1) << eth::Instruction::SWAP1 << eth::Instruction::SUB;
		}
	}
	else
	{
		solAssert(_sourceType.getCategory() == m_dataType.getCategory(),
			"Wrong type conversation for assignment.");
		if (m_dataType.getCategory() == Type::Category::Array)
		{
			ArrayUtils(m_context).copyArrayToStorage(
						dynamic_cast<ArrayType const&>(m_dataType),
						dynamic_cast<ArrayType const&>(_sourceType));
			if (_move)
				m_context << eth::Instruction::POP;
		}
		else if (m_dataType.getCategory() == Type::Category::Struct)
		{
			// stack layout: source_ref target_ref
			auto const& structType = dynamic_cast<StructType const&>(m_dataType);
			solAssert(structType == _sourceType, "Struct assignment with conversion.");
			for (auto const& member: structType.getMembers())
			{
				// assign each member that is not a mapping
				TypePointer const& memberType = member.second;
				if (memberType->getCategory() == Type::Category::Mapping)
					continue;
				m_context << structType.getStorageOffsetOfMember(member.first)
					<< eth::Instruction::DUP3 << eth::Instruction::DUP2 << eth::Instruction::ADD;
				// stack: source_ref target_ref member_offset source_member_ref
				StorageItem(m_context, *memberType).retrieveValue(_location, true);
				// stack: source_ref target_ref member_offset source_value...
				m_context << eth::dupInstruction(2 + memberType->getSizeOnStack())
					<< eth::dupInstruction(2 + memberType->getSizeOnStack()) << eth::Instruction::ADD;
				// stack: source_ref target_ref member_offset source_value... target_member_ref
				StorageItem(m_context, *memberType).storeValue(*memberType, _location, true);
				m_context << eth::Instruction::POP;
			}
			if (_move)
				m_context << eth::Instruction::POP;
			else
				m_context << eth::Instruction::SWAP1;
			m_context << eth::Instruction::POP;
		}
		else
			BOOST_THROW_EXCEPTION(InternalCompilerError()
				<< errinfo_sourceLocation(_location) << errinfo_comment("Invalid non-value type for assignment."));
	}
}

void StorageItem::setToZero(SourceLocation const&, bool _removeReference) const
{
	if (m_dataType.getCategory() == Type::Category::Array)
	{
		if (!_removeReference)
			m_context << eth::Instruction::DUP1;
		ArrayUtils(m_context).clearArray(dynamic_cast<ArrayType const&>(m_dataType));
	}
	else if (m_dataType.getCategory() == Type::Category::Struct)
	{
		// stack layout: ref
		auto const& structType = dynamic_cast<StructType const&>(m_dataType);
		for (auto const& member: structType.getMembers())
		{
			// zero each member that is not a mapping
			TypePointer const& memberType = member.second;
			if (memberType->getCategory() == Type::Category::Mapping)
				continue;
			m_context << structType.getStorageOffsetOfMember(member.first)
				<< eth::Instruction::DUP2 << eth::Instruction::ADD;
			StorageItem(m_context, *memberType).setToZero();
		}
		if (_removeReference)
			m_context << eth::Instruction::POP;
	}
	else
	{
		if (m_size == 0 && _removeReference)
			m_context << eth::Instruction::POP;
		else if (m_size == 1)
			m_context
				<< u256(0) << (_removeReference ? eth::Instruction::SWAP1 : eth::Instruction::DUP2)
				<< eth::Instruction::SSTORE;
		else
		{
			if (!_removeReference)
				m_context << eth::Instruction::DUP1;
			for (unsigned i = 0; i < m_size; ++i)
				if (i + 1 >= m_size)
					m_context << u256(0) << eth::Instruction::SWAP1 << eth::Instruction::SSTORE;
				else
					m_context << u256(0) << eth::Instruction::DUP2 << eth::Instruction::SSTORE
						<< u256(1) << eth::Instruction::ADD;
		}
	}
}

/// Used in StorageByteArrayElement
static IntegerType byteType(8, IntegerType::Modifier::Hash);

StorageByteArrayElement::StorageByteArrayElement(CompilerContext& _compilerContext):
	LValue(_compilerContext, byteType)
{
}

void StorageByteArrayElement::retrieveValue(SourceLocation const&, bool _remove) const
{
	// stack: ref byte_number
	if (_remove)
		m_context << eth::Instruction::SWAP1 << eth::Instruction::SLOAD
			<< eth::Instruction::SWAP1 << eth::Instruction::BYTE;
	else
		m_context << eth::Instruction::DUP2 << eth::Instruction::SLOAD
			<< eth::Instruction::DUP2 << eth::Instruction::BYTE;
}

void StorageByteArrayElement::storeValue(Type const&, SourceLocation const&, bool _move) const
{
	//@todo optimize this

	// stack: value ref byte_number
	m_context << u256(31) << eth::Instruction::SUB << u256(0x100) << eth::Instruction::EXP;
	// stack: value ref (1<<(8*(31-byte_number)))
	m_context << eth::Instruction::DUP2 << eth::Instruction::SLOAD;
	// stack: value ref (1<<(8*(31-byte_number))) old_full_value
	// clear byte in old value
	m_context << eth::Instruction::DUP2 << u256(0xff) << eth::Instruction::MUL
		<< eth::Instruction::NOT << eth::Instruction::AND;
	// stack: value ref (1<<(32-byte_number)) old_full_value_with_cleared_byte
	m_context << eth::Instruction::SWAP1 << eth::Instruction::DUP4 << eth::Instruction::MUL
		<< eth::Instruction::OR;
	// stack: value ref new_full_value
	m_context << eth::Instruction::SWAP1 << eth::Instruction::SSTORE;
	if (_move)
		m_context << eth::Instruction::POP;
}

void StorageByteArrayElement::setToZero(SourceLocation const&, bool _removeReference) const
{
	// stack: ref byte_number
	if (!_removeReference)
		m_context << eth::Instruction::SWAP1 << eth::Instruction::DUP2;
	m_context << u256(31) << eth::Instruction::SUB << u256(0x100) << eth::Instruction::EXP;
	// stack: ref (1<<(8*(31-byte_number)))
	m_context << eth::Instruction::DUP2 << eth::Instruction::SLOAD;
	// stack: ref (1<<(8*(31-byte_number))) old_full_value
	// clear byte in old value
	m_context << eth::Instruction::SWAP1 << u256(0xff) << eth::Instruction::MUL << eth::Instruction::AND;
	// stack: ref old_full_value_with_cleared_byte
	m_context << eth::Instruction::SWAP1 << eth::Instruction::SSTORE;
	if (!_removeReference)
		m_context << eth::Instruction::SWAP1;
	else
		m_context << eth::Instruction::POP;
}

StorageArrayLength::StorageArrayLength(CompilerContext& _compilerContext, const ArrayType& _arrayType):
	LValue(_compilerContext, *_arrayType.getMemberType("length")),
	m_arrayType(_arrayType)
{
	solAssert(m_arrayType.isDynamicallySized(), "");
}

void StorageArrayLength::retrieveValue(SourceLocation const&, bool _remove) const
{
	if (!_remove)
		m_context << eth::Instruction::DUP1;
	m_context << eth::Instruction::SLOAD;
}

void StorageArrayLength::storeValue(Type const&, SourceLocation const&, bool _move) const
{
	if (_move)
		m_context << eth::Instruction::SWAP1;
	else
		m_context << eth::Instruction::DUP2;
	ArrayUtils(m_context).resizeDynamicArray(m_arrayType);
}

void StorageArrayLength::setToZero(SourceLocation const&, bool _removeReference) const
{
	if (!_removeReference)
		m_context << eth::Instruction::DUP1;
	ArrayUtils(m_context).clearDynamicArray(m_arrayType);
}
