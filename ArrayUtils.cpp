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
 * Code generation utils that handle arrays.
 */

#include <libsolidity/ArrayUtils.h>
#include <libevmcore/Instruction.h>
#include <libsolidity/CompilerContext.h>
#include <libsolidity/CompilerUtils.h>
#include <libsolidity/Types.h>
#include <libsolidity/Utils.h>
#include <libsolidity/LValue.h>

using namespace std;
using namespace dev;
using namespace solidity;

void ArrayUtils::copyArrayToStorage(ArrayType const& _targetType, ArrayType const& _sourceType) const
{
	// stack layout: [source_ref] target_ref (top)
	// need to leave target_ref on the stack at the end
	solAssert(_targetType.getLocation() == ArrayType::Location::Storage, "");
	solAssert(
		_sourceType.getLocation() == ArrayType::Location::CallData ||
			_sourceType.getLocation() == ArrayType::Location::Storage,
		"Given array location not implemented."
	);

	IntegerType uint256(256);
	Type const* targetBaseType = _targetType.isByteArray() ? &uint256 : &(*_targetType.getBaseType());
	Type const* sourceBaseType = _sourceType.isByteArray() ? &uint256 : &(*_sourceType.getBaseType());

	// this copies source to target and also clears target if it was larger

	// TODO unroll loop for small sizes

	// stack: source_ref [source_length] target_ref
	// store target_ref
	for (unsigned i = _sourceType.getSizeOnStack(); i > 0; --i)
		m_context << eth::swapInstruction(i);
	if (_sourceType.getLocation() != ArrayType::Location::CallData || !_sourceType.isDynamicallySized())
		retrieveLength(_sourceType); // otherwise, length is already there
	// stack: target_ref source_ref source_length
	m_context << eth::Instruction::DUP3;
	// stack: target_ref source_ref source_length target_ref
	retrieveLength(_targetType);
	// stack: target_ref source_ref source_length target_ref target_length
	if (_targetType.isDynamicallySized())
		// store new target length
		m_context << eth::Instruction::DUP3 << eth::Instruction::DUP3 << eth::Instruction::SSTORE;
	if (sourceBaseType->getCategory() == Type::Category::Mapping)
	{
		solAssert(targetBaseType->getCategory() == Type::Category::Mapping, "");
		solAssert(_sourceType.getLocation() == ArrayType::Location::Storage, "");
		// nothing to copy
		m_context
			<< eth::Instruction::POP << eth::Instruction::POP
			<< eth::Instruction::POP << eth::Instruction::POP;
		return;
	}
	// compute hashes (data positions)
	m_context << eth::Instruction::SWAP1;
	if (_targetType.isDynamicallySized())
		CompilerUtils(m_context).computeHashStatic();
	// stack: target_ref source_ref source_length target_length target_data_pos
	m_context << eth::Instruction::SWAP1;
	convertLengthToSize(_targetType);
	m_context << eth::Instruction::DUP2 << eth::Instruction::ADD;
	// stack: target_ref source_ref source_length target_data_pos target_data_end
	m_context << eth::Instruction::SWAP3;
	// stack: target_ref target_data_end source_length target_data_pos source_ref
	// skip copying if source length is zero
	m_context << eth::Instruction::DUP3 << eth::Instruction::ISZERO;
	eth::AssemblyItem copyLoopEnd = m_context.newTag();
	m_context.appendConditionalJumpTo(copyLoopEnd);

	if (_sourceType.getLocation() == ArrayType::Location::Storage && _sourceType.isDynamicallySized())
		CompilerUtils(m_context).computeHashStatic();
	// stack: target_ref target_data_end source_length target_data_pos source_data_pos
	m_context << eth::Instruction::SWAP2;
	convertLengthToSize(_sourceType);
	m_context << eth::Instruction::DUP3 << eth::Instruction::ADD;
	// stack: target_ref target_data_end source_data_pos target_data_pos source_data_end
	eth::AssemblyItem copyLoopStart = m_context.newTag();
	m_context << copyLoopStart;
	// check for loop condition
	m_context
		<< eth::Instruction::DUP3 << eth::Instruction::DUP2
		<< eth::Instruction::GT << eth::Instruction::ISZERO;
	m_context.appendConditionalJumpTo(copyLoopEnd);
	// stack: target_ref target_data_end source_data_pos target_data_pos source_data_end
	// copy
	if (sourceBaseType->getCategory() == Type::Category::Array)
	{
		m_context << eth::Instruction::DUP3 << eth::Instruction::DUP3;
		copyArrayToStorage(
			dynamic_cast<ArrayType const&>(*targetBaseType),
			dynamic_cast<ArrayType const&>(*sourceBaseType)
		);
		m_context << eth::Instruction::POP;
	}
	else
	{
		m_context << eth::Instruction::DUP3;
		if (_sourceType.getLocation() == ArrayType::Location::Storage)
			StorageItem(m_context, *sourceBaseType).retrieveValue(SourceLocation(), true);
		else if (sourceBaseType->isValueType())
			CompilerUtils(m_context).loadFromMemoryDynamic(*sourceBaseType, true, true, false);
		else
			solAssert(false, "Copying of unknown type requested: " + sourceBaseType->toString());
		m_context << eth::dupInstruction(2 + sourceBaseType->getSizeOnStack());
		StorageItem(m_context, *targetBaseType).storeValue(*sourceBaseType, SourceLocation(), true);
	}
	// increment source
	m_context
		<< eth::Instruction::SWAP2
		<< (_sourceType.getLocation() == ArrayType::Location::Storage ?
			sourceBaseType->getStorageSize() :
			sourceBaseType->getCalldataEncodedSize())
		<< eth::Instruction::ADD
		<< eth::Instruction::SWAP2;
	// increment target
	m_context
		<< eth::Instruction::SWAP1
		<< targetBaseType->getStorageSize()
		<< eth::Instruction::ADD
		<< eth::Instruction::SWAP1;
	m_context.appendJumpTo(copyLoopStart);
	m_context << copyLoopEnd;

	// zero-out leftovers in target
	// stack: target_ref target_data_end source_data_pos target_data_pos source_data_end
	m_context << eth::Instruction::POP << eth::Instruction::SWAP1 << eth::Instruction::POP;
	// stack: target_ref target_data_end target_data_pos_updated
	clearStorageLoop(*targetBaseType);
	m_context << eth::Instruction::POP;
}

void ArrayUtils::clearArray(ArrayType const& _type) const
{
	solAssert(_type.getLocation() == ArrayType::Location::Storage, "");
	if (_type.isDynamicallySized())
		clearDynamicArray(_type);
	else if (_type.getLength() == 0 || _type.getBaseType()->getCategory() == Type::Category::Mapping)
		m_context << eth::Instruction::POP;
	else if (_type.getLength() < 5) // unroll loop for small arrays @todo choose a good value
	{
		solAssert(!_type.isByteArray(), "");
		for (unsigned i = 1; i < _type.getLength(); ++i)
		{
			StorageItem(m_context, *_type.getBaseType()).setToZero(SourceLocation(), false);
			m_context << u256(_type.getBaseType()->getStorageSize()) << eth::Instruction::ADD;
		}
		StorageItem(m_context, *_type.getBaseType()).setToZero(SourceLocation(), true);
	}
	else
	{
		solAssert(!_type.isByteArray(), "");
		m_context
			<< eth::Instruction::DUP1 << u256(_type.getLength())
			<< u256(_type.getBaseType()->getStorageSize())
			<< eth::Instruction::MUL << eth::Instruction::ADD << eth::Instruction::SWAP1;
		clearStorageLoop(*_type.getBaseType());
		m_context << eth::Instruction::POP;
	}
}

void ArrayUtils::clearDynamicArray(ArrayType const& _type) const
{
	solAssert(_type.getLocation() == ArrayType::Location::Storage, "");
	solAssert(_type.isDynamicallySized(), "");

	// fetch length
	m_context << eth::Instruction::DUP1 << eth::Instruction::SLOAD;
	// set length to zero
	m_context << u256(0) << eth::Instruction::DUP3 << eth::Instruction::SSTORE;
	// stack: ref old_length
	convertLengthToSize(_type);
	// compute data positions
	m_context << eth::Instruction::SWAP1;
	CompilerUtils(m_context).computeHashStatic();
	// stack: len data_pos (len is in slots for byte array and in items for other arrays)
	m_context << eth::Instruction::SWAP1 << eth::Instruction::DUP2 << eth::Instruction::ADD
		<< eth::Instruction::SWAP1;
	// stack: data_pos_end data_pos
	if (_type.isByteArray())
		clearStorageLoop(IntegerType(256));
	else
		clearStorageLoop(*_type.getBaseType());
	// cleanup
	m_context << eth::Instruction::POP;
}

void ArrayUtils::resizeDynamicArray(const ArrayType& _type) const
{
	solAssert(_type.getLocation() == ArrayType::Location::Storage, "");
	solAssert(_type.isDynamicallySized(), "");

	eth::AssemblyItem resizeEnd = m_context.newTag();

	// stack: ref new_length
	// fetch old length
	m_context << eth::Instruction::DUP2 << eth::Instruction::SLOAD;
	// stack: ref new_length old_length
	// store new length
	m_context << eth::Instruction::DUP2 << eth::Instruction::DUP4 << eth::Instruction::SSTORE;
	// skip if size is not reduced
	m_context << eth::Instruction::DUP2 << eth::Instruction::DUP2
		<< eth::Instruction::ISZERO << eth::Instruction::GT;
	m_context.appendConditionalJumpTo(resizeEnd);

	// size reduced, clear the end of the array
	// stack: ref new_length old_length
	convertLengthToSize(_type);
	m_context << eth::Instruction::DUP2;
	convertLengthToSize(_type);
	// stack: ref new_length old_size new_size
	// compute data positions
	m_context << eth::Instruction::DUP4;
	CompilerUtils(m_context).computeHashStatic();
	// stack: ref new_length old_size new_size data_pos
	m_context << eth::Instruction::SWAP2 << eth::Instruction::DUP3 << eth::Instruction::ADD;
	// stack: ref new_length data_pos new_size delete_end
	m_context << eth::Instruction::SWAP2 << eth::Instruction::ADD;
	// stack: ref new_length delete_end delete_start
	if (_type.isByteArray())
		clearStorageLoop(IntegerType(256));
	else
		clearStorageLoop(*_type.getBaseType());

	m_context << resizeEnd;
	// cleanup
	m_context << eth::Instruction::POP << eth::Instruction::POP << eth::Instruction::POP;
}

void ArrayUtils::clearStorageLoop(Type const& _type) const
{
	if (_type.getCategory() == Type::Category::Mapping)
	{
		m_context << eth::Instruction::POP;
		return;
	}
	// stack: end_pos pos
	eth::AssemblyItem loopStart = m_context.newTag();
	m_context << loopStart;
	// check for loop condition
	m_context << eth::Instruction::DUP1 << eth::Instruction::DUP3
			   << eth::Instruction::GT << eth::Instruction::ISZERO;
	eth::AssemblyItem zeroLoopEnd = m_context.newTag();
	m_context.appendConditionalJumpTo(zeroLoopEnd);
	// delete
	StorageItem(m_context, _type).setToZero(SourceLocation(), false);
	// increment
	m_context << u256(1) << eth::Instruction::ADD;
	m_context.appendJumpTo(loopStart);
	// cleanup
	m_context << zeroLoopEnd;
	m_context << eth::Instruction::POP;
}

void ArrayUtils::convertLengthToSize(ArrayType const& _arrayType, bool _pad) const
{
	if (_arrayType.getLocation() == ArrayType::Location::Storage)
	{
		if (_arrayType.isByteArray())
			m_context << u256(31) << eth::Instruction::ADD
				<< u256(32) << eth::Instruction::SWAP1 << eth::Instruction::DIV;
		else if (_arrayType.getBaseType()->getStorageSize() > 1)
			m_context << _arrayType.getBaseType()->getStorageSize() << eth::Instruction::MUL;
	}
	else
	{
		if (!_arrayType.isByteArray())
			m_context << _arrayType.getBaseType()->getCalldataEncodedSize() << eth::Instruction::MUL;
		else if (_pad)
			m_context << u256(31) << eth::Instruction::ADD
				<< u256(32) << eth::Instruction::DUP1
				<< eth::Instruction::SWAP2 << eth::Instruction::DIV << eth::Instruction::MUL;
	}
}

void ArrayUtils::retrieveLength(ArrayType const& _arrayType) const
{
	if (!_arrayType.isDynamicallySized())
		m_context << _arrayType.getLength();
	else
	{
		m_context << eth::Instruction::DUP1;
		switch (_arrayType.getLocation())
		{
		case ArrayType::Location::CallData:
			// length is stored on the stack
			break;
		case ArrayType::Location::Memory:
			m_context << eth::Instruction::MLOAD;
			break;
		case ArrayType::Location::Storage:
			m_context << eth::Instruction::SLOAD;
			break;
		}
	}
}

