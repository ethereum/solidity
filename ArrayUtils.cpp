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

	IntegerType uint256(256);
	Type const* targetBaseType = _targetType.isByteArray() ? &uint256 : &(*_targetType.getBaseType());
	Type const* sourceBaseType = _sourceType.isByteArray() ? &uint256 : &(*_sourceType.getBaseType());

	switch (_sourceType.getLocation())
	{
	case ArrayType::Location::CallData:
	{
		solAssert(_targetType.isByteArray(), "Non byte arrays not yet implemented here.");
		solAssert(_sourceType.isByteArray(), "Non byte arrays not yet implemented here.");
		// This also assumes that after "length" we only have zeros, i.e. it cannot be used to
		// slice a byte array from calldata.

		// stack: source_offset source_len target_ref
		// fetch old length and convert to words
		m_context << eth::Instruction::DUP1 << eth::Instruction::SLOAD;
		convertLengthToSize(_targetType);
		// stack here: source_offset source_len target_ref target_length_words
		// actual array data is stored at SHA3(storage_offset)
		m_context << eth::Instruction::DUP2;
		CompilerUtils(m_context).computeHashStatic();
		// compute target_data_end
		m_context << eth::Instruction::DUP1 << eth::Instruction::SWAP2 << eth::Instruction::ADD
				  << eth::Instruction::SWAP1;
		// stack here: source_offset source_len target_ref target_data_end target_data_ref
		// store length (in bytes)
		m_context << eth::Instruction::DUP4 << eth::Instruction::DUP1 << eth::Instruction::DUP5
			<< eth::Instruction::SSTORE;
		// jump to end if length is zero
		m_context << eth::Instruction::ISZERO;
		eth::AssemblyItem copyLoopEnd = m_context.newTag();
		m_context.appendConditionalJumpTo(copyLoopEnd);
		// store start offset
		m_context << eth::Instruction::DUP5;
		// stack now: source_offset source_len target_ref target_data_end target_data_ref calldata_offset
		eth::AssemblyItem copyLoopStart = m_context.newTag();
		m_context << copyLoopStart
				  // copy from calldata and store
				  << eth::Instruction::DUP1 << eth::Instruction::CALLDATALOAD
				  << eth::Instruction::DUP3 << eth::Instruction::SSTORE
				  // increment target_data_ref by 1
				  << eth::Instruction::SWAP1 << u256(1) << eth::Instruction::ADD
				  // increment calldata_offset by 32
				  << eth::Instruction::SWAP1 << u256(32) << eth::Instruction::ADD
				  // check for loop condition
				  << eth::Instruction::DUP1 << eth::Instruction::DUP6 << eth::Instruction::GT;
		m_context.appendConditionalJumpTo(copyLoopStart);
		m_context << eth::Instruction::POP;
		m_context << copyLoopEnd;

		// now clear leftover bytes of the old value
		// stack now: source_offset source_len target_ref target_data_end target_data_ref
		clearStorageLoop(IntegerType(256));
		// stack now: source_offset source_len target_ref target_data_end

		m_context << eth::Instruction::POP << eth::Instruction::SWAP2
			<< eth::Instruction::POP << eth::Instruction::POP;
		break;
	}
	case ArrayType::Location::Storage:
	{
		// this copies source to target and also clears target if it was larger

		solAssert(sourceBaseType->getStorageSize() == targetBaseType->getStorageSize(),
			"Copying with different storage sizes not yet implemented.");
		// stack: source_ref target_ref
		// store target_ref
		m_context << eth::Instruction::SWAP1 << eth::Instruction::DUP2;
		// stack: target_ref source_ref target_ref
		// fetch lengthes
		retrieveLength(_targetType);
		m_context << eth::Instruction::SWAP2;
		// stack: target_ref target_len target_ref source_ref
		retrieveLength(_sourceType);
		// stack: target_ref target_len target_ref source_ref source_len
		if (_targetType.isDynamicallySized())
			// store new target length
			m_context << eth::Instruction::DUP1 << eth::Instruction::DUP4 << eth::Instruction::SSTORE;
		// compute hashes (data positions)
		m_context << eth::Instruction::SWAP2;
		if (_targetType.isDynamicallySized())
			CompilerUtils(m_context).computeHashStatic();
		m_context << eth::Instruction::SWAP1;
		if (_sourceType.isDynamicallySized())
			CompilerUtils(m_context).computeHashStatic();
		// stack: target_ref target_len source_len target_data_pos source_data_pos
		m_context << eth::Instruction::DUP4;
		convertLengthToSize(_sourceType);
		m_context << eth::Instruction::DUP4;
		convertLengthToSize(_sourceType);
		// stack: target_ref target_len source_len target_data_pos source_data_pos target_size source_size
		// @todo we might be able to go without a third counter
		m_context << u256(0);
		// stack: target_ref target_len source_len target_data_pos source_data_pos target_size source_size counter
		eth::AssemblyItem copyLoopStart = m_context.newTag();
		m_context << copyLoopStart;
		// check for loop condition
		m_context << eth::Instruction::DUP1 << eth::Instruction::DUP3
				   << eth::Instruction::GT << eth::Instruction::ISZERO;
		eth::AssemblyItem copyLoopEnd = m_context.newTag();
		m_context.appendConditionalJumpTo(copyLoopEnd);
		// copy
		m_context << eth::Instruction::DUP4 << eth::Instruction::DUP2 << eth::Instruction::ADD;
		StorageItem(m_context, *sourceBaseType).retrieveValue(SourceLocation(), true);
		m_context << eth::dupInstruction(5 + sourceBaseType->getSizeOnStack())
			<< eth::dupInstruction(2 + sourceBaseType->getSizeOnStack()) << eth::Instruction::ADD;
		StorageItem(m_context, *targetBaseType).storeValue(*sourceBaseType, SourceLocation(), true);
		// increment
		m_context << targetBaseType->getStorageSize() << eth::Instruction::ADD;
		m_context.appendJumpTo(copyLoopStart);
		m_context << copyLoopEnd;

		// zero-out leftovers in target
		// stack: target_ref target_len source_len target_data_pos source_data_pos target_size source_size counter
		// add counter to target_data_pos
		m_context << eth::Instruction::DUP5 << eth::Instruction::ADD
				  << eth::Instruction::SWAP5 << eth::Instruction::POP;
		// stack: target_ref target_len target_data_pos_updated target_data_pos source_data_pos target_size source_size
		// add size to target_data_pos to get target_data_end
		m_context << eth::Instruction::POP << eth::Instruction::DUP3 << eth::Instruction::ADD
				  << eth::Instruction::SWAP4
				  << eth::Instruction::POP  << eth::Instruction::POP << eth::Instruction::POP;
		// stack: target_ref target_data_end target_data_pos_updated
		clearStorageLoop(*targetBaseType);
		m_context << eth::Instruction::POP;
		break;
	}
	default:
		solAssert(false, "Given byte array location not implemented.");
	}
}

void ArrayUtils::clearArray(ArrayType const& _type) const
{
	solAssert(_type.getLocation() == ArrayType::Location::Storage, "");
	if (_type.isDynamicallySized())
		clearDynamicArray(_type);
	else if (_type.getLength() == 0)
		m_context << eth::Instruction::POP;
	else if (_type.getLength() < 5) // unroll loop for small arrays @todo choose a good value
	{
		for (unsigned i = 1; i < _type.getLength(); ++i)
		{
			StorageItem(m_context, *_type.getBaseType()).setToZero(SourceLocation(), false);
			m_context << u256(_type.getBaseType()->getStorageSize()) << eth::Instruction::ADD;
		}
		StorageItem(m_context, *_type.getBaseType()).setToZero(SourceLocation(), true);
	}
	else
	{
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

void ArrayUtils::convertLengthToSize(ArrayType const& _arrayType) const
{
	if (_arrayType.isByteArray())
		m_context << u256(31) << eth::Instruction::ADD
			<< u256(32) << eth::Instruction::SWAP1 << eth::Instruction::DIV;
	else if (_arrayType.getBaseType()->getStorageSize() > 1)
		m_context << _arrayType.getBaseType()->getStorageSize() << eth::Instruction::MUL;
}

void ArrayUtils::retrieveLength(ArrayType const& _arrayType) const
{
	if (_arrayType.isDynamicallySized())
		m_context << eth::Instruction::DUP1 << eth::Instruction::SLOAD;
	else
		m_context << _arrayType.getLength();
}

