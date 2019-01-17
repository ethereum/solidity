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
 * @author Christian <c@ethdev.com>
 * @date 2015
 * LValues for use in the expression compiler.
 */

#include <libsolidity/codegen/LValue.h>

#include <libsolidity/ast/AST.h>
#include <libsolidity/ast/Types.h>
#include <libsolidity/codegen/CompilerUtils.h>
#include <libevmasm/Instruction.h>

using namespace std;
using namespace dev;
using namespace langutil;
using namespace solidity;


StackVariable::StackVariable(CompilerContext& _compilerContext, VariableDeclaration const& _declaration):
	LValue(_compilerContext, _declaration.annotation().type.get()),
	m_baseStackOffset(m_context.baseStackOffsetOfVariable(_declaration)),
	m_size(m_dataType->sizeOnStack())
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
		m_context << dupInstruction(stackPos + 1);
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
			m_context << swapInstruction(stackDiff) << Instruction::POP;
	if (!_move)
		retrieveValue(_location);
}

void StackVariable::setToZero(SourceLocation const& _location, bool) const
{
	CompilerUtils(m_context).pushZeroValue(*m_dataType);
	storeValue(*m_dataType, _location, true);
}

MemoryItem::MemoryItem(CompilerContext& _compilerContext, Type const& _type, bool _padded):
	LValue(_compilerContext, &_type),
	m_padded(_padded)
{
}

void MemoryItem::retrieveValue(SourceLocation const&, bool _remove) const
{
	if (m_dataType->isValueType())
	{
		if (!_remove)
			m_context << Instruction::DUP1;
		CompilerUtils(m_context).loadFromMemoryDynamic(*m_dataType, false, m_padded, false);
	}
	else
		m_context << Instruction::MLOAD;
}

void MemoryItem::storeValue(Type const& _sourceType, SourceLocation const&, bool _move) const
{
	CompilerUtils utils(m_context);
	if (m_dataType->isValueType())
	{
		solAssert(_sourceType.isValueType(), "");
		utils.moveIntoStack(_sourceType.sizeOnStack());
		utils.convertType(_sourceType, *m_dataType, true);
		if (!_move)
		{
			utils.moveToStackTop(m_dataType->sizeOnStack());
			utils.copyToStackTop(1 + m_dataType->sizeOnStack(), m_dataType->sizeOnStack());
		}
		if (!m_padded)
		{
			solAssert(m_dataType->calldataEncodedSize(false) == 1, "Invalid non-padded type.");
			if (m_dataType->category() == Type::Category::FixedBytes)
				m_context << u256(0) << Instruction::BYTE;
			m_context << Instruction::SWAP1 << Instruction::MSTORE8;
		}
		else
		{
			utils.storeInMemoryDynamic(*m_dataType, m_padded);
			m_context << Instruction::POP;
		}
	}
	else
	{
		solUnimplementedAssert(_sourceType == *m_dataType, "Conversion not implemented for assignment to memory.");

		solAssert(m_dataType->sizeOnStack() == 1, "");
		if (!_move)
			m_context << Instruction::DUP2 << Instruction::SWAP1;
		// stack: [value] value lvalue
		// only store the reference
		m_context << Instruction::MSTORE;
	}
}

void MemoryItem::setToZero(SourceLocation const&, bool _removeReference) const
{
	CompilerUtils utils(m_context);
	solAssert(_removeReference, "");
	utils.pushZeroValue(*m_dataType);
	utils.storeInMemoryDynamic(*m_dataType, m_padded);
	m_context << Instruction::POP;
}

StorageItem::StorageItem(CompilerContext& _compilerContext, VariableDeclaration const& _declaration):
	StorageItem(_compilerContext, *_declaration.annotation().type)
{
	auto const& location = m_context.storageLocationOfVariable(_declaration);
	m_context << location.first << u256(location.second);
}

StorageItem::StorageItem(CompilerContext& _compilerContext, Type const& _type):
	LValue(_compilerContext, &_type)
{
	if (m_dataType->isValueType())
	{
		if (m_dataType->category() != Type::Category::Function)
			solAssert(m_dataType->storageSize() == m_dataType->sizeOnStack(), "");
		solAssert(m_dataType->storageSize() == 1, "Invalid storage size.");
	}
}

void StorageItem::retrieveValue(SourceLocation const&, bool _remove) const
{
	// stack: storage_key storage_offset
	if (!m_dataType->isValueType())
	{
		solAssert(m_dataType->sizeOnStack() == 1, "Invalid storage ref size.");
		if (_remove)
			m_context << Instruction::POP; // remove byte offset
		else
			m_context << Instruction::DUP2;
		return;
	}
	if (!_remove)
		CompilerUtils(m_context).copyToStackTop(sizeOnStack(), sizeOnStack());
	if (m_dataType->storageBytes() == 32)
		m_context << Instruction::POP << Instruction::SLOAD;
	else
	{
		bool cleaned = false;
		m_context
			<< Instruction::SWAP1 << Instruction::SLOAD << Instruction::SWAP1
			<< u256(0x100) << Instruction::EXP << Instruction::SWAP1 << Instruction::DIV;
		if (m_dataType->category() == Type::Category::FixedPoint)
			// implementation should be very similar to the integer case.
			solUnimplemented("Not yet implemented - FixedPointType.");
		if (m_dataType->category() == Type::Category::FixedBytes)
		{
			CompilerUtils(m_context).leftShiftNumberOnStack(256 - 8 * m_dataType->storageBytes());
			cleaned = true;
		}
		else if (
			m_dataType->category() == Type::Category::Integer &&
			dynamic_cast<IntegerType const&>(*m_dataType).isSigned()
		)
		{
			m_context << u256(m_dataType->storageBytes() - 1) << Instruction::SIGNEXTEND;
			cleaned = true;
		}
		else if (FunctionType const* fun = dynamic_cast<decltype(fun)>(m_dataType))
		{
			if (fun->kind() == FunctionType::Kind::External)
			{
				CompilerUtils(m_context).splitExternalFunctionType(false);
				cleaned = true;
			}
		}
		if (!cleaned)
		{
			solAssert(m_dataType->sizeOnStack() == 1, "");
			m_context << ((u256(0x1) << (8 * m_dataType->storageBytes())) - 1) << Instruction::AND;
		}
	}
}

void StorageItem::storeValue(Type const& _sourceType, SourceLocation const& _location, bool _move) const
{
	CompilerUtils utils(m_context);
	solAssert(m_dataType, "");

	// stack: value storage_key storage_offset
	if (m_dataType->isValueType())
	{
		solAssert(m_dataType->storageBytes() <= 32, "Invalid storage bytes size.");
		solAssert(m_dataType->storageBytes() > 0, "Invalid storage bytes size.");
		if (m_dataType->storageBytes() == 32)
		{
			solAssert(m_dataType->sizeOnStack() == 1, "Invalid stack size.");
			// offset should be zero
			m_context << Instruction::POP;
			if (!_move)
				m_context << Instruction::DUP2 << Instruction::SWAP1;

			m_context << Instruction::SWAP1;
			utils.convertType(_sourceType, *m_dataType, true);
			m_context << Instruction::SWAP1;

			m_context << Instruction::SSTORE;
		}
		else
		{
			// OR the value into the other values in the storage slot
			m_context << u256(0x100) << Instruction::EXP;
			// stack: value storage_ref multiplier
			// fetch old value
			m_context << Instruction::DUP2 << Instruction::SLOAD;
			// stack: value storege_ref multiplier old_full_value
			// clear bytes in old value
			m_context
				<< Instruction::DUP2 << ((u256(1) << (8 * m_dataType->storageBytes())) - 1)
				<< Instruction::MUL;
			m_context << Instruction::NOT << Instruction::AND << Instruction::SWAP1;
			// stack: value storage_ref cleared_value multiplier
			utils.copyToStackTop(3 + m_dataType->sizeOnStack(), m_dataType->sizeOnStack());
			// stack: value storage_ref cleared_value multiplier value
			if (FunctionType const* fun = dynamic_cast<decltype(fun)>(m_dataType))
			{
				solAssert(_sourceType == *m_dataType, "function item stored but target is not equal to source");
				if (fun->kind() == FunctionType::Kind::External)
					// Combine the two-item function type into a single stack slot.
					utils.combineExternalFunctionType(false);
				else
					m_context <<
						((u256(1) << (8 * m_dataType->storageBytes())) - 1) <<
						Instruction::AND;
			}
			else if (m_dataType->category() == Type::Category::FixedBytes)
			{
				solAssert(_sourceType.category() == Type::Category::FixedBytes, "source not fixed bytes");
				CompilerUtils(m_context).rightShiftNumberOnStack(256 - 8 * dynamic_cast<FixedBytesType const&>(*m_dataType).numBytes());
			}
			else
			{
				solAssert(m_dataType->sizeOnStack() == 1, "Invalid stack size for opaque type.");
				// remove the higher order bits
				utils.convertType(_sourceType, *m_dataType, true, true);
			}
			m_context  << Instruction::MUL << Instruction::OR;
			// stack: value storage_ref updated_value
			m_context << Instruction::SWAP1 << Instruction::SSTORE;
			if (_move)
				utils.popStackElement(*m_dataType);
		}
	}
	else
	{
		solAssert(
			_sourceType.category() == m_dataType->category(),
			"Wrong type conversation for assignment.");
		if (m_dataType->category() == Type::Category::Array)
		{
			m_context << Instruction::POP; // remove byte offset
			ArrayUtils(m_context).copyArrayToStorage(
				dynamic_cast<ArrayType const&>(*m_dataType),
				dynamic_cast<ArrayType const&>(_sourceType)
			);
			if (_move)
				m_context << Instruction::POP;
		}
		else if (m_dataType->category() == Type::Category::Struct)
		{
			// stack layout: source_ref target_ref target_offset
			// note that we have structs, so offset should be zero and are ignored
			m_context << Instruction::POP;
			auto const& structType = dynamic_cast<StructType const&>(*m_dataType);
			auto const& sourceType = dynamic_cast<StructType const&>(_sourceType);
			solAssert(
				structType.structDefinition() == sourceType.structDefinition(),
				"Struct assignment with conversion."
			);
			solAssert(sourceType.location() != DataLocation::CallData, "Structs in calldata not supported.");
			for (auto const& member: structType.members(nullptr))
			{
				// assign each member that is not a mapping
				TypePointer const& memberType = member.type;
				if (memberType->category() == Type::Category::Mapping)
					continue;
				TypePointer sourceMemberType = sourceType.memberType(member.name);
				if (sourceType.location() == DataLocation::Storage)
				{
					// stack layout: source_ref target_ref
					pair<u256, unsigned> const& offsets = sourceType.storageOffsetsOfMember(member.name);
					m_context << offsets.first << Instruction::DUP3 << Instruction::ADD;
					m_context << u256(offsets.second);
					// stack: source_ref target_ref source_member_ref source_member_off
					StorageItem(m_context, *sourceMemberType).retrieveValue(_location, true);
					// stack: source_ref target_ref source_value...
				}
				else
				{
					solAssert(sourceType.location() == DataLocation::Memory, "");
					// stack layout: source_ref target_ref
					TypePointer sourceMemberType = sourceType.memberType(member.name);
					m_context << sourceType.memoryOffsetOfMember(member.name);
					m_context << Instruction::DUP3 << Instruction::ADD;
					MemoryItem(m_context, *sourceMemberType).retrieveValue(_location, true);
					// stack layout: source_ref target_ref source_value...
				}
				unsigned stackSize = sourceMemberType->sizeOnStack();
				pair<u256, unsigned> const& offsets = structType.storageOffsetsOfMember(member.name);
				m_context << dupInstruction(1 + stackSize) << offsets.first << Instruction::ADD;
				m_context << u256(offsets.second);
				// stack: source_ref target_ref target_off source_value... target_member_ref target_member_byte_off
				StorageItem(m_context, *memberType).storeValue(*sourceMemberType, _location, true);
			}
			// stack layout: source_ref target_ref
			solAssert(sourceType.sizeOnStack() == 1, "Unexpected source size.");
			if (_move)
				utils.popStackSlots(2);
			else
				m_context << Instruction::SWAP1 << Instruction::POP;
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
	if (m_dataType->category() == Type::Category::Array)
	{
		if (!_removeReference)
			CompilerUtils(m_context).copyToStackTop(sizeOnStack(), sizeOnStack());
		ArrayUtils(m_context).clearArray(dynamic_cast<ArrayType const&>(*m_dataType));
	}
	else if (m_dataType->category() == Type::Category::Struct)
	{
		// stack layout: storage_key storage_offset
		// @todo this can be improved: use StorageItem for non-value types, and just store 0 in
		// all slots that contain value types later.
		auto const& structType = dynamic_cast<StructType const&>(*m_dataType);
		for (auto const& member: structType.members(nullptr))
		{
			// zero each member that is not a mapping
			TypePointer const& memberType = member.type;
			if (memberType->category() == Type::Category::Mapping)
				continue;
			pair<u256, unsigned> const& offsets = structType.storageOffsetsOfMember(member.name);
			m_context
				<< offsets.first << Instruction::DUP3 << Instruction::ADD
				<< u256(offsets.second);
			StorageItem(m_context, *memberType).setToZero();
		}
		if (_removeReference)
			m_context << Instruction::POP << Instruction::POP;
	}
	else
	{
		solAssert(m_dataType->isValueType(), "Clearing of unsupported type requested: " + m_dataType->toString());
		if (!_removeReference)
			CompilerUtils(m_context).copyToStackTop(sizeOnStack(), sizeOnStack());
		if (m_dataType->storageBytes() == 32)
		{
			// offset should be zero
			m_context
				<< Instruction::POP << u256(0)
				<< Instruction::SWAP1 << Instruction::SSTORE;
		}
		else
		{
			m_context << u256(0x100) << Instruction::EXP;
			// stack: storage_ref multiplier
			// fetch old value
			m_context << Instruction::DUP2 << Instruction::SLOAD;
			// stack: storege_ref multiplier old_full_value
			// clear bytes in old value
			m_context
				<< Instruction::SWAP1 << ((u256(1) << (8 * m_dataType->storageBytes())) - 1)
				<< Instruction::MUL;
			m_context << Instruction::NOT << Instruction::AND;
			// stack: storage_ref cleared_value
			m_context << Instruction::SWAP1 << Instruction::SSTORE;
		}
	}
}

/// Used in StorageByteArrayElement
static FixedBytesType byteType(1);

StorageByteArrayElement::StorageByteArrayElement(CompilerContext& _compilerContext):
	LValue(_compilerContext, &byteType)
{
}

void StorageByteArrayElement::retrieveValue(SourceLocation const&, bool _remove) const
{
	// stack: ref byte_number
	if (_remove)
		m_context << Instruction::SWAP1 << Instruction::SLOAD
			<< Instruction::SWAP1 << Instruction::BYTE;
	else
		m_context << Instruction::DUP2 << Instruction::SLOAD
			<< Instruction::DUP2 << Instruction::BYTE;
	m_context << (u256(1) << (256 - 8)) << Instruction::MUL;
}

void StorageByteArrayElement::storeValue(Type const&, SourceLocation const&, bool _move) const
{
	// stack: value ref byte_number
	m_context << u256(31) << Instruction::SUB << u256(0x100) << Instruction::EXP;
	// stack: value ref (1<<(8*(31-byte_number)))
	m_context << Instruction::DUP2 << Instruction::SLOAD;
	// stack: value ref (1<<(8*(31-byte_number))) old_full_value
	// clear byte in old value
	m_context << Instruction::DUP2 << u256(0xff) << Instruction::MUL
		<< Instruction::NOT << Instruction::AND;
	// stack: value ref (1<<(32-byte_number)) old_full_value_with_cleared_byte
	m_context << Instruction::SWAP1;
	m_context << (u256(1) << (256 - 8)) << Instruction::DUP5 << Instruction::DIV
		<< Instruction::MUL << Instruction::OR;
	// stack: value ref new_full_value
	m_context << Instruction::SWAP1 << Instruction::SSTORE;
	if (_move)
		m_context << Instruction::POP;
}

void StorageByteArrayElement::setToZero(SourceLocation const&, bool _removeReference) const
{
	// stack: ref byte_number
	solAssert(_removeReference, "");
	m_context << u256(31) << Instruction::SUB << u256(0x100) << Instruction::EXP;
	// stack: ref (1<<(8*(31-byte_number)))
	m_context << Instruction::DUP2 << Instruction::SLOAD;
	// stack: ref (1<<(8*(31-byte_number))) old_full_value
	// clear byte in old value
	m_context << Instruction::SWAP1 << u256(0xff) << Instruction::MUL;
	m_context << Instruction::NOT << Instruction::AND;
	// stack: ref old_full_value_with_cleared_byte
	m_context << Instruction::SWAP1 << Instruction::SSTORE;
}

StorageArrayLength::StorageArrayLength(CompilerContext& _compilerContext, const ArrayType& _arrayType):
	LValue(_compilerContext, _arrayType.memberType("length").get()),
	m_arrayType(_arrayType)
{
	solAssert(m_arrayType.isDynamicallySized(), "");
}

void StorageArrayLength::retrieveValue(SourceLocation const&, bool _remove) const
{
	ArrayUtils(m_context).retrieveLength(m_arrayType);
	if (_remove)
		m_context << Instruction::SWAP1 << Instruction::POP;
}

void StorageArrayLength::storeValue(Type const&, SourceLocation const&, bool _move) const
{
	if (_move)
		m_context << Instruction::SWAP1;
	else
		m_context << Instruction::DUP2;
	ArrayUtils(m_context).resizeDynamicArray(m_arrayType);
}

void StorageArrayLength::setToZero(SourceLocation const&, bool _removeReference) const
{
	solAssert(_removeReference, "");
	ArrayUtils(m_context).clearDynamicArray(m_arrayType);
}


TupleObject::TupleObject(
	CompilerContext& _compilerContext,
	std::vector<std::unique_ptr<LValue>>&& _lvalues
):
	LValue(_compilerContext), m_lvalues(move(_lvalues))
{
}

unsigned TupleObject::sizeOnStack() const
{
	unsigned size = 0;
	for (auto const& lv: m_lvalues)
		if (lv)
			size += lv->sizeOnStack();
	return size;
}

void TupleObject::retrieveValue(SourceLocation const&, bool) const
{
	solAssert(false, "Tried to retrieve value of tuple.");
}

void TupleObject::storeValue(Type const& _sourceType, SourceLocation const& _location, bool) const
{
	// values are below the lvalue references
	unsigned valuePos = sizeOnStack();
	TypePointers const& valueTypes = dynamic_cast<TupleType const&>(_sourceType).components();
	solAssert(valueTypes.size() == m_lvalues.size(), "");
	// valuePos .... refPos ...
	// We will assign from right to left to optimize stack layout.
	for (size_t i = 0; i < m_lvalues.size(); ++i)
	{
		unique_ptr<LValue> const& lvalue = m_lvalues[m_lvalues.size() - i - 1];
		TypePointer const& valType = valueTypes[valueTypes.size() - i - 1];
		unsigned stackHeight = m_context.stackHeight();
		solAssert(!valType == !lvalue, "");
		if (!lvalue)
			continue;
		valuePos += valType->sizeOnStack();
		// copy value to top
		CompilerUtils(m_context).copyToStackTop(valuePos, valType->sizeOnStack());
		// move lvalue ref above value
		CompilerUtils(m_context).moveToStackTop(valType->sizeOnStack(), lvalue->sizeOnStack());
		lvalue->storeValue(*valType, _location, true);
		valuePos += m_context.stackHeight() - stackHeight;
	}
	// As the type of an assignment to a tuple type is the empty tuple, we always move.
	CompilerUtils(m_context).popStackElement(_sourceType);
}

void TupleObject::setToZero(SourceLocation const&, bool) const
{
	solAssert(false, "Tried to delete tuple.");
}
