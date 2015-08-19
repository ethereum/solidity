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
	if (stackPos + 1 > 16) //@todo correct this by fetching earlier or moving to memory
		BOOST_THROW_EXCEPTION(
			CompilerError() <<
			errinfo_sourceLocation(_location) <<
			errinfo_comment("Stack too deep, try removing local variables.")
		);
	solAssert(stackPos + 1 >= m_size, "Size and stack pos mismatch.");
	for (unsigned i = 0; i < m_size; ++i)
		m_context << eth::dupInstruction(stackPos + 1);
}

void StackVariable::storeValue(Type const&, SourceLocation const& _location, bool _move) const
{
	unsigned stackDiff = m_context.baseToCurrentStackOffset(m_baseStackOffset) - m_size + 1;
	if (stackDiff > 16)
		BOOST_THROW_EXCEPTION(
			CompilerError() <<
			errinfo_sourceLocation(_location) <<
			errinfo_comment("Stack too deep, try removing local variables.")
		);
	else if (stackDiff > 0)
		for (unsigned i = 0; i < m_size; ++i)
			m_context << eth::swapInstruction(stackDiff) << eth::Instruction::POP;
	if (!_move)
		retrieveValue(_location);
}

void StackVariable::setToZero(SourceLocation const& _location, bool) const
{
	CompilerUtils(m_context).pushZeroValue(m_dataType);
	storeValue(m_dataType, _location, true);
}

MemoryItem::MemoryItem(CompilerContext& _compilerContext, Type const& _type, bool _padded):
	LValue(_compilerContext, _type),
	m_padded(_padded)
{
}

void MemoryItem::retrieveValue(SourceLocation const&, bool _remove) const
{
	if (m_dataType.isValueType())
	{
		if (!_remove)
			m_context << eth::Instruction::DUP1;
		CompilerUtils(m_context).loadFromMemoryDynamic(m_dataType, false, m_padded, false);
	}
	else
		m_context << eth::Instruction::MLOAD;
}

void MemoryItem::storeValue(Type const& _sourceType, SourceLocation const&, bool _move) const
{
	CompilerUtils utils(m_context);
	if (m_dataType.isValueType())
	{
		solAssert(_sourceType.isValueType(), "");
		utils.moveIntoStack(_sourceType.getSizeOnStack());
		utils.convertType(_sourceType, m_dataType, true);
		if (!_move)
		{
			utils.moveToStackTop(m_dataType.getSizeOnStack());
			utils.copyToStackTop(2, m_dataType.getSizeOnStack());
		}
		utils.storeInMemoryDynamic(m_dataType, m_padded);
		m_context << eth::Instruction::POP;
	}
	else
	{
		solAssert(_sourceType == m_dataType, "Conversion not implemented for assignment to memory.");

		solAssert(m_dataType.getSizeOnStack() == 1, "");
		if (!_move)
			m_context << eth::Instruction::DUP2 << eth::Instruction::SWAP1;
		// stack: [value] value lvalue
		// only store the reference
		m_context << eth::Instruction::MSTORE;
	}
}

void MemoryItem::setToZero(SourceLocation const&, bool _removeReference) const
{
	CompilerUtils utils(m_context);
	if (!_removeReference)
		m_context << eth::Instruction::DUP1;
	utils.pushZeroValue(m_dataType);
	utils.storeInMemoryDynamic(m_dataType, m_padded);
	m_context << eth::Instruction::POP;
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
	{
		solAssert(m_dataType.getSizeOnStack() == 1, "Invalid storage ref size.");
		if (_remove)
			m_context << eth::Instruction::POP; // remove byte offset
		else
			m_context << eth::Instruction::DUP2;
		return;
	}
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
	CompilerUtils utils(m_context);
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
					<< (u256(0x1) << (256 - 8 * dynamic_cast<FixedBytesType const&>(m_dataType).numBytes()))
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
			m_context << eth::Instruction::POP; // remove byte offset
			ArrayUtils(m_context).copyArrayToStorage(
						dynamic_cast<ArrayType const&>(m_dataType),
						dynamic_cast<ArrayType const&>(_sourceType));
			if (_move)
				m_context << eth::Instruction::POP;
		}
		else if (m_dataType.getCategory() == Type::Category::Struct)
		{
			// stack layout: source_ref target_ref target_offset
			// note that we have structs, so offset should be zero and are ignored
			m_context << eth::Instruction::POP;
			auto const& structType = dynamic_cast<StructType const&>(m_dataType);
			auto const& sourceType = dynamic_cast<StructType const&>(_sourceType);
			solAssert(
				structType.structDefinition() == sourceType.structDefinition(),
				"Struct assignment with conversion."
			);
			solAssert(sourceType.location() != DataLocation::CallData, "Structs in calldata not supported.");
			for (auto const& member: structType.getMembers())
			{
				// assign each member that is not a mapping
				TypePointer const& memberType = member.type;
				if (memberType->getCategory() == Type::Category::Mapping)
					continue;
				TypePointer sourceMemberType = sourceType.getMemberType(member.name);
				if (sourceType.location() == DataLocation::Storage)
				{
					// stack layout: source_ref target_ref
					pair<u256, unsigned> const& offsets = sourceType.getStorageOffsetsOfMember(member.name);
					m_context << offsets.first << eth::Instruction::DUP3 << eth::Instruction::ADD;
					m_context << u256(offsets.second);
					// stack: source_ref target_ref source_member_ref source_member_off
					StorageItem(m_context, *sourceMemberType).retrieveValue(_location, true);
					// stack: source_ref target_ref source_value...
				}
				else
				{
					solAssert(sourceType.location() == DataLocation::Memory, "");
					// stack layout: source_ref target_ref
					TypePointer sourceMemberType = sourceType.getMemberType(member.name);
					m_context << sourceType.memoryOffsetOfMember(member.name);
					m_context << eth::Instruction::DUP3 << eth::Instruction::ADD;
					MemoryItem(m_context, *sourceMemberType).retrieveValue(_location, true);
					// stack layout: source_ref target_ref source_value...
				}
				unsigned stackSize = sourceMemberType->getSizeOnStack();
				pair<u256, unsigned> const& offsets = structType.getStorageOffsetsOfMember(member.name);
				m_context << eth::dupInstruction(1 + stackSize) << offsets.first << eth::Instruction::ADD;
				m_context << u256(offsets.second);
				// stack: source_ref target_ref target_off source_value... target_member_ref target_member_byte_off
				StorageItem(m_context, *memberType).storeValue(*sourceMemberType, _location, true);
			}
			// stack layout: source_ref target_ref
			solAssert(sourceType.getSizeOnStack() == 1, "Unexpected source size.");
			if (_move)
				utils.popStackSlots(2);
			else
				m_context << eth::Instruction::SWAP1 << eth::Instruction::POP;
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
