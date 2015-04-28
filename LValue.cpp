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
	auto const& location = m_context.getStorageLocationOfVariable(_declaration);
	m_context << location.first << u256(location.second);
}

StorageItem::StorageItem(CompilerContext& _compilerContext, Type const& _type):
	LValue(_compilerContext, _type)
{
	if (m_dataType.isValueType())
	{
		solAssert(m_dataType.getStorageSize() == m_dataType.getSizeOnStack(), "");
		solAssert(m_dataType.getStorageSize() == 1, "Invalid storage size.");
	}
}

void StorageItem::retrieveValue(SourceLocation const&, bool _remove) const
{
	// stack: storage_key storage_offset
	if (!m_dataType.isValueType())
		return; // no distinction between value and reference for non-value types
	if (!_remove)
		CompilerUtils(m_context).copyToStackTop(sizeOnStack(), sizeOnStack());
	if (m_dataType.getStorageBytes() == 32)
		m_context << eth::Instruction::POP << eth::Instruction::SLOAD;
	else
	{
		m_context
			<< eth::Instruction::SWAP1 << eth::Instruction::SLOAD << eth::Instruction::SWAP1
			<< u256(0x100) << eth::Instruction::EXP << eth::Instruction::SWAP1 << eth::Instruction::DIV;
		if (m_dataType.getCategory() == Type::Category::FixedBytes)
			m_context << (u256(0x1) << (256 - 8 * m_dataType.getStorageBytes())) << eth::Instruction::MUL;
		else if (
			m_dataType.getCategory() == Type::Category::Integer &&
			dynamic_cast<IntegerType const&>(m_dataType).isSigned()
		)
			m_context << u256(m_dataType.getStorageBytes() - 1) << eth::Instruction::SIGNEXTEND;
		else
			m_context << ((u256(0x1) << (8 * m_dataType.getStorageBytes())) - 1) << eth::Instruction::AND;
	}
}

void StorageItem::storeValue(Type const& _sourceType, SourceLocation const& _location, bool _move) const
{
	// stack: value storage_key storage_offset
	if (m_dataType.isValueType())
	{
		solAssert(m_dataType.getStorageBytes() <= 32, "Invalid storage bytes size.");
		solAssert(m_dataType.getStorageBytes() > 0, "Invalid storage bytes size.");
		if (m_dataType.getStorageBytes() == 32)
		{
			// offset should be zero
			m_context << eth::Instruction::POP;
			if (!_move)
				m_context << eth::Instruction::DUP2 << eth::Instruction::SWAP1;
			m_context << eth::Instruction::SSTORE;
		}
		else
		{
			// OR the value into the other values in the storage slot
			m_context << u256(0x100) << eth::Instruction::EXP;
			// stack: value storage_ref multiplier
			// fetch old value
			m_context << eth::Instruction::DUP2 << eth::Instruction::SLOAD;
			// stack: value storege_ref multiplier old_full_value
			// clear bytes in old value
			m_context
				<< eth::Instruction::DUP2 << ((u256(1) << (8 * m_dataType.getStorageBytes())) - 1)
				<< eth::Instruction::MUL;
			m_context << eth::Instruction::NOT << eth::Instruction::AND;
			// stack: value storage_ref multiplier cleared_value
			m_context
				<< eth::Instruction::SWAP1 << eth::Instruction::DUP4;
			// stack: value storage_ref cleared_value multiplier value
			if (m_dataType.getCategory() == Type::Category::FixedBytes)
				m_context
					<< (u256(0x1) << (256 - 8 * dynamic_cast<FixedBytesType const&>(m_dataType).getNumBytes()))
					<< eth::Instruction::SWAP1 << eth::Instruction::DIV;
			else if (
				m_dataType.getCategory() == Type::Category::Integer &&
				dynamic_cast<IntegerType const&>(m_dataType).isSigned()
			)
				// remove the higher order bits
				m_context
					<< (u256(1) << (8 * (32 - m_dataType.getStorageBytes())))
					<< eth::Instruction::SWAP1
					<< eth::Instruction::DUP2
					<< eth::Instruction::MUL
					<< eth::Instruction::DIV;
			m_context  << eth::Instruction::MUL << eth::Instruction::OR;
			// stack: value storage_ref updated_value
			m_context << eth::Instruction::SWAP1 << eth::Instruction::SSTORE;
			if (_move)
				m_context << eth::Instruction::POP;
		}
	}
	else
	{
		solAssert(
			_sourceType.getCategory() == m_dataType.getCategory(),
			"Wrong type conversation for assignment.");
		if (m_dataType.getCategory() == Type::Category::Array)
		{
			ArrayUtils(m_context).copyArrayToStorage(
						dynamic_cast<ArrayType const&>(m_dataType),
						dynamic_cast<ArrayType const&>(_sourceType));
			if (_move)
				CompilerUtils(m_context).popStackElement(_sourceType);
		}
		else if (m_dataType.getCategory() == Type::Category::Struct)
		{
			// stack layout: source_ref source_offset target_ref target_offset
			// note that we have structs, so offsets should be zero and are ignored
			auto const& structType = dynamic_cast<StructType const&>(m_dataType);
			solAssert(structType == _sourceType, "Struct assignment with conversion.");
			for (auto const& member: structType.getMembers())
			{
				// assign each member that is not a mapping
				TypePointer const& memberType = member.type;
				if (memberType->getCategory() == Type::Category::Mapping)
					continue;
				pair<u256, unsigned> const& offsets = structType.getStorageOffsetsOfMember(member.name);
				m_context
					<< offsets.first << u256(offsets.second)
					<< eth::Instruction::DUP6 << eth::Instruction::DUP3
					<< eth::Instruction::ADD << eth::Instruction::DUP2;
				// stack: source_ref source_off target_ref target_off member_slot_offset member_byte_offset source_member_ref source_member_off
				StorageItem(m_context, *memberType).retrieveValue(_location, true);
				// stack: source_ref source_off target_ref target_off member_offset source_value...
				solAssert(4 + memberType->getSizeOnStack() <= 16, "Stack too deep.");
				m_context
					<< eth::dupInstruction(4 + memberType->getSizeOnStack())
					<< eth::dupInstruction(3 + memberType->getSizeOnStack()) << eth::Instruction::ADD
					<< eth::dupInstruction(2 + memberType->getSizeOnStack());
				// stack: source_ref source_off target_ref target_off member_slot_offset member_byte_offset source_value... target_member_ref target_member_byte_off
				StorageItem(m_context, *memberType).storeValue(*memberType, _location, true);
				m_context << eth::Instruction::POP << eth::Instruction::POP;
			}
			if (_move)
				m_context
					<< eth::Instruction::POP << eth::Instruction::POP
					<< eth::Instruction::POP << eth::Instruction::POP;
			else
				m_context
					<< eth::Instruction::SWAP2 << eth::Instruction::POP
					<< eth::Instruction::SWAP2 << eth::Instruction::POP;
		}
		else
			BOOST_THROW_EXCEPTION(
				InternalCompilerError()
					<< errinfo_sourceLocation(_location)
					<< errinfo_comment("Invalid non-value type for assignment."));
	}
}

void StorageItem::setToZero(SourceLocation const&, bool _removeReference) const
{
	if (m_dataType.getCategory() == Type::Category::Array)
	{
		if (!_removeReference)
			CompilerUtils(m_context).copyToStackTop(sizeOnStack(), sizeOnStack());
		ArrayUtils(m_context).clearArray(dynamic_cast<ArrayType const&>(m_dataType));
	}
	else if (m_dataType.getCategory() == Type::Category::Struct)
	{
		// stack layout: storage_key storage_offset
		// @todo this can be improved: use StorageItem for non-value types, and just store 0 in
		// all slots that contain value types later.
		auto const& structType = dynamic_cast<StructType const&>(m_dataType);
		for (auto const& member: structType.getMembers())
		{
			// zero each member that is not a mapping
			TypePointer const& memberType = member.type;
			if (memberType->getCategory() == Type::Category::Mapping)
				continue;
			pair<u256, unsigned> const& offsets = structType.getStorageOffsetsOfMember(member.name);
			m_context
				<< offsets.first << eth::Instruction::DUP3 << eth::Instruction::ADD
				<< u256(offsets.second);
			StorageItem(m_context, *memberType).setToZero();
		}
		if (_removeReference)
			m_context << eth::Instruction::POP << eth::Instruction::POP;
	}
	else
	{
		solAssert(m_dataType.isValueType(), "Clearing of unsupported type requested: " + m_dataType.toString());
		if (!_removeReference)
			CompilerUtils(m_context).copyToStackTop(sizeOnStack(), sizeOnStack());
		if (m_dataType.getStorageBytes() == 32)
		{
			// offset should be zero
			m_context
				<< eth::Instruction::POP << u256(0)
				<< eth::Instruction::SWAP1 << eth::Instruction::SSTORE;
		}
		else
		{
			m_context << u256(0x100) << eth::Instruction::EXP;
			// stack: storage_ref multiplier
			// fetch old value
			m_context << eth::Instruction::DUP2 << eth::Instruction::SLOAD;
			// stack: storege_ref multiplier old_full_value
			// clear bytes in old value
			m_context
				<< eth::Instruction::SWAP1 << ((u256(1) << (8 * m_dataType.getStorageBytes())) - 1)
				<< eth::Instruction::MUL;
			m_context << eth::Instruction::NOT << eth::Instruction::AND;
			// stack: storage_ref cleared_value
			m_context << eth::Instruction::SWAP1 << eth::Instruction::SSTORE;
		}
	}
}

/// Used in StorageByteArrayElement
static FixedBytesType byteType(1);

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
	m_context << (u256(1) << (256 - 8)) << eth::Instruction::MUL;
}

void StorageByteArrayElement::storeValue(Type const&, SourceLocation const&, bool _move) const
{
	// stack: value ref byte_number
	m_context << u256(31) << eth::Instruction::SUB << u256(0x100) << eth::Instruction::EXP;
	// stack: value ref (1<<(8*(31-byte_number)))
	m_context << eth::Instruction::DUP2 << eth::Instruction::SLOAD;
	// stack: value ref (1<<(8*(31-byte_number))) old_full_value
	// clear byte in old value
	m_context << eth::Instruction::DUP2 << u256(0xff) << eth::Instruction::MUL
		<< eth::Instruction::NOT << eth::Instruction::AND;
	// stack: value ref (1<<(32-byte_number)) old_full_value_with_cleared_byte
	m_context << eth::Instruction::SWAP1;
	m_context << (u256(1) << (256 - 8)) << eth::Instruction::DUP5 << eth::Instruction::DIV
		<< eth::Instruction::MUL << eth::Instruction::OR;
	// stack: value ref new_full_value
	m_context << eth::Instruction::SWAP1 << eth::Instruction::SSTORE;
	if (_move)
		m_context << eth::Instruction::POP;
}

void StorageByteArrayElement::setToZero(SourceLocation const&, bool _removeReference) const
{
	// stack: ref byte_number
	if (!_removeReference)
		m_context << eth::Instruction::DUP2 << eth::Instruction::DUP2;
	m_context << u256(31) << eth::Instruction::SUB << u256(0x100) << eth::Instruction::EXP;
	// stack: ref (1<<(8*(31-byte_number)))
	m_context << eth::Instruction::DUP2 << eth::Instruction::SLOAD;
	// stack: ref (1<<(8*(31-byte_number))) old_full_value
	// clear byte in old value
	m_context << eth::Instruction::SWAP1 << u256(0xff) << eth::Instruction::MUL;
	m_context << eth::Instruction::NOT << eth::Instruction::AND;
	// stack: ref old_full_value_with_cleared_byte
	m_context << eth::Instruction::SWAP1 << eth::Instruction::SSTORE;
}

StorageArrayLength::StorageArrayLength(CompilerContext& _compilerContext, const ArrayType& _arrayType):
	LValue(_compilerContext, *_arrayType.getMemberType("length")),
	m_arrayType(_arrayType)
{
	solAssert(m_arrayType.isDynamicallySized(), "");
	// storage byte offset must be zero
	m_context << eth::Instruction::POP;
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
